/*
 * ============================================================================
 * PUCRS - Escola Politécnica - Ciência da Computação
 * Disciplina: Laboratório de Sistemas Operacionais
 * Trabalho Prático: sched_profiler
 * 
 * Autores:
 *   - Diego Fraga de Mello
 *   - Vitor Rafael Borges
 * ============================================================================
 */

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
static pthread_barrier_t start_barrier;

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

static const char *policy_to_filename(int policy)
{
    switch (policy) {
        case SCHED_OTHER:    return "other";
        case SCHED_FIFO:     return "fifo";
        case SCHED_RR:       return "rr";
        case SCHED_BATCH:    return "batch";
        case SCHED_IDLE:     return "idle";
        case SCHED_DEADLINE: return "deadline";
        case SCHED_LOW_IDLE: return "low_idle";
        default:             return "custom";
    }
}

static const char *policy_to_arg_name(int policy)
{
    switch (policy) {
        case SCHED_OTHER:    return "SCHED_OTHER";
        case SCHED_FIFO:     return "SCHED_FIFO";
        case SCHED_RR:       return "SCHED_RR";
        case SCHED_BATCH:    return "SCHED_BATCH";
        case SCHED_IDLE:     return "SCHED_IDLE";
        case SCHED_DEADLINE: return "SCHED_DEADLINE";
        case SCHED_LOW_IDLE: return "SCHED_LOW_IDLE";
        default:             return "SCHED_OTHER";
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
    if (ret != 0 && ret != EPERM) {
        fprintf(stderr, "[Thread %c] Erro pthread_setschedparam: %s\n",
                data->character, strerror(ret));
    }

    /* Sincroniza o inicio: aguarda todas as threads serem criadas antes de competir */
    pthread_barrier_wait(&start_barrier);

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

        /* Consumo de CPU para que a fatia de tempo (quantum) expire naturalmente no Linux */
        volatile int k;
        for (k = 0; k < 10000; k++) {
            /* queima de ciclos de processamento */
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int buffer_size = DEFAULT_BUFFER_SIZE;
    int num_threads = DEFAULT_NUM_THREADS;
    int policy = DEFAULT_POLICY;
    int show_raw = 0; /* Padrao: 0 (--showpos: nao imprimir buffer antes do pos-processamento) */
    int record_trace = 0;
    int i;

    if (argc < 4) {
        printf("Uso: %s <buffer_size> <num_threads> <policy> [--showpos | --showpre] [--record]\n", argv[0]);
        printf("Politicas: SCHED_LOW_IDLE, SCHED_IDLE, SCHED_FIFO, SCHED_RR, SCHED_OTHER\n");
        printf("Opcoes:\n");
        printf("  --showpos   Exibe apenas a saida apos o pos-processamento (padrao)\n");
        printf("  --showpre   Exibe tambem o buffer bruto antes do pos-processamento\n");
        printf("  --record    Grava o trace do kernel automaticamente via trace-cmd\n");
        printf("Exemplo: %s 10000 4 SCHED_RR --showpos --record\n", argv[0]);
        if (argc == 1) {
            printf("\nNenhum parametro fornecido. Utilizando valores padrao:\n");
            printf("  Buffer: %d | Threads: %d | Politica: %s | Exibicao: %s | Record: %s\n\n",
                   buffer_size, num_threads, policy_to_string(policy),
                   show_raw ? "--showpre" : "--showpos",
                   record_trace ? "Sim" : "Nao");
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

    /* Processa argumentos adicionais e flags opcionais */
    for (i = 4; i < argc; i++) {
        if (strcasecmp(argv[i], "--record") == 0 || strcasecmp(argv[i], "-record") == 0) {
            record_trace = 1;
        } else if (strcasecmp(argv[i], "--showpre") == 0 || strcasecmp(argv[i], "-showpre") == 0 || strcmp(argv[i], "1") == 0) {
            show_raw = 1;
        } else if (strcasecmp(argv[i], "--showpos") == 0 || strcasecmp(argv[i], "-showpos") == 0 || strcmp(argv[i], "0") == 0) {
            show_raw = 0;
        }
    }

    /* Se a opcao --record foi solicitada, aciona o trace-cmd de forma transparente */
    if (record_trace) {
        char cmd[512];
        const char *pname = policy_to_filename(policy);
        const char *parg = (argc >= 4) ? argv[3] : policy_to_arg_name(policy);
        const char *flag_show = show_raw ? "--showpre" : "--showpos";
        const char *out_dir = (access("/root", W_OK) == 0) ? "/root" : ".";

        snprintf(cmd, sizeof(cmd),
                 "trace-cmd record -e sched:sched_switch -o %s/trace_%s.dat %s %d %d %s %s",
                 out_dir, pname, argv[0], buffer_size, num_threads, parg, flag_show);

        printf("============================================================\n");
        printf(" [sched_profiler] Modo --record ativado!\n");
        printf(" Gravando trace do kernel em: %s/trace_%s.dat\n", out_dir, pname);
        printf(" Executando comando de gravacao:\n   %s\n", cmd);
        printf("============================================================\n\n");

        int ret = system(cmd);
        if (ret == 0) {
            printf("\n============================================================\n");
            printf(" [sched_profiler] Gravacao concluida com sucesso!\n");
            printf(" Arquivo de trace gerado: %s/trace_%s.dat\n", out_dir, pname);
            printf("============================================================\n");
        } else {
            fprintf(stderr, "\n>> Erro ao executar trace-cmd (codigo de retorno %d).\n", ret);
        }
        return ret;
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
    memset(global_buffer, 0, global_size + 1);

    /* Inicializa semaforo binario compartilhado entre threads */
    if (sem_init(&sem_buffer, 0, 1) != 0) {
        perror("Erro ao inicializar semaforo binario");
        free(global_buffer);
        return EXIT_FAILURE;
    }

    /* Barreira de sincronizacao inicial para todas as threads */
    if (pthread_barrier_init(&start_barrier, NULL, num_threads) != 0) {
        perror("Erro ao inicializar barreira de threads");
        free(global_buffer);
        sem_destroy(&sem_buffer);
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
        pthread_barrier_destroy(&start_barrier);
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

    /* 1. Saida sem pos-processamento (apenas se show_raw == 1) */
    if (show_raw) {
        printf("Saida sem pos-processamento:\n");
        for (i = 0; i < global_size; i++) {
            putchar(global_buffer[i]);
            if ((i + 1) % 60 == 0) {
                putchar('\n');
            }
        }
        if (global_size % 60 != 0) {
            putchar('\n');
        }
        printf("\n");
    }

    /* 2. Pos-processamento do Buffer Global */
    char *aggregated_summary = (char *)malloc(global_size + 1);
    int agg_len = 0;
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
    pthread_barrier_destroy(&start_barrier);

    return EXIT_SUCCESS;
}
