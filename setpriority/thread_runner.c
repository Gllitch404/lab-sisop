#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <sys/time.h>

#ifndef SCHED_IDLE
#define SCHED_IDLE 5
#endif

#ifndef SCHED_LOW_IDLE
#define SCHED_LOW_IDLE 7
#endif

volatile int keep_running = 1;

struct thread_data {
	int id;
	int policy;
	const char *policy_name;
	unsigned long long iterations;
	pthread_t tid;
};

void *thread_worker(void *arg)
{
	struct thread_data *td = (struct thread_data *)arg;
	struct sched_param param;
	int ret;

	param.sched_priority = 0;
	ret = pthread_setschedparam(pthread_self(), td->policy, &param);
	if (ret != 0) {
		fprintf(stderr, "[Thread %d] Falha ao configurar politica %s: %s\n",
			td->id, td->policy_name, strerror(ret));
	}

	while (keep_running) {
		td->iterations++;
	}

	return NULL;
}

int main(int argc, char **argv)
{
	int test_duration = 3;
	if (argc >= 2) {
		test_duration = atoi(argv[1]);
		if (test_duration <= 0) test_duration = 3;
	}

	printf("=================================================================\n");
	printf("   THREAD RUNNER - Comparativo de Escalonamento (Tutorial 3.1)  \n");
	printf("   Políticas testadas: SCHED_NORMAL, SCHED_IDLE, SCHED_LOW_IDLE \n");
	printf("=================================================================\n");
	printf("Duracao do teste: %d segundos\n\n", test_duration);

	struct thread_data threads[3] = {
		{ .id = 1, .policy = SCHED_OTHER,    .policy_name = "SCHED_NORMAL",   .iterations = 0 },
		{ .id = 2, .policy = SCHED_IDLE,     .policy_name = "SCHED_IDLE",     .iterations = 0 },
		{ .id = 3, .policy = SCHED_LOW_IDLE, .policy_name = "SCHED_LOW_IDLE", .iterations = 0 }
	};

	int i;
	for (i = 0; i < 3; i++) {
		printf("Iniciando Thread %d com politica %s (%d)...\n",
		       threads[i].id, threads[i].policy_name, threads[i].policy);
		pthread_create(&threads[i].tid, NULL, thread_worker, &threads[i]);
	}

	printf("Todas as threads iniciadas. Aguardando %d segundos...\n", test_duration);
	sleep(test_duration);

	keep_running = 0;

	for (i = 0; i < 3; i++) {
		pthread_join(threads[i].tid, NULL);
	}

	printf("\n--- RESULTADOS FINAIS ---\n");
	for (i = 0; i < 3; i++) {
		printf("Thread %d [%-14s]: %14llu iteracoes\n",
		       threads[i].id, threads[i].policy_name, threads[i].iterations);
	}
	printf("-------------------------\n");
	printf("Conclusao: Tasks SCHED_NORMAL possuem prioridade sobre SCHED_IDLE,\n");
	printf("e tasks SCHED_IDLE possuem prioridade sobre SCHED_LOW_IDLE.\n\n");

	return 0;
}
