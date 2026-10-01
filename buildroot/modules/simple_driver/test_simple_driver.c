#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEVICE_PATH "/dev/simple_driver"
#define BUFFER_SIZE 1024

void menu(void) {
	printf("\n========================================\n");
	printf("       TESTE DO SIMPLE DRIVER           \n");
	printf("========================================\n");
	printf("1. Escrever mensagem no driver (write)\n");
	printf("2. Ler mensagem do driver (read)\n");
	printf("3. Teste automatico da lista encadeada (Atividade 1)\n");
	printf("4. Sair\n");
	printf("Escolha uma opcao: ");
}

int do_write(const char *msg) {
	int fd = open(DEVICE_PATH, O_WRONLY);
	if (fd < 0) {
		perror("Falha ao abrir " DEVICE_PATH " para escrita");
		return -1;
	}
	ssize_t ret = write(fd, msg, strlen(msg));
	if (ret < 0) {
		perror("Falha na escrita");
		close(fd);
		return -1;
	}
	printf("Sucesso: %zd bytes escritos: \"%s\"\n", ret, msg);
	close(fd);
	return 0;
}

int do_read(void) {
	char buf[BUFFER_SIZE];
	int fd = open(DEVICE_PATH, O_RDONLY);
	if (fd < 0) {
		perror("Falha ao abrir " DEVICE_PATH " para leitura");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	ssize_t ret = read(fd, buf, sizeof(buf) - 1);
	if (ret < 0) {
		perror("Falha na leitura");
		close(fd);
		return -1;
	}
	if (ret == 0) {
		printf("Fila vazia: nenhuma mensagem para ler.\n");
	} else {
		printf("Mensagem lida (%zd bytes): \"%s\"\n", ret, buf);
	}
	close(fd);
	return 0;
}

int auto_test(void) {
	printf("\n--- Iniciando Teste da Lista Encadeada (Atividade 1) ---\n");
	printf("Enviando mensagem 1: \"Primeira mensagem na lista\"\n");
	do_write("Primeira mensagem na lista");
	printf("Enviando mensagem 2: \"Segunda mensagem na lista\"\n");
	do_write("Segunda mensagem na lista");
	printf("Enviando mensagem 3: \"Terceira mensagem na lista\"\n");
	do_write("Terceira mensagem na lista");

	printf("\nLendo mensagens de volta (esperada ordem FIFO):\n");
	printf("Leitura 1: ");
	do_read();
	printf("Leitura 2: ");
	do_read();
	printf("Leitura 3: ");
	do_read();
	printf("Leitura 4 (deve estar vazia): ");
	do_read();
	printf("--- Teste concluido com sucesso! ---\n\n");
	return 0;
}

int main(int argc, char **argv) {
	if (argc >= 2) {
		if (strcmp(argv[1], "write") == 0) {
			const char *msg = (argc >= 3) ? argv[2] : "Mensagem padrao";
			return do_write(msg);
		} else if (strcmp(argv[1], "read") == 0) {
			return do_read();
		} else if (strcmp(argv[1], "test") == 0) {
			return auto_test();
		} else {
			printf("Uso: %s [write <msg> | read | test]\n", argv[0]);
			return 1;
		}
	}

	int op = 0;
	char input[256];

	while (1) {
		menu();
		if (!fgets(input, sizeof(input), stdin))
			break;
		op = atoi(input);

		switch (op) {
		case 1:
			printf("Digite a mensagem a ser enviada: ");
			if (fgets(input, sizeof(input), stdin)) {
				input[strcspn(input, "\r\n")] = 0;
				do_write(input);
			}
			break;
		case 2:
			do_read();
			break;
		case 3:
			auto_test();
			break;
		case 4:
			printf("Encerrando test_simple_driver.\n");
			return 0;
		default:
			printf("Opcao invalida!\n");
			break;
		}
	}
	return 0;
}

