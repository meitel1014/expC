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

	if ((sid = semget(key, 1, 0666 | IPC_CREAT)) == -1)
	{
		perror("semget");
		exit(1);
	}
	semctl(sid, 0, SETVAL, 1);

	struct sembuf waitsem, sigsem;
	waitsem.sem_num = 0;
	waitsem.sem_op = -1;
	waitsem.sem_flg = 0;
	sigsem.sem_num = 0;
	sigsem.sem_op = 1;
	sigsem.sem_flg = 0;

	if ((pid = fork()) == -1)
	{
		perror("fork failed.");
		exit(1);
	}
	if (pid == 0)
	{ /* Child process */
		for (int i = 0; i < 100; i++)
		{
			if (semop(sid, &waitsem, 1) == -1)
			{
				perror("sem_wait");
				exit(1);
			}
			printf("B[%d] = %d\n", i, B[i]);
			if (semop(sid, &sigsem, 1) == -1)
			{
				perror("sem_signal");
				exit(1);
			}
			exit(0);
		}
	}
	else
	{
		for (int i = 0; i < 100; i++)
		{
			if (semop(sid, &waitsem, 1) == -1)
			{
				perror("sem_wait");
				exit(1);
			}
			printf("A[%d] = %d\n", i, A[i]);
			if (semop(sid, &sigsem, 1) == -1)
			{
				perror("sem_signal");
				exit(1);
			}
			exit(0);
		}
	}
}