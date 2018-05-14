#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 10138

int main(int argc,char *argv[]) {
	int sock,csock;
	struct sockaddr_in svr;
	struct sockaddr_in clt;
	struct hostent *sp;
	int clen;
	char rbuf[1024];
	int nbytes;
	int reuse;
	
	if (argc != 2) {
		fprintf(stderr,"Usage: echoclient hostname\n");
		exit(1);
	}
	/* ソケットの生成 */
	if ((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
		perror("socket");
		exit(1);
	}
	/* ソケットアドレス再利用の指定 */
	reuse=1;
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0) {
		perror("setsockopt");
		exit(1);
	}

	/* svr( ソケットの接続先 ) の情報設定 */
	bzero(&svr,sizeof(svr));
	svr.sin_family=AF_INET;
	svr.sin_port=htons(PORT);
	if ( ( sp = gethostbyname(argv[1]) ) == NULL ) {
		fprintf(stderr,"unknown host %s\n",argv[1]);
		exit(1);
	}
	bcopy(sp->h_addr,&svr.sin_addr,sp->h_length);
	if(connect(sock,(struct sockaddr*)&svr,sizeof(svr))==-1){
		perror("connect");
		exit(1);
	}
	
	while(fgets(rbuf,sizeof(rbuf),stdin)>0){
		write(sock,rbuf,strlen(rbuf));
		while(read(sock,rbuf,sizeof(rbuf))<0){}
		write(1,rbuf,strlen(rbuf));
	}
	
	close(sock);
}
