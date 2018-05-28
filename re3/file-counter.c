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
	int i, count,sid, pid, status;
	FILE *ct;
	struct sembuf waitsem,sigsem;
	key_t key;

	setbuf(stdout, NULL); /* set stdout to be unbufferd */
	count = 0;
	if((key = ftok(".",1)) == -1){
		perror("ftok");
		exit(1);
	}
	
	if((sid=semget(key, 1 , 0666 | IPC_CREAT)) ==-1){
		perror("semget");
		exit(1);
	}
	semctl(sid, 0 ,SETVAL, 1);
	waitsem.sem_num=0;
	waitsem.sem_op=-1;
	waitsem.sem_flg=0;
	sigsem.sem_num=0;
	sigsem.sem_op=1;
	sigsem.sem_flg=0;

	if((ct = fopen(filename, "w")) == NULL){
		exit(1);
	}

	fprintf(ct, "%d\n", count);
	fclose(ct);
	for(i = 0; i < NUMPROCS; i++){
		if((pid = fork()) == -1){
			perror("fork failed.");
			exit(1);
		}
		if(pid == 0){ /* Child process */
			if(semop(sid,&waitsem,1)==-1){
				perror("sem_wait");
				exit(1);
			}
			count = count1();
			printf("count = %d\n", count);
			if(semop(sid,&sigsem,1)==-1){
				perror("sem_signal");
				exit(1);
			}		
			exit(0);
		}
	}
	for(i = 0; i < NUMPROCS; i++){
		wait(&status);
	}
	exit(0);
}
