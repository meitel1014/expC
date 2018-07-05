#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 10140

int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in svr;
	struct hostent *sp;
	char rbuf[1024];
	int nbytes;

	if(argc != 3) {
		fprintf(stderr, "Usage: chatclient hostname username\n");
		exit(1);
	}
	setbuf(stdout, NULL); /* set stdout to be unbufferd */

	// c1 (初期状態)
	/* ソケットの生成 */
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	/* ソケットアドレス再利用の指定 */
	int reuse = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt");
		exit(1);
	}

	/* svr( ソケットの接続先 ) の情報設定 */
	bzero(&svr, sizeof(svr));
	svr.sin_family = AF_INET;
	svr.sin_port = htons(PORT);
	if((sp = gethostbyname(argv[1])) == NULL) {
		fprintf(stderr, "unknown host %s\n", argv[1]);
		exit(1);
	}
	bcopy(sp->h_addr, &svr.sin_addr, sp->h_length);
	if(connect(sock, (struct sockaddr *)&svr, sizeof(svr)) == -1) {
		perror("connect");
		exit(1);
	}

	// c2 (参加)
	read(sock, rbuf, 17);
	if(strncmp(rbuf, "REQUEST ACCEPTED\n", 17) != 0) {
		goto C6;
	}
	puts("REQUEST ACCEPTED");

	// c3 (ユーザ名登録)
	char my_name[101];
	strncpy(my_name, argv[2], 100);
	my_name[strlen(my_name)] = '\n';
	write(sock, my_name, strlen(my_name));
	nbytes = read(sock, rbuf, 20);
	if(strncmp(rbuf, "USERNAME REGISTERED\n", 20) != 0) {
		goto C6;
	}
	puts("USERNAME REGISTERED");

	// c4 (メッセージ送受信)
	for(;;) {
		printf("%s >", argv[2]);
		fd_set rfds; /* select() で用いるファイル記述子集合 */
		/* 入力を監視するファイル記述子の集合を変数 rfds にセットする */
		FD_ZERO(&rfds);      /* rfds を空集合に初期化 */
		FD_SET(0, &rfds);    /* 標準入力 */
		FD_SET(sock, &rfds); /* クライアントを受け付けたソケット */
		/* 標準入力とソケットからの受信を同時に監視する */
		if(select(sock + 1, &rfds, NULL, NULL, NULL) > 0) {
			if(FD_ISSET(0, &rfds)) { /* 標準入力から入力があったなら */
				/* 標準入力から読み込みサーバに送信 */
				if((nbytes = read(0, rbuf, sizeof(rbuf))) < 0) {
					perror("read");
				} else if(nbytes == 0) {
					printf("\e[1K\r");
					break;
				} else {
					printf("\e[1A\e[2K\r");
					write(sock, rbuf, nbytes);
				}
			}

			if(FD_ISSET(sock, &rfds)) { /* ソケットから受信したなら */
				printf("\e[1K\r");
				/* ソケットから読み込み端末に出力 */
				if((nbytes = read(sock, rbuf, sizeof(rbuf))) < 0) {
					perror("read");
				} else if(nbytes == 0) {
					break;
				} else {
					rbuf[nbytes] = '\0';
					printf("%s", rbuf);
				}
			}
		} else {
			close(sock);
			printf("\e[1K\rconnection timed out.\n");
			exit(0);
		}
	}

	// c5 (離脱)
	close(sock);
	printf("closed\n");
	exit(0);

// c6 (例外処理)
C6:
	close(sock);
	exit(1);
}
