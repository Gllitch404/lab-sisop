#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>

#define DEVICE_NAME "simple_driver"
#define CLASS_NAME  "simple"
#define BUFFER_SIZE 1024

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("DIEGO FRAGA DE MELLO");
MODULE_DESCRIPTION("Simple Linux Character Device Driver with struct list_head");

static int major_number;
static struct class*  simple_class  = NULL;
static struct device* simple_device = NULL;

/* Estrutura para os nós da lista encadeada (Atividade 1) */
struct message_node {
	char data[BUFFER_SIZE];
	size_t len;
	struct list_head list;
};

static LIST_HEAD(msg_list);
static DEFINE_MUTEX(msg_mutex);

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
	.owner   = THIS_MODULE,
	.open    = dev_open,
	.read    = dev_read,
	.write   = dev_write,
	.release = dev_release,
};

static int __init simple_driver_init(void) {
	printk(KERN_INFO "simple_driver: Initializing the simple_driver\n");

	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "simple_driver: Failed to register a major number\n");
		return major_number;
	}
	printk(KERN_INFO "simple_driver: registered with major number %d\n", major_number);

	simple_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(simple_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "simple_driver: Failed to register device class\n");
		return PTR_ERR(simple_class);
	}

	simple_device = device_create(simple_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(simple_device)) {
		class_destroy(simple_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "simple_driver: Failed to create device /dev/%s\n", DEVICE_NAME);
		return PTR_ERR(simple_device);
	}

	printk(KERN_INFO "simple_driver: Device /dev/%s created successfully\n", DEVICE_NAME);
	return 0;
}

static void __exit simple_driver_exit(void) {
	struct message_node *entry, *tmp;

	/* Limpa nós restantes da lista encadeada */
	mutex_lock(&msg_mutex);
	list_for_each_entry_safe(entry, tmp, &msg_list, list) {
		list_del(&entry->list);
		kfree(entry);
	}
	mutex_unlock(&msg_mutex);

	device_destroy(simple_class, MKDEV(major_number, 0));
	class_unregister(simple_class);
	class_destroy(simple_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "simple_driver: Unloaded and cleaned up successfully\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "simple_driver: Device opened\n");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "simple_driver: Device closed\n");
	return 0;
}

/*
 * Atividade 1:
 * Toda vez que a função read() for chamada, a próxima mensagem da lista
 * deve ser removida e devolvida para a aplicação.
 */
static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
	struct message_node *node = NULL;
	size_t to_copy;
	int ret;

	mutex_lock(&msg_mutex);
	if (list_empty(&msg_list)) {
		mutex_unlock(&msg_mutex);
		printk(KERN_INFO "simple_driver: Read called, but list is empty\n");
		return 0; /* EOF / lista vazia */
	}

	/* Obtém o primeiro elemento da lista (FIFO) e o desvincula */
	node = list_first_entry(&msg_list, struct message_node, list);
	list_del(&node->list);
	mutex_unlock(&msg_mutex);

	to_copy = (len < node->len) ? len : node->len;
	ret = copy_to_user(buffer, node->data, to_copy);
	if (ret != 0) {
		kfree(node);
		return -EFAULT;
	}

	printk(KERN_INFO "simple_driver: Read removed message \"%s\" (%zu bytes)\n", node->data, to_copy);
	kfree(node);

	return to_copy;
}

/*
 * Atividade 1:
 * Armazena mensagens recebidas através da chamada write() em uma lista encadeada.
 */
static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
	struct message_node *node;
	size_t to_copy;

	if (len == 0)
		return 0;

	node = kmalloc(sizeof(*node), GFP_KERNEL);
	if (!node)
		return -ENOMEM;

	to_copy = (len > BUFFER_SIZE - 1) ? BUFFER_SIZE - 1 : len;
	if (copy_from_user(node->data, buffer, to_copy)) {
		kfree(node);
		return -EFAULT;
	}

	node->data[to_copy] = '\0';
	node->len = to_copy;

	mutex_lock(&msg_mutex);
	list_add_tail(&node->list, &msg_list);
	mutex_unlock(&msg_mutex);

	printk(KERN_INFO "simple_driver: Stored message \"%s\" (%zu bytes) in linked list\n", node->data, to_copy);
	return to_copy;
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);
