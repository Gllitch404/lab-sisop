#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/xtea_driver"
#define BUFFER_SIZE 4096

int send_command(const char *cmd, char *response, size_t resp_size) {
	int fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("Erro ao abrir " DEVICE_PATH);
		return -1;
	}

	ssize_t written = write(fd, cmd, strlen(cmd));
	if (written < 0) {
		perror("Erro na escrita para " DEVICE_PATH);
		close(fd);
		return -1;
	}

	memset(response, 0, resp_size);
	ssize_t read_bytes = read(fd, response, resp_size - 1);
	if (read_bytes < 0) {
		perror("Erro na leitura de " DEVICE_PATH);
		close(fd);
		return -1;
	}

	response[strcspn(response, "\r\n")] = 0;
	close(fd);
	return 0;
}

int main(int argc, char **argv) {
	char response[BUFFER_SIZE];
	char enc_cmd[BUFFER_SIZE];
	char dec_cmd[BUFFER_SIZE];

	printf("====================================================\n");
	printf("        TESTE DO XTEA DRIVER (Atividade 2 & Desafio)\n");
	printf("====================================================\n");

	if (argc >= 2) {
		/* Envio direto de comando customizado */
		char custom_cmd[BUFFER_SIZE] = {0};
		int i;
		for (i = 1; i < argc; i++) {
			strcat(custom_cmd, argv[i]);
			if (i < argc - 1) strcat(custom_cmd, " ");
		}
		printf("Enviando comando: %s\n", custom_cmd);
		if (send_command(custom_cmd, response, sizeof(response)) == 0) {
			printf("Resposta do driver: %s\n", response);
		}
		return 0;
	}

	/* Teste 1: Atividade 2 (formato completo especificado no tutorial) */
	const char *key0 = "f0e1d2c3";
	const char *key1 = "b4a59687";
	const char *key2 = "78695a4b";
	const char *key3 = "3c2d1e0f";
	int data_size = 16;
	const char *original_data = "aabbccddeeff00112233445566778899";

	printf("\n[Teste 1: Atividade 2 - Criptografia com parametros na chamada]\n");
	snprintf(enc_cmd, sizeof(enc_cmd), "enc %s %s %s %s %d %s",
		 key0, key1, key2, key3, data_size, original_data);
	printf("1. Comando: %s\n", enc_cmd);
	if (send_command(enc_cmd, response, sizeof(response)) < 0) {
		return 1;
	}
	printf("   Texto cifrado recebido: %s\n", response);

	char cipher_copy[BUFFER_SIZE];
	strncpy(cipher_copy, response, sizeof(cipher_copy));

	printf("2. Testando decriptografia com a mesma chave...\n");
	snprintf(dec_cmd, sizeof(dec_cmd), "dec %s %s %s %s %d %s",
		 key0, key1, key2, key3, data_size, cipher_copy);
	printf("   Comando: %s\n", dec_cmd);
	if (send_command(dec_cmd, response, sizeof(response)) < 0) {
		return 1;
	}
	printf("   Texto decifrado recebido: %s\n", response);
	if (strncmp(response, original_data, strlen(original_data)) == 0) {
		printf("   >>> SUCESSO! O texto decifrado confere perfeitamente com o original.\n");
	} else {
		printf("   >>> ERRO: O texto decifrado difere do original!\n");
	}

	/* Teste 2: Desafio (usando as chaves carregadas via modprobe) */
	printf("\n[Teste 2: Desafio - Criptografia usando chaves do modprobe]\n");
	snprintf(enc_cmd, sizeof(enc_cmd), "enc %d %s", data_size, original_data);
	printf("1. Comando simplificado: %s\n", enc_cmd);
	if (send_command(enc_cmd, response, sizeof(response)) < 0) {
		return 1;
	}
	printf("   Texto cifrado recebido: %s\n", response);
	strncpy(cipher_copy, response, sizeof(cipher_copy));

	snprintf(dec_cmd, sizeof(dec_cmd), "dec %d %s", data_size, cipher_copy);
	printf("2. Comando simplificado: %s\n", dec_cmd);
	if (send_command(dec_cmd, response, sizeof(response)) < 0) {
		return 1;
	}
	printf("   Texto decifrado recebido: %s\n", response);
	if (strncmp(response, original_data, strlen(original_data)) == 0) {
		printf("   >>> SUCESSO! Desafio concluido com sucesso.\n");
	} else {
		printf("   >>> ERRO: Decriptografia do Desafio falhou!\n");
	}

	return 0;
}
