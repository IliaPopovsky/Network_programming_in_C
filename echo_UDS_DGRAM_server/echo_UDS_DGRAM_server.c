// echo_UDS_DGRAM_server.c
#include	"unp.h"

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int n;
	socklen_t len;
	char mesg[MAXLINE];

	for ( ; ; ) {
	        printf("Наш сервер работает!\n");
		len = clilen;
		n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		sendto(sockfd, mesg, n, 0, pcliaddr, len);
		printf("Ответ сервера отправлен!\n");
	}
}

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_un servaddr, cliaddr;

	sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
       
        unlink(UNIXDG_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXDG_PATH);
	
	bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
}


