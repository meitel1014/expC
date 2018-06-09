#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFSIZE 256
#define TIMEOUT 10

void myalarm(int sec){
	static pid_t pid = 0;
	/* ゾンビを作成しない*/
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_NOCLDWAIT;
	if(pid != 0){
		kill(pid, SIGKILL);
	}

	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		exit(1);
	}

	if((pid = fork()) == -1){
		perror("fork");
		exit(1);
	}
	if(pid == 0){ /* Child process */
		sleep(sec);
		kill(getppid(), SIGALRM);
		exit(0);
	}else{
		return;
	}
}

void timeout(){
	printf("This program is timeout.\n");
	exit(0);
}

int main(){
	char buf[BUFSIZE];
	if(signal(SIGALRM, timeout) == SIG_ERR){
		perror("signal failed.");
		exit(1);
	}
	myalarm(TIMEOUT);
	while(fgets(buf, BUFSIZE, stdin) != NULL){
		printf("echo: %s", buf);
		myalarm(TIMEOUT);
	}
}
