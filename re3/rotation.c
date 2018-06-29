#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
	int A[100], B[100];
	srand(time(NULL));

	for (int i = 0; i < 100; i++)
	{
		A[i] = rand() % 10000;
		B[i] = rand() % 10000;
	}

	int sid, pid, status;
	key_t key;

	setbuf(stdout, NULL); /* set stdout to be unbufferd */
	if ((key = ftok(".", 1)) == -1)
	{
		perror("ftok");
		exit(1);
	}

	if ((sid = semget(key, 2, 0666 | IPC_CREAT)) == -1)
	{
		perror("semget");
		exit(1);
	}
	semctl(sid, 0, SETVAL, 1);
	semctl(sid, 1, SETVAL, 0);

	struct sembuf waitsem1,waitsem2,sigsem1, sigsem2;
	waitsem1.sem_num = 0;
	waitsem1.sem_op = -1;
	waitsem1.sem_flg = 0;
	sigsem1.sem_num = 0;
	sigsem1.sem_op = 1;
	sigsem1.sem_flg = 0;
	waitsem2.sem_num = 1;
	waitsem2.sem_op = -1;
	waitsem2.sem_flg = 0;
	sigsem2.sem_num = 1;
	sigsem2.sem_op = 1;
	sigsem2.sem_flg = 0;

	if ((pid = fork()) == -1)
	{
		perror("fork failed.");
		exit(1);
	}
	if (pid == 0)
	{ /* Child process */
		for (int i = 0; i < 100; i++)
		{
			if (semop(sid, &waitsem2, 1) == -1)
			{
				perror("sem_wait2");
				exit(1);
			}
			printf("B[%d] = %d\n", i, B[i]);
			if (semop(sid, &sigsem1, 1) == -1)
			{
				perror("sem_signal1");
				exit(1);
			}
		}
	}
	else
	{
		for (int i = 0; i < 100; i++)
		{
			if (semop(sid, &waitsem1, 1) == -1)
			{
				perror("sem_wait1");
				exit(1);
			}
			printf("A[%d] = %d\n", i, A[i]);
			if (semop(sid, &sigsem2, 1) == -1)
			{
				perror("sem_signal2");
				exit(1);
			}
		}
		wait(&status);
	}
}
