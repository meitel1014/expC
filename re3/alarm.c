#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFSIZE 256
#define TIMEOUT 10

void myalarm(int sec){
	alarm(sec);
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
