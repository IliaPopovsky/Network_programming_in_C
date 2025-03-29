// select_paral_echo_TCP_UDP_UDS_server_IPv4.c
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

int Socket(int family, int type, int protocol)                                 // Наша собственная функция-обертка для функции socket.
{
  int n;
  if( (n = socket(family, type, protocol)) < 0)
       err_sys("socket error");
  return (n);
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)                  // Наша собственная функция-обертка для функции bind.
{
	if (bind(fd, sa, salen) < 0)
		err_sys("bind error");
}

void Listen(int fd, int backlog)                                               // Наша собственная функция-обертка для функции listen.
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		err_sys("listen error");
}

void Write(int fd, void *ptr, size_t nbytes)                                   // Наша собственная функция-обертка для функции write.
{
	if (write(fd, ptr, nbytes) != nbytes)
		err_sys("write error");
}

void Close(int fd)                                                             // Наша собственная функция-обертка для функции close.
{
	if (close(fd) == -1)
		err_sys("close error");
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)                   // Наша собственная функция-обертка для функции accept. 
{
	int		n;

again:
	if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else
			err_sys("accept error");
	}
	return(n);
}



ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}

void str_echo(int sockfd)
{
   ssize_t n;
   char buf[MAXLINE];
   
   for(;;){
     if( (n = read(sockfd, buf, MAXLINE)) == 0)
        return;            /* соединение закрыто с другого конца */
        
     Writen(sockfd, buf, n); 
     printf("Ответ сервера отправлен через сокет TCP!\n"); 
   }
}

void str_echo_uds(int sockfd)
{
   long n;
   char buf[MAXLINE];
   
   for(;;){
     if( (n = read(sockfd, buf, MAXLINE)) <= 0)
     {
        printf("%ld\n", n);
        return; 
     }           
     printf("%ld\n", n); 
     Writen(sockfd, buf, n); 
     printf("Ответ сервера отправлен через сокет UDS!\n");
   }
}

void sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

int
main(int argc, char **argv)
{
	int listenfd, connfd, udpfd, udsfd, nready, maxfdp1, udsconnfd;
	char mesg[MAXLINE];
	pid_t childpid;
	fd_set rset;
	ssize_t n;
	socklen_t len;
	const int on = 1;
	char sock_file[300] = "/tmp/unix_domain_socket";
	struct sockaddr_in	cliaddr, servaddr;
	struct sockaddr_un     servaddr_un; 
	
       /* 4create listening TCP socket */
       listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

		/* 4create UDP socket */
	udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(udpfd, (SA *) &servaddr, sizeof(servaddr));
        
        unlink(sock_file);
        udsfd = Socket(AF_UNIX, SOCK_STREAM, 0);
        
        bzero(&servaddr_un, sizeof(servaddr_un));
        servaddr_un.sun_family = AF_UNIX;                                      
        strncpy(servaddr_un.sun_path, sock_file, sizeof(servaddr_un.sun_path) - 1);
        
        Bind(udsfd, (SA *) &servaddr_un, sizeof(servaddr_un));
        Listen(udsfd, LISTENQ);                                     // Делаем сокет udsfd прослушиваемым, чтобы когда число установленных соединений ненулевое, функция select() показала готовность 
                                                                    // данного сокета для чтения.
        signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	FD_ZERO(&rset);
	maxfdp1 = max(listenfd, udpfd);
	maxfdp1 = max(maxfdp1, udsfd) + 1;
	printf("listenfd = %d, udpfd = %d, udsfd = %d, maxfdp1 = %d\n", listenfd, udpfd, udsfd, maxfdp1);
	for ( ; ; ) {
	        printf("Наш сервер работает!\n");
		FD_SET(listenfd, &rset);
		FD_SET(udpfd, &rset);
		FD_SET(udsfd, &rset);
		if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("select error");
		}
               printf("nready = %d\n", nready);
		if (FD_ISSET(listenfd, &rset)) {
			len = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &len);
	
			if ( (childpid = fork()) == 0) {	/* child process */
				Close(listenfd);	/* close listening socket */
				str_echo(connfd);	/* process the request */
				//printf("Ответ сервера отправлен через сокет TCP!\n"); 
				exit(0);
			}
			Close(connfd);			/* parent closes connected socket */
		}

		if (FD_ISSET(udpfd, &rset)) {
			len = sizeof(cliaddr);
			n = recvfrom(udpfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);

			sendto(udpfd, mesg, n, 0, (SA *) &cliaddr, len);
			printf("Ответ сервера отправлен через сокет UDP!\n");
		}
		#if 0                                       
		if (FD_ISSET(udsfd, &rset)) {                                    // В данном варианте, без использования функции Listen(udsfd, LISTENQ) для данного сокета, получается, что сокет udsfd
		       str_echo_uds(udsfd);                                      // всегда готов для чтения, сразу с момента его создания. Получается случай, который не описывается ни в одном
		       //printf("Ответ сервера отправлен через сокет UDS!\n");   // из 4 возможных условий готовности сокета для чтения, которые использует функция select()? Или это случай ошибки
		}                                                                // сокета, ожидающей обработки (условие 4 ошибка сокета, ожидающая обработки). В данном варианте клиент UDS 
		#endif                                                           // получает ошибку connect error: Connection refused, клиенты TCP, UDP работают нормально.
		#if 1
		if (FD_ISSET(udsfd, &rset)) {                                    // В данном варианте, с использованием функции Listen(udsfd, LISTENQ) для данного сокета, получается, что сокет udsfd
		       udsconnfd = Accept(udsfd, NULL, NULL);                    // готов для чтения, только когда число установленных соединений ненулевое. (условие 3 сокет является прослушиваемым
	                                                                         // и число установленных соединений ненулевое.
			if ( (childpid = fork()) == 0) {	/* child process */
				Close(udsfd);	/* close listening socket */
				str_echo_uds(udsconnfd);	/* process the request */
				//printf("Ответ сервера отправлен через сокет UDS!\n"); 
				exit(0);
			}
			Close(udsconnfd);			/* parent closes connected socket */
		}
		#endif
	}
}
