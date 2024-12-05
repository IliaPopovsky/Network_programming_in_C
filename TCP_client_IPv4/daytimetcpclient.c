// daytimetcpclient.c
#include "unp.h"
#include <stdarg.h>
#include <syslog.h>		                                                /* for syslog() */

int daemon_proc;		                                                /* set nonzero by daemon_init() */

static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
	int errno_save, n;
	char buf[MAXLINE];

	errno_save = errno;		                                        /* value caller might want printed */
#ifdef	HAVE_VSNPRINTF
	vsnprintf(buf, sizeof(buf), fmt, ap);	                                 /* this is safe */
#else
	vsprintf(buf, fmt, ap);					          /* this is not safe */
#endif
	n = strlen(buf);
	if (errnoflag)
		snprintf(buf+n, sizeof(buf)-n, ": %s", strerror(errno_save));
	strcat(buf, "\n");

	if (daemon_proc) {
		syslog(level, buf);
	} else {
		fflush(stdout);		                                 /* in case stdout and stderr are the same */
		fputs(buf, stderr);
		fflush(stderr);
	}
	return;
}

void err_quit(const char *fmt, ...)
{
	va_list ap;
       va_start(ap, fmt);
	err_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_sys(const char *fmt, ...)
{
	va_list ap;
        va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}



int main(int argc, char **argv)
{
   int sockfd, n;
   char recvline[MAXLINE + 1];
   struct sockaddr_in servaddr;
   
   if(argc != 2)
       err_quit("usage: a.out <IPaddress>");
   
   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
       err_sys("socket error");
       
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(13);  /* сервер времени и даты */
   if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
      err_quit("inet_pton error for %s", argv[1]);
   
   if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
      err_sys("connect error");
      
   while((n = read(sockfd, recvline, MAXLINE)) > 0)
   {
      recvline[n] = 0; /* завершающий нуль */
      if(fputs(recvline, stdout) == EOF)
          err_sys("fputs error");
   }
   if(n < 0)
      err_sys("read error");
      
   exit(0);
}
