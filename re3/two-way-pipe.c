#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFSIZE 256

int main(int argc, char *argv[]){
	char buf[BUFSIZE];
	int ctop[2],ptoc[2];
	int pid, msglen, status;
	if(argc != 3){
		printf("bad argument.\n");
		exit(1);
	}
	if(pipe(ptoc) == -1){
		perror("pipe failed.");
		exit(1);
	}
	if(pipe(ctop) == -1){
		perror("pipe failed.");
		exit(1);
	}
	if((pid = fork()) == -1){
		perror("fork failed.");
		exit(1);
	}
	if(pid == 0){ /* Child process */
		close(ctop[0]);
		close(ptoc[1]);
		msglen = strlen(argv[1]) + 1;
		if(write(ctop[1], argv[1], msglen) == -1){
			perror("pipe write.");
			exit(1);
		}
		if(read(ptoc[0], buf, BUFSIZE) == -1){
			perror("pipe read.");
			exit(1);
		}
		printf("message from parent process: \n");
		printf("\t%s\n", buf);

		exit(0);
	}else{ /* Parent process */
		close(ptoc[0]);
		close(ctop[1]);
		msglen = strlen(argv[2]) + 1;
		if(write(ptoc[1], argv[2], msglen) == -1){
			perror("pipe write.");
			exit(1);
		}
		if(read(ctop[0], buf, BUFSIZE) == -1){
			perror("pipe read.");
			exit(1);
		}
		printf("message from child process: \n");
		printf("\t%s\n", buf);
		wait(&status);
	}
}
