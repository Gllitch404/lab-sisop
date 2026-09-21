#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/moduleparam.h>

#define DEVICE_NAME "xtea_driver"
#define CLASS_NAME  "xtea"
#define MAX_BUF_LEN 4096

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("DIEGO FRAGA DE MELLO");
MODULE_DESCRIPTION("XTEA Encryption Device Driver with Module Parameters");

/* 5. Desafio: Parâmetros de módulo key0..key3 */
static char *key0 = "f0e1d2c3";
static char *key1 = "b4a59687";
static char *key2 = "78695a4b";
static char *key3 = "3c2d1e0f";

module_param(key0, charp, 0000);
MODULE_PARM_DESC(key0, "XTEA key part 0 (hex 32-bit string)");
module_param(key1, charp, 0000);
MODULE_PARM_DESC(key1, "XTEA key part 1 (hex 32-bit string)");
module_param(key2, charp, 0000);
MODULE_PARM_DESC(key2, "XTEA key part 2 (hex 32-bit string)");
module_param(key3, charp, 0000);
MODULE_PARM_DESC(key3, "XTEA key part 3 (hex 32-bit string)");

static u32 mod_key[4];

static int major_number;
static struct class*  xtea_class  = NULL;
static struct device* xtea_device = NULL;

static char output_buffer[MAX_BUF_LEN];
static size_t output_len = 0;

/* Algoritmo XTEA */
static void encipher(u32 num_rounds, u32 v[2], const u32 key[4]) {
	u32 i;
	u32 v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;
	for (i = 0; i < num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}
	v[0] = v0;
	v[1] = v1;
}

static void decipher(u32 num_rounds, u32 v[2], const u32 key[4]) {
	u32 i;
	u32 v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta * num_rounds;
	for (i = 0; i < num_rounds; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0] = v0;
	v[1] = v1;
}

static int hex_char_to_val(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static u32 parse_hex_u32(const char *str) {
	u32 val = 0;
	while (*str) {
		int v = hex_char_to_val(*str);
		if (v >= 0)
			val = (val << 4) | (u32)v;
		str++;
	}
	return val;
}

static int hex_to_bytes(const char *hex, u8 *bytes, int max_bytes) {
	int count = 0;
	while (hex[0] && hex[1] && count < max_bytes) {
		int high = hex_char_to_val(hex[0]);
		int low  = hex_char_to_val(hex[1]);
		if (high < 0 || low < 0)
			break;
		bytes[count++] = (u8)((high << 4) | low);
		hex += 2;
	}
	return count;
}

static int dev_open(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "xtea_driver: Device opened\n");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "xtea_driver: Device closed\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
	size_t to_copy;

	if (*offset >= output_len)
		return 0;

	to_copy = (len < output_len - *offset) ? len : output_len - *offset;
	if (copy_to_user(buffer, output_buffer + *offset, to_copy))
		return -EFAULT;

	*offset += to_copy;
	return to_copy;
}

/*
 * Atividade 2 e Desafio:
 * Formato suportado:
 *   1) enc <key0> <key1> <key2> <key3> <data_size> <hex_data>
 *   2) dec <key0> <key1> <key2> <key3> <data_size> <hex_data>
 *   3) enc <data_size> <hex_data> (usa chaves passadas no modprobe)
 *   4) dec <data_size> <hex_data> (usa chaves passadas no modprobe)
 */
static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
	char *kbuf;
	char *hex_data;
	u8 *raw_data;
	char cmd[16];
	char s_k0[32], s_k1[32], s_k2[32], s_k3[32];
	int data_size = 0;
	u32 key[4];
	int tokens;
	int byte_count;
	int is_encrypt = 1;
	int num_blocks;
	int i;
	int out_pos = 0;

	if (len == 0 || len >= MAX_BUF_LEN)
		return -EINVAL;

	kbuf = kmalloc(len + 1, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	hex_data = kmalloc(MAX_BUF_LEN, GFP_KERNEL);
	if (!hex_data) {
		kfree(kbuf);
		return -ENOMEM;
	}

	raw_data = kmalloc(1024, GFP_KERNEL);
	if (!raw_data) {
		kfree(hex_data);
		kfree(kbuf);
		return -ENOMEM;
	}

	if (copy_from_user(kbuf, buffer, len)) {
		kfree(raw_data);
		kfree(hex_data);
		kfree(kbuf);
		return -EFAULT;
	}
	kbuf[len] = '\0';

	/* Tenta o formato completo com 7 campos */
	tokens = sscanf(kbuf, "%15s %31s %31s %31s %31s %d %s",
			cmd, s_k0, s_k1, s_k2, s_k3, &data_size, hex_data);

	if (tokens == 7) {
		key[0] = parse_hex_u32(s_k0);
		key[1] = parse_hex_u32(s_k1);
		key[2] = parse_hex_u32(s_k2);
		key[3] = parse_hex_u32(s_k3);
	} else {
		/* Tenta o formato simplificado usando as chaves do modprobe (Desafio) */
		tokens = sscanf(kbuf, "%15s %d %s", cmd, &data_size, hex_data);
		if (tokens == 3) {
			key[0] = mod_key[0];
			key[1] = mod_key[1];
			key[2] = mod_key[2];
			key[3] = mod_key[3];
		} else {
			kfree(raw_data);
			kfree(hex_data);
			kfree(kbuf);
			printk(KERN_ERR "xtea_driver: Formato de comando invalido\n");
			return -EINVAL;
		}
	}

	if (strcmp(cmd, "enc") == 0) {
		is_encrypt = 1;
	} else if (strcmp(cmd, "dec") == 0) {
		is_encrypt = 0;
	} else {
		kfree(raw_data);
		kfree(hex_data);
		kfree(kbuf);
		printk(KERN_ERR "xtea_driver: Comando desconhecido \"%s\"\n", cmd);
		return -EINVAL;
	}

	byte_count = hex_to_bytes(hex_data, raw_data, 1024);
	if (data_size > byte_count)
		data_size = byte_count;

	/* Alinha para blocos de 8 bytes (64 bits) do XTEA */
	num_blocks = (data_size + 7) / 8;
	for (i = data_size; i < num_blocks * 8; i++) {
		raw_data[i] = 0; /* Padding */
	}

	for (i = 0; i < num_blocks; i++) {
		u32 v[2];
		u8 *p = raw_data + (i * 8);

		v[0] = ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
		v[1] = ((u32)p[4] << 24) | ((u32)p[5] << 16) | ((u32)p[6] << 8) | (u32)p[7];

		if (is_encrypt)
			encipher(32, v, key);
		else
			decipher(32, v, key);

		p[0] = (u8)(v[0] >> 24);
		p[1] = (u8)(v[0] >> 16);
		p[2] = (u8)(v[0] >> 8);
		p[3] = (u8)(v[0]);
		p[4] = (u8)(v[1] >> 24);
		p[5] = (u8)(v[1] >> 16);
		p[6] = (u8)(v[1] >> 8);
		p[7] = (u8)(v[1]);
	}

	/* Formata resultado em string hexadecimal */
	out_pos = 0;
	for (i = 0; i < num_blocks * 8; i++) {
		out_pos += snprintf(output_buffer + out_pos, MAX_BUF_LEN - out_pos, "%02x", raw_data[i]);
	}
	out_pos += snprintf(output_buffer + out_pos, MAX_BUF_LEN - out_pos, "\n");
	output_len = out_pos;

	printk(KERN_INFO "xtea_driver: %s executado com sucesso em %d bytes. Resultado: %s",
	       is_encrypt ? "Criptografia" : "Decriptografia", data_size, output_buffer);

	kfree(raw_data);
	kfree(hex_data);
	kfree(kbuf);
	return len;
}

static struct file_operations fops = {
	.owner   = THIS_MODULE,
	.open    = dev_open,
	.read    = dev_read,
	.write   = dev_write,
	.release = dev_release,
};

static int __init xtea_driver_init(void) {
	printk(KERN_INFO "xtea_driver: Inicializando xtea_driver\n");

	mod_key[0] = parse_hex_u32(key0);
	mod_key[1] = parse_hex_u32(key1);
	mod_key[2] = parse_hex_u32(key2);
	mod_key[3] = parse_hex_u32(key3);

	printk(KERN_INFO "xtea_driver: Chaves carregadas key0=0x%08x key1=0x%08x key2=0x%08x key3=0x%08x\n",
	       mod_key[0], mod_key[1], mod_key[2], mod_key[3]);

	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "xtea_driver: Falha ao registrar major number\n");
		return major_number;
	}

	xtea_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(xtea_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		return PTR_ERR(xtea_class);
	}

	xtea_device = device_create(xtea_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(xtea_device)) {
		class_destroy(xtea_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		return PTR_ERR(xtea_device);
	}

	printk(KERN_INFO "xtea_driver: Dispositivo /dev/%s criado com sucesso (major %d)\n", DEVICE_NAME, major_number);
	return 0;
}

static void __exit xtea_driver_exit(void) {
	device_destroy(xtea_class, MKDEV(major_number, 0));
	class_unregister(xtea_class);
	class_destroy(xtea_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "xtea_driver: Descarregado com sucesso\n");
}

module_init(xtea_driver_init);
module_exit(xtea_driver_exit);
