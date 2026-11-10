#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>

#ifndef SCHED_IDLE
#define SCHED_IDLE 5
#endif

#ifndef SCHED_DEADLINE
#define SCHED_DEADLINE 6
#endif

#ifndef SCHED_LOW_IDLE
#define SCHED_LOW_IDLE 7
#endif

#define DEFAULT_BUFFER_SIZE 10000
#define DEFAULT_NUM_THREADS 4
#define DEFAULT_POLICY      SCHED_RR

/* Estrutura compartilhada global */
static char *global_buffer = NULL;
static int global_size = 0;
static int global_index = 0;
static sem_t sem_buffer;

/* Estrutura de contexto para cada thread */
typedef struct {
    int id;
    char character;
    int policy;
    int priority;
} thread_data_t;

/* Converte nome textual de politica para o codigo inteiro */
static int parse_policy_string(const char *str)
{
    if (!str) return DEFAULT_POLICY;
    if (strcasecmp(str, "SCHED_LOW_IDLE") == 0 || strcasecmp(str, "low_idle") == 0 || strcmp(str, "7") == 0)
        return SCHED_LOW_IDLE;
    if (strcasecmp(str, "SCHED_IDLE") == 0 || strcasecmp(str, "idle") == 0 || strcmp(str, "5") == 0)
        return SCHED_IDLE;
    if (strcasecmp(str, "SCHED_FIFO") == 0 || strcasecmp(str, "fifo") == 0 || strcmp(str, "1") == 0)
        return SCHED_FIFO;
    if (strcasecmp(str, "SCHED_RR") == 0 || strcasecmp(str, "rr") == 0 || strcmp(str, "2") == 0)
        return SCHED_RR;
    if (strcasecmp(str, "SCHED_OTHER") == 0 || strcasecmp(str, "other") == 0 ||
        strcasecmp(str, "SCHED_NORMAL") == 0 || strcasecmp(str, "normal") == 0 || strcmp(str, "0") == 0)
        return SCHED_OTHER;
    if (strcasecmp(str, "SCHED_BATCH") == 0 || strcasecmp(str, "batch") == 0 || strcmp(str, "3") == 0)
        return SCHED_BATCH;
    if (strcasecmp(str, "SCHED_DEADLINE") == 0 || strcasecmp(str, "deadline") == 0 || strcmp(str, "6") == 0)
        return SCHED_DEADLINE;

    return atoi(str);
}

static const char *policy_to_string(int policy)
{
    switch (policy) {
        case SCHED_OTHER:    return "SCHED_OTHER (0)";
        case SCHED_FIFO:     return "SCHED_FIFO (1)";
        case SCHED_RR:       return "SCHED_RR (2)";
        case SCHED_BATCH:    return "SCHED_BATCH (3)";
        case SCHED_IDLE:     return "SCHED_IDLE (5)";
        case SCHED_DEADLINE: return "SCHED_DEADLINE (6)";
        case SCHED_LOW_IDLE: return "SCHED_LOW_IDLE (7)";
        default:             return "DESCONHECIDA";
    }
}

/* Funcao executada por cada thread */
static void *thread_worker(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    struct sched_param param;
    int ret;

    /* Configura politica de escalonamento */
    param.sched_priority = data->priority;
    ret = pthread_setschedparam(pthread_self(), data->policy, &param);
    if (ret != 0) {
        if (ret == EPERM) {
            fprintf(stderr, "[Thread %c] Aviso: Permissao negada (EPERM) para politica %s. (Execute como root/sudo para FIFO/RR).\n",
                    data->character, policy_to_string(data->policy));
        } else {
            fprintf(stderr, "[Thread %c] Erro pthread_setschedparam: %s\n",
                    data->character, strerror(ret));
        }
    }

    /* Loop de escrita no buffer compartilhado sincronizado com semaforo binario */
    while (1) {
        if (sem_wait(&sem_buffer) != 0) {
            perror("sem_wait falhou");
            break;
        }

        if (global_index >= global_size) {
            sem_post(&sem_buffer);
            break;
        }

        global_buffer[global_index] = data->character;
        global_index++;

        sem_post(&sem_buffer);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int buffer_size = DEFAULT_BUFFER_SIZE;
    int num_threads = DEFAULT_NUM_THREADS;
    int policy = DEFAULT_POLICY;
    int i;

    if (argc < 4) {
        printf("Uso: %s <buffer_size> <num_threads> <policy>\n", argv[0]);
        printf("Politicas suportadas: SCHED_LOW_IDLE, SCHED_IDLE, SCHED_FIFO, SCHED_RR, SCHED_OTHER\n");
        printf("Exemplo: %s 10000 4 SCHED_RR\n", argv[0]);
        if (argc == 1) {
            printf("\nNenhum parametro fornecido. Utilizando valores padrao para demonstracao:\n");
            printf("  Buffer: %d | Threads: %d | Politica: %s\n\n",
                   buffer_size, num_threads, policy_to_string(policy));
        }
    }

    if (argc >= 2) {
        buffer_size = atoi(argv[1]);
        if (buffer_size <= 0) buffer_size = DEFAULT_BUFFER_SIZE;
    }
    if (argc >= 3) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0) num_threads = DEFAULT_NUM_THREADS;
    }
    if (argc >= 4) {
        policy = parse_policy_string(argv[3]);
    }

    printf("============================================================\n");
    printf("   Laboratorio de Sistemas Operacionais - sched_profiler   \n");
    printf("============================================================\n");
    printf("Configuracao:\n");
    printf("  Tamanho do Buffer:  %d bytes\n", buffer_size);
    printf("  Numero de Threads:  %d\n", num_threads);
    printf("  Politica Escolhida: %s\n", policy_to_string(policy));
    printf("============================================================\n\n");

    /* Alocacao do buffer global */
    global_size = buffer_size;
    global_index = 0;
    global_buffer = (char *)malloc(global_size + 1);
    if (!global_buffer) {
        perror("Erro ao alocar global_buffer");
        return EXIT_FAILURE;
    }

    /* Inicializa semaforo binario compartilhado entre threads */
    if (sem_init(&sem_buffer, 0, 1) != 0) {
        perror("Erro ao inicializar semaforo binario");
        free(global_buffer);
        return EXIT_FAILURE;
    }

    /* Determina prioridade adequada conforme a politica */
    int priority = 0;
    if (policy == SCHED_FIFO || policy == SCHED_RR) {
        int min_p = sched_get_priority_min(policy);
        priority = (min_p > 0) ? min_p : 1;
    }

    /* Cria os dados e threads */
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_data_t *tdata = (thread_data_t *)malloc(num_threads * sizeof(thread_data_t));
    if (!threads || !tdata) {
        perror("Erro ao alocar memoria para threads");
        free(global_buffer);
        sem_destroy(&sem_buffer);
        return EXIT_FAILURE;
    }

    for (i = 0; i < num_threads; i++) {
        tdata[i].id = i;
        tdata[i].character = (char)('A' + (i % 26));
        tdata[i].policy = policy;
        tdata[i].priority = priority;

        if (pthread_create(&threads[i], NULL, thread_worker, &tdata[i]) != 0) {
            perror("Erro em pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* Aguarda todas as threads finalizarem */
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    global_buffer[global_size] = '\0';

    /* 1. Saida sem pos-processamento (Buffer Bruto) */
    printf("Saida sem pos-processamento:\n");
    puts(global_buffer);
    printf("\n");

    /* 2. Pos-processamento do Buffer Global */
    /* Aloca espaco para o buffer resumido (no maximo global_size + 1) */
    char *aggregated_summary = (char *)malloc(global_size + 1);
    int agg_len = 0;

    /* Contagem de vezes que cada thread foi escalonada (bursts de execucao) */
    int *thread_burst_counts = (int *)calloc(num_threads, sizeof(int));
    if (!aggregated_summary || !thread_burst_counts) {
        perror("Erro ao alocar estruturas de pos-processamento");
        return EXIT_FAILURE;
    }

    char prev_char = '\0';
    for (i = 0; i < global_size; i++) {
        char cur = global_buffer[i];
        if (cur != prev_char) {
            aggregated_summary[agg_len++] = cur;
            /* Identifica o indice da thread pelo caracter */
            int tid = cur - 'A';
            if (tid >= 0 && tid < num_threads) {
                thread_burst_counts[tid]++;
            }
            prev_char = cur;
        }
    }
    aggregated_summary[agg_len] = '\0';

    /* Saida apos pos-processamento */
    printf("Saida apos pos-processamento:\n");
    puts(aggregated_summary);
    for (i = 0; i < num_threads; i++) {
        char c = (char)('A' + (i % 26));
        printf("%c = %d\n", c, thread_burst_counts[i]);
    }
    printf("\n");

    /* Limpeza de recursos */
    free(aggregated_summary);
    free(thread_burst_counts);
    free(threads);
    free(tdata);
    free(global_buffer);
    sem_destroy(&sem_buffer);

    return EXIT_SUCCESS;
}
