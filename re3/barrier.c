#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc,char *argv[]){
	int sid, pid, status,procs;
	char msg[128];
	struct sembuf waitsem,sigsem;
	key_t key;
	
	if (argc != 2) {
		fprintf(stderr,"Usage: barrier procs\n");
		exit(1);
	}
	
	procs=atoi(argv[1]);

	setbuf(stdout, NULL); /* set stdout to be unbufferd */
	if((key = ftok(".",1)) == -1){
		perror("ftok");
		exit(1);
	}
	
	if((sid=semget(key, 1 , 0666 | IPC_CREAT)) ==-1){
		perror("semget");
		exit(1);
	}
	semctl(sid, 0 ,SETVAL, 0);
	waitsem.sem_num=0;
	waitsem.sem_op=-procs;
	waitsem.sem_flg=0;
	sigsem.sem_num=0;
	sigsem.sem_op=1;
	sigsem.sem_flg=0;

	for(int i = 0; i < procs; i++){
		if((pid = fork()) == -1){
			perror("fork failed.");
			exit(1);
		}
		if(pid == 0){ /* Child process */
			sleep(getpid()%5);
			printf("Child process %d ended\n",getpid());
			
			if(semop(sid,&sigsem,1)==-1){
				perror("sem_signal");
				exit(1);
			}		
			exit(0);
		}
	}

	if(semop(sid,&waitsem,1)==-1){
		perror("sem_wait");
		exit(1);
	}
	
	printf("All child processes ended\n");
	
	for(int i = 0; i < procs; i++){
		wait(&status);
	}
	
	return 0;
}
