#include <stdio.h>
#include <string.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

#define SYSCALL_PROCESSINFO	385
#define SYSCALL_SLEEPPROCESSES	386
#define SYSCALL_PRINTMESSAGE	387

void usage(char* s){
	printf("Usage:\n");
	printf("  %s <PID>             : listProcessInfo (Syscall 385)\n", s);
	printf("  %s sleep             : listSleepProcesses (Syscall 386 - Desafio 1)\n", s);
	printf("  %s msg <mensagem>    : printMessage to kernel log (Syscall 387 - Desafio 2)\n", s);
	exit(0);
}

int main(int argc, char** argv){  
	char buf[4096];
	long ret;
	
	if(argc < 2){
		usage(argv[0]);
	}
	
	if(strcmp(argv[1], "sleep") == 0 || strcmp(argv[1], "-s") == 0) {
		printf("Invoking 'listSleepProcesses' system call (Desafio 1).\n");
		ret = syscall(SYSCALL_SLEEPPROCESSES, buf, sizeof(buf));
		if(ret >= 0) {
			printf("Sleeping processes:\n%s\n", buf);
			printf("Total bytes returned: %ld\n", ret);
		} else {
			printf("System call 'listSleepProcesses' did not execute as expected error %ld\n", ret);
		}
		return 0;
	}

	if(strcmp(argv[1], "msg") == 0 || strcmp(argv[1], "-m") == 0) {
		char *msg = (argc >= 3) ? argv[2] : "Ola do espaco de usuario!";
		printf("Invoking 'printMessage' system call (Desafio 2) with message: \"%s\"\n", msg);
		ret = syscall(SYSCALL_PRINTMESSAGE, msg, strlen(msg) + 1);
		if(ret >= 0) {
			printf("Success! Message logged to kernel dmesg. Return: %ld\n", ret);
		} else {
			printf("System call 'printMessage' did not execute as expected error %ld\n", ret);
		}
		return 0;
	}

	int pid = atoi(argv[1]);
	
	printf("Invoking 'listProcessInfo' system call.\n");
         
	ret = syscall(SYSCALL_PROCESSINFO, pid, buf, sizeof(buf)); 
         
	if(ret > 0) {
		/* Success, show the process info. */
		printf("%s\n", buf);
	}
	else {
		printf("System call 'listProcessInfo' did not execute as expected error %ld\n", ret);
	}
          
	return 0;
}
