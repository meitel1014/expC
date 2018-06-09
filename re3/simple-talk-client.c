#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define PORT 10130
#define NAME_LEN 128
#define TIMEOUT 10

void myalarm(int sec);
void timeout(int);

int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in svr;
	struct hostent *sp;
	char rbuf[1024];
	int nbytes;
	int reuse;
	char my_name[128], sname[128];

	if(argc != 2){
		fprintf(stderr, "Usage: simple-talk-server hostname\n");
		exit(1);
	}

	if(signal(SIGALRM, timeout) == SIG_ERR){
		perror("signal failed.");
		exit(1);
	}

	printf("Enter your name>");
	fgets(my_name, sizeof(my_name), stdin);
	for(int i = 0; my_name[i] != '\0'; i++){
		if(my_name[i] == '\n')
			my_name[i] = '\0';
	}

	/* ソケットの生成 */
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket");
		exit(1);
	}
	/* ソケットアドレス再利用の指定 */
	reuse = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
		perror("setsockopt");
		exit(1);
	}

	/* svr( ソケットの接続先 ) の情報設定 */
	bzero(&svr, sizeof(svr));
	svr.sin_family = AF_INET;
	svr.sin_port = htons(PORT);
	if((sp = gethostbyname(argv[1])) == NULL){
		fprintf(stderr, "unknown host %s\n", argv[1]);
		exit(1);
	}
	bcopy(sp->h_addr, &svr.sin_addr, sp->h_length);
	if(connect(sock, (struct sockaddr *)&svr, sizeof(svr)) == -1){
		perror("connect");
		exit(1);
	}

	write(sock, my_name, strlen(my_name));
	while((nbytes = read(sock, rbuf, sizeof(rbuf))) < 0){
	}
	rbuf[nbytes] = '\0';
	strcpy(sname, rbuf);

	printf("connected with %s\n", sname);

	for(;;){
		write(1, my_name, strlen(my_name));
		write(1, " > ", 4);
		fd_set rfds; /* select() で用いるファイル記述子集合 */
		/* 入力を監視するファイル記述子の集合を変数 rfds にセットする */
		FD_ZERO(&rfds); /* rfds を空集合に初期化 */
		FD_SET(0, &rfds); /* 標準入力 */
		FD_SET(sock, &rfds); /* クライアントを受け付けたソケット */
		myalarm(TIMEOUT);
		/* 標準入力とソケットからの受信を同時に監視する */
		if(select(sock + 1, &rfds, NULL, NULL, NULL) > 0){
			if(FD_ISSET(0, &rfds)){ /* 標準入力から入力があったなら */
				/* 標準入力から読み込みクライアントに送信 */
				if((nbytes = read(0, rbuf, sizeof(rbuf))) < 0){
					perror("read");
				}else if(nbytes == 0){
					printf("\e[1K\r");
					break;
				}else{
					write(sock, rbuf, nbytes);
				}
			}

			if(FD_ISSET(sock, &rfds)){ /* ソケットから受信したなら */
				printf("\e[1K\r");
				/* ソケットから読み込み端末に出力 */
				if((nbytes = read(sock, rbuf, sizeof(rbuf))) < 0){
					perror("read");
				}else if(nbytes == 0){
					break;
				}else{
					rbuf[nbytes] = '\0';
					printf("%s > %s", sname, rbuf);
				}
			}
		}else{
			if(errno == EINTR){
				close(sock);
				printf("\e[1K\rconnection timed out.\n");
				exit(0);
			}
		}
	}

	close(sock);
	printf("closed\n");
}

void myalarm(int sec){
	static pid_t pid = 0;
	/* ゾンビを作成しない*/
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0 /*SA_NOCLDWAIT*/;
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

void timeout(int sig){
}
