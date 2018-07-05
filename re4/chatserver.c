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
#define MAXCLIENTS 5

typedef struct socklist {
	int csock;
	char username[101];
	struct socklist* next;
} SOCKLIST;

SOCKLIST* sockhead = NULL;
int csocknum = 0;
int time_up = 0;

SOCKLIST* add_sock(int csock) {
	SOCKLIST* new = malloc(sizeof(SOCKLIST));
	new->csock = csock;
	new->next = sockhead;
	sockhead = new;
	csocknum++;
	return new;
}

int remove_sock(int csock) {
	SOCKLIST* tmp = sockhead;
	SOCKLIST* prev = NULL;
	while(tmp != NULL) {
		if(tmp->csock == csock) {
			if(prev != NULL) {
				prev->next = tmp->next;
			}
			free(tmp);
			csocknum--;
			sockhead = prev;
			return 0;
		}
		prev = tmp;
		tmp = tmp->next;
	}

	return -1;
}

void broadcast(char* msg) {
	SOCKLIST* bsock = sockhead;
	while(bsock != NULL) {
		write(bsock->csock, msg, strlen(msg));
		bsock = bsock->next;
	}
}

void timeup(int sig) {
	time_up = 1;
}

int main(int argc, char* argv[]) {
	int sock;
	struct sockaddr_in svr, clt;
	char rbuf[1024];
	int nbytes;

	// s1 (初期状態)
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
	/* client 受付用ソケットの情報設定 */
	bzero(&svr, sizeof(svr));
	svr.sin_family = AF_INET;
	svr.sin_addr.s_addr = htonl(INADDR_ANY);
	/* 受付側の IP アドレスは任意 */
	svr.sin_port = htons(PORT); /* ポート番号 10140 を介して受け付ける */
	/* ソケットにソケットアドレスを割り当てる */
	if(bind(sock, (struct sockaddr*)&svr, sizeof(svr)) < 0) {
		perror("bind");
		exit(1);
	}
	/* 待ち受けクライアント数の設定 */
	if(listen(sock, 5) < 0) { /* 待ち受け数に 5 を指定 */
		perror("listen");
		exit(1);
	}

	signal(SIGINT, SIG_IGN);

	for(;;) {
		// s2 (入力待ち)
		int maxfd = sock;
		fd_set rfds; /* select() で用いるファイル記述子集合 */
		/* 入力を監視するファイル記述子の集合を変数 rfds にセットする */
		FD_ZERO(&rfds);      /* rfds を空集合に初期化 */
		FD_SET(sock, &rfds); /* クライアントを受け付けるソケット */
		SOCKLIST* tmp = sockhead;
		while(tmp != NULL) {
			FD_SET(tmp->csock, &rfds); /* クライアントを受け付けたソケット */
			if(tmp->csock > maxfd) {
				maxfd = tmp->csock;
			}
			tmp = tmp->next;
		}
		// s3 (入力処理)
		/* ソケットからの受信を同時に監視する */
		if(select(maxfd + 1, &rfds, NULL, NULL, NULL) > 0) {
			// s4 (参加受付)
			if(FD_ISSET(sock, &rfds)) {
				/* クライアントの受付 */
				int csock;
				socklen_t clen = sizeof(clt);
				if((csock = accept(sock, (struct sockaddr*)&clt, &clen)) < 0) {
					perror("accept");
					exit(1);
				}
				if(csocknum >= MAXCLIENTS) {
					write(csock, "REQUEST REJECTED\n", 17);
					close(csock);
					continue;
				}

				write(csock, "REQUEST ACCEPTED\n", 17);
				SOCKLIST* newsock = add_sock(csock);
				// s5 (ユーザ名登録)
				nbytes = read(csock, rbuf, sizeof(rbuf));
				rbuf[nbytes - 1] = '\0';
				SOCKLIST* tmp = sockhead;
				while(tmp != NULL) {
					if(strcmp(tmp->username, rbuf) == 0) {
						write(csock, "USERNAME REJECTED\n", 17);
						printf("REJECTED:%s\n", rbuf);
						close(csock);
						remove_sock(csock);
						break;
					}
					tmp = tmp->next;
				}
				strcpy(newsock->username, rbuf);
				write(csock, "USERNAME REGISTERED\n", 20);
				printf("NEW USER:%s\n", newsock->username);
				continue;
			}
			// s6 (メッセージ配信)
			SOCKLIST* msgsock = sockhead;
			while(msgsock != NULL) {
				if(FD_ISSET(msgsock->csock,
							&rfds)) { /* ソケットから受信したなら */
					printf("\e[1K\r");
					if((nbytes = read(msgsock->csock, rbuf, sizeof(rbuf)))
					   < 0) {
						perror("read");
					} else if(nbytes == 0) {
						// s7 (離脱処理)
						close(msgsock->csock);
						printf("client %s closed\n", msgsock->username);
						char closed_name[101];
						strcpy(closed_name, msgsock->username);
						remove_sock(msgsock->csock);

						char buf[1024] = "";
						sprintf(buf, "%s exited\n", closed_name);
						broadcast(buf);
						break;
					} else {
						char buf[1024] = "";
						rbuf[nbytes] = '\0';
						if(strcmp(rbuf, "/list\n") == 0) {
							SOCKLIST* member = sockhead;
							while(member != NULL) {
								strcat(buf, member->username);
								strcat(buf, "\n");
								member = member->next;
							}
							broadcast(buf);
						} else if(strncmp(rbuf, "/send ", 6) == 0) {
							char targetname[101], msg[1024];
							sscanf("/send %s %s", targetname, msg);
							SOCKLIST* target = sockhead;
							while(target != NULL) {
								if(strcmp(target->username, targetname) == 0) {
									break;
								}
								target = target->next;
							}
							if(target == NULL) {
								write(msgsock->csock, "No such user\n", 14);
							} else {
								sprintf(buf, "%s*>%s", msgsock->username, msg);
								write(target->csock, buf, strlen(buf));
							}
						} else {
							sprintf(buf, "%s >%s", msgsock->username, rbuf);
							broadcast(buf);
						}
						break;
					}
				}
			}
			msgsock = msgsock->next;
		} else {
			if(timeup) {
				SOCKLIST* csock = sockhead;
				while(csock != NULL) {
					close(csock->csock);
				}
				close(sock);
				exit(0);
			} else if(errno == EINTR) {
				alarm(10);
				signal(SIGALRM, timeup);

				broadcast("Server >10 seconds remaining\n");
			}
		}
	}
}
