#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>

#ifndef SCHED_IDLE
#define SCHED_IDLE 5
#endif

#ifndef SCHED_DEADLINE
#define SCHED_DEADLINE 6
#endif

#ifndef SCHED_LOW_IDLE
#define SCHED_LOW_IDLE 7
#endif

volatile int running = 1;

void *worker_thread(void *data)
{
	long thread_id = (long)data;
	unsigned long long counter = 0;
	while (running) {
		counter++;
	}
	printf("[Thread %ld] finalizou com %llu iteracoes\n", thread_id, counter);
	return NULL;
}

void print_sched(int policy)
{
	int priority_min, priority_max;

	switch (policy) {
		case SCHED_DEADLINE:
			printf("SCHED_DEADLINE (6)");
			break;
		case SCHED_FIFO:
			printf("SCHED_FIFO (1)");
			break;
		case SCHED_RR:
			printf("SCHED_RR (2)");
			break;
		case SCHED_OTHER:
			printf("SCHED_OTHER/NORMAL (0)");
			break;
		case SCHED_BATCH:
			printf("SCHED_BATCH (3)");
			break;
		case SCHED_IDLE:
			printf("SCHED_IDLE (5)");
			break;
		case SCHED_LOW_IDLE:
			printf("SCHED_LOW_IDLE (7)");
			break;
		default:
			printf("Desconhecida (%d)", policy);
			break;
	}
	priority_min = sched_get_priority_min(policy);
	priority_max = sched_get_priority_max(policy);
	printf(" | PRI_MIN: %d PRI_MAX: %d\n", priority_min, priority_max);
}

int parse_policy(const char *str)
{
	if (strcasecmp(str, "SCHED_LOW_IDLE") == 0 || strcasecmp(str, "low_idle") == 0 || strcmp(str, "7") == 0)
		return SCHED_LOW_IDLE;
	if (strcasecmp(str, "SCHED_IDLE") == 0 || strcasecmp(str, "idle") == 0 || strcmp(str, "5") == 0)
		return SCHED_IDLE;
	if (strcasecmp(str, "SCHED_NORMAL") == 0 || strcasecmp(str, "SCHED_OTHER") == 0 || strcasecmp(str, "normal") == 0 || strcasecmp(str, "other") == 0 || strcmp(str, "0") == 0)
		return SCHED_OTHER;
	if (strcasecmp(str, "SCHED_FIFO") == 0 || strcasecmp(str, "fifo") == 0 || strcmp(str, "1") == 0)
		return SCHED_FIFO;
	if (strcasecmp(str, "SCHED_RR") == 0 || strcasecmp(str, "rr") == 0 || strcmp(str, "2") == 0)
		return SCHED_RR;
	if (strcasecmp(str, "SCHED_BATCH") == 0 || strcasecmp(str, "batch") == 0 || strcmp(str, "3") == 0)
		return SCHED_BATCH;
	if (strcasecmp(str, "SCHED_DEADLINE") == 0 || strcasecmp(str, "deadline") == 0 || strcmp(str, "6") == 0)
		return SCHED_DEADLINE;
	return atoi(str);
}

int set_thread_priority(pthread_t *thr, int newpolicy, int newpriority)
{
	int policy, ret;
	struct sched_param param;

	int min_prio = sched_get_priority_min(newpolicy);
	int max_prio = sched_get_priority_max(newpolicy);

	if (min_prio >= 0 && max_prio >= 0) {
		if (newpriority > max_prio || newpriority < min_prio) {
			fprintf(stderr, "Prioridade invalida para a politica: MIN: %d, MAX: %d\n", min_prio, max_prio);
			return -1;
		}
	}

	pthread_getschedparam(*thr, &policy, &param);
	printf("Politica anterior da thread: ");
	print_sched(policy);

	param.sched_priority = newpriority;
	ret = pthread_setschedparam(*thr, newpolicy, &param);
	if (ret != 0) {
		fprintf(stderr, "pthread_setschedparam falhou: %s\n", strerror(ret));
		return -1;
	}

	pthread_getschedparam(*thr, &policy, &param);
	printf("Nova politica da thread:     ");
	print_sched(policy);

	return 0;
}

int set_pid_priority(pid_t pid, int newpolicy, int newpriority)
{
	int policy;
	struct sched_param param;

	policy = sched_getscheduler(pid);
	if (policy < 0) {
		perror("sched_getscheduler falhou");
		return -1;
	}
	printf("Politica anterior do PID %d: ", pid);
	print_sched(policy);

	param.sched_priority = newpriority;
	if (sched_setscheduler(pid, newpolicy, &param) < 0) {
		perror("sched_setscheduler falhou");
		return -1;
	}

	policy = sched_getscheduler(pid);
	printf("Nova politica do PID %d:     ", pid);
	print_sched(policy);

	return 0;
}

void usage(const char *prog)
{
	printf("Uso:\n");
	printf("  %s pid <PID> <politica> [prioridade=0]\n", prog);
	printf("      Modifica a politica de escalonamento de um processo existente via sched_setscheduler.\n");
	printf("  %s <politica> [prioridade=0] [tempo_exec_segundos=3]\n", prog);
	printf("      Cria uma thread, altera sua politica e a executa pelo tempo especificado.\n\n");
	printf("Politicas disponiveis:\n");
	printf("  0 | SCHED_NORMAL / SCHED_OTHER\n");
	printf("  1 | SCHED_FIFO\n");
	printf("  2 | SCHED_RR\n");
	printf("  3 | SCHED_BATCH\n");
	printf("  5 | SCHED_IDLE\n");
	printf("  7 | SCHED_LOW_IDLE  <-- Nova politica implementada (Tutorial 3.1)\n\n");
	printf("Exemplos:\n");
	printf("  %s pid 123 SCHED_LOW_IDLE\n", prog);
	printf("  %s SCHED_LOW_IDLE 0 5\n", prog);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "pid") == 0) {
		if (argc < 4) {
			usage(argv[0]);
			return 1;
		}
		pid_t pid = (pid_t)atoi(argv[2]);
		int policy = parse_policy(argv[3]);
		int priority = (argc >= 5) ? atoi(argv[4]) : 0;
		return set_pid_priority(pid, policy, priority);
	}

	int policy = parse_policy(argv[1]);
	int priority = (argc >= 3) ? atoi(argv[2]) : 0;
	int timesleep = (argc >= 4) ? atoi(argv[3]) : 3;

	pthread_t thr;
	printf("Criando thread de teste...\n");
	pthread_create(&thr, NULL, worker_thread, (void *)1);

	printf("Aplicando politica para a thread...\n");
	if (set_thread_priority(&thr, policy, priority) < 0) {
		running = 0;
		pthread_join(thr, NULL);
		return 1;
	}

	printf("Executando thread por %d segundos...\n", timesleep);
	sleep(timesleep);
	running = 0;
	pthread_join(thr, NULL);
	printf("Teste concluido com sucesso!\n");

	return 0;
}
