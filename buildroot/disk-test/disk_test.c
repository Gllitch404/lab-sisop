#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_LENGTH 256
#define DISC_SZ       1073741824ULL /* 1 GB */
#define SECTOR_SZ     512
#define DEVICE_DEFAULT "/dev/sdb"

static void print_usage(const char *prog) {
	printf("Uso do utilitario de teste de disco (Tutorial 2.4):\n");
	printf("  %s                           : Executa o padrao do tutorial (escreve \"Hello World!\" no inicio de /dev/sdb)\n", prog);
	printf("  %s read [bytes] [offset]     : Le [bytes] a partir de [offset] em /dev/sdb\n", prog);
	printf("  %s write <texto> [offset]    : Escreve <texto> a partir de [offset] em /dev/sdb\n", prog);
	printf("  %s sector <offset>           : Calcula o setor correspondente ao deslocamento (offset / 512)\n", prog);
	printf("  %s writesec <setor> <texto>  : Escreve <texto> no inicio do setor especificado\n", prog);
	printf("  %s readsec <setor> [qtd]     : Le [qtd] setores a partir do setor especificado\n", prog);
	printf("  %s dropcache                 : Executa 'echo 3 > /proc/sys/vm/drop_caches'\n", prog);
}

/* Executa exatamente a operacao descrita na Secao 4 do Tutorial 2.4 */
static int run_tutorial_default(const char *dev) {
	int fd;
	char buf[BUFFER_LENGTH] = "Hello World!";
	ssize_t written;

	printf("Abrindo dispositivo de bloco %s em modo O_RDWR...\n", dev);
	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}

	/* Posicionar-se no inicio do disco. */
	if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		perror("Failed to seek to beginning of disk");
		close(fd);
		return errno;
	}

	/* Executa escrita. */
	written = write(fd, buf, strlen(buf));
	if (written < 0) {
		perror("Failed to write to the device");
		close(fd);
		return errno;
	}

	close(fd);

	printf("Sucesso: \"%s\" escrito no inicio do disco (%zd bytes escritos).\n", buf, written);
	printf("Para verificar na maquina host execute:\n  hexdump -C sdb.bin | head\n");
	return 0;
}

static int run_read(const char *dev, size_t count, off_t offset) {
	int fd;
	unsigned char *buf;
	ssize_t bytes_read;
	size_t i;

	fd = open(dev, O_RDONLY);
	if (fd < 0) {
		perror("Failed to open the device for read");
		return errno;
	}

	if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
		perror("Failed to seek");
		close(fd);
		return errno;
	}

	buf = malloc(count + 1);
	if (!buf) {
		fprintf(stderr, "Memoria insuficiente\n");
		close(fd);
		return ENOMEM;
	}

	bytes_read = read(fd, buf, count);
	if (bytes_read < 0) {
		perror("Failed to read");
		free(buf);
		close(fd);
		return errno;
	}
	close(fd);

	printf("Lidos %zd bytes a partir do offset %lld (setor %lld):\n",
	       bytes_read, (long long)offset, (long long)(offset / SECTOR_SZ));

	/* Hexdump simples */
	for (i = 0; i < (size_t)bytes_read; i += 16) {
		size_t j;
		printf("%08llx  ", (long long)(offset + i));
		for (j = 0; j < 16; j++) {
			if (i + j < (size_t)bytes_read)
				printf("%02x ", buf[i + j]);
			else
				printf("   ");
			if (j == 7) printf(" ");
		}
		printf(" |");
		for (j = 0; j < 16 && (i + j) < (size_t)bytes_read; j++) {
			unsigned char c = buf[i + j];
			printf("%c", isprint(c) ? c : '.');
		}
		printf("|\n");
	}

	free(buf);
	return 0;
}

static int run_write(const char *dev, const char *text, off_t offset) {
	int fd;
	ssize_t written;
	size_t len = strlen(text);

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("Failed to open the device for write");
		return errno;
	}

	if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
		perror("Failed to seek");
		close(fd);
		return errno;
	}

	written = write(fd, text, len);
	if (written < 0) {
		perror("Failed to write");
		close(fd);
		return errno;
	}
	close(fd);

	printf("Sucesso: %zd bytes escritos no offset %lld (setor %lld): \"%s\"\n",
	       written, (long long)offset, (long long)(offset / SECTOR_SZ), text);
	return 0;
}

static int run_drop_cache(void) {
	int fd;
	printf("Esvaziando cache de disco (echo 3 > /proc/sys/vm/drop_caches)...\n");
	sync();
	fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	if (fd < 0) {
		perror("Falha ao abrir /proc/sys/vm/drop_caches (requer root)");
		return errno;
	}
	if (write(fd, "3\n", 2) < 0) {
		perror("Falha ao escrever em /proc/sys/vm/drop_caches");
		close(fd);
		return errno;
	}
	close(fd);
	printf("Cache de disco esvaziada com sucesso!\n");
	return 0;
}

int main(int argc, char *argv[]) {
	const char *dev = DEVICE_DEFAULT;

	if (argc <= 1) {
		return run_tutorial_default(dev);
	}

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
		print_usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "read") == 0) {
		size_t count = (argc >= 3) ? (size_t)strtoul(argv[2], NULL, 0) : 32;
		off_t offset = (argc >= 4) ? (off_t)strtoull(argv[3], NULL, 0) : 0;
		return run_read(dev, count, offset);
	}

	if (strcmp(argv[1], "write") == 0) {
		if (argc < 3) {
			fprintf(stderr, "Erro: informe o texto a ser escrito.\n");
			print_usage(argv[0]);
			return 1;
		}
		const char *text = argv[2];
		off_t offset = (argc >= 4) ? (off_t)strtoull(argv[3], NULL, 0) : 0;
		return run_write(dev, text, offset);
	}

	if (strcmp(argv[1], "sector") == 0) {
		if (argc < 3) {
			fprintf(stderr, "Erro: informe o offset em bytes.\n");
			return 1;
		}
		unsigned long long offset = strtoull(argv[2], NULL, 0);
		unsigned long long sector = offset / SECTOR_SZ;
		unsigned long long rem = offset % SECTOR_SZ;
		printf("Offset: %llu bytes\n", offset);
		printf("Tamanho do setor: %d bytes\n", SECTOR_SZ);
		printf("Setor calculado: %llu (offset exato no setor: byte %llu)\n", sector, rem);
		printf("Exemplo do tutorial: %llu / %d = %.2f (Setor %llu)\n",
		       offset, SECTOR_SZ, (double)offset / SECTOR_SZ, sector);
		return 0;
	}

	if (strcmp(argv[1], "writesec") == 0) {
		if (argc < 4) {
			fprintf(stderr, "Erro: uso: %s writesec <setor> <texto>\n", argv[0]);
			return 1;
		}
		unsigned long long sector = strtoull(argv[2], NULL, 0);
		const char *text = argv[3];
		return run_write(dev, text, (off_t)(sector * SECTOR_SZ));
	}

	if (strcmp(argv[1], "readsec") == 0) {
		unsigned long long sector = (argc >= 3) ? strtoull(argv[2], NULL, 0) : 0;
		size_t num_sectors = (argc >= 4) ? (size_t)strtoul(argv[3], NULL, 0) : 1;
		return run_read(dev, num_sectors * SECTOR_SZ, (off_t)(sector * SECTOR_SZ));
	}

	if (strcmp(argv[1], "dropcache") == 0) {
		return run_drop_cache();
	}

	fprintf(stderr, "Comando desconhecido: %s\n", argv[1]);
	print_usage(argv[0]);
	return 1;
}
