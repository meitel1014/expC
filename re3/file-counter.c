#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#define NUMPROCS 4
char filename[] = "counter";

int count1(){
	FILE *ct;
	int count;
	if((ct = fopen(filename, "r")) == NULL)
		exit(1);
	fscanf(ct, "%d\n", &count);
	count++;
	fclose(ct);
	if((ct = fopen(filename, "w")) == NULL)
		exit(1);
	fprintf(ct, "%d\n", count);
	fclose(ct);
	return count;
}

int main(){
	int i, count, pid, status;
	FILE *ct;
	setbuf(stdout, NULL); /* set stdout to be unbufferd */
	count = 0;
	if((ct = fopen(filename, "w")) == NULL)
		exit(1);
	fprintf(ct, "%d\n", count);
	fclose(ct);
	for(i = 0; i < NUMPROCS; i++){
		if((pid = fork()) == -1){
			perror("fork failed.");
			exit(1);
		}
		if(pid == 0){ /* Child process */
			count = count1();
			printf("count = %d\n", count);
			exit(0);
		}
	}
	for(i = 0; i < NUMPROCS; i++){
		wait(&status);
	}
	exit(0);
}
