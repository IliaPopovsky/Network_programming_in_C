// paral_calc_TCP_server_IPv4.c
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
   long arg1, arg2;
   
   for(;;){
     if( (n = Readline(sockfd, buf, MAXLINE)) == 0)
        return;            /* соединение закрыто с другого конца */
        
     if(sscanf(buf, "%ld%ld", &arg1, &arg2) == 2)
         snprintf(buf, sizeof(buf), "%ld\n", arg1 + arg2);
     else
         snprintf(buf, sizeof(buf), "input error\n");
         
     n = strlen(buf); 
     Writen(sockfd, buf, n); 
     printf("Ответ сервера отправлен!\n"); 
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

Sigfunc *signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}

static ssize_t my_read(int fd, char *ptr)
{
	static int	read_cnt = 0;
	static char	*read_ptr;
	static char	read_buf[MAXLINE];

	if (read_cnt <= 0) {
again:
		if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (read_cnt == 0)
			return(0);
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	int		n, rc;
	char	c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;	/* newline is stored, like fgets() */
		} else if (rc == 0) {
			if (n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;	/* null terminate like fgets() */
	return(n);
}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}

int main(int argc, char **argv)
{
   int listenfd, connfd;
   pid_t childpid;
   socklen_t clilen, servlen;
   struct sockaddr_in cliaddr, servaddr;
   char buff[MAXLINE];
   
   
   listenfd = Socket(AF_INET, SOCK_STREAM, 0);                         // Создание сокета. Функция socket() создает объект сокета в ядре ОС с помощью системного вызова socket, который имеет номер
                                                                       // 41 для 64-битной ОС Linux и номер 359 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Системный вызов socket() создаёт конечную точку соединения и возвращает файловый дескриптор, указывающий на эту точку. 
                                                                       // Возвращаемый при успешном выполнении файловый дескриптор будет иметь самый маленький номер, который не используется 
                                                                       // процессом.                                     
   bzero(&servaddr, sizeof(servaddr));                                 // Инициализируем всю структуру адреса сервера нулями. Функция bzero() имеет свою реализацию в некоторых ОС. У нас она
                                                                       // имеет вид функционального макроса, который вызывает библиотечную функцию memset() и располагается в файле unp.h
                                                                       // на случай, если в какой-то ОС не будет своей реализации функции bzero().
   servaddr.sin_family = AF_INET;                                      // Присваиваем значение полю структуры адреса сервера, отвечающему за семейство адресов.
   servaddr.sin_port = htons(SERV_PORT);                               // Присваиваем значение полю структуры адреса сервера, отвечающему за номер порта. Библиотечная функция htons() преобразует
                                                                       // двузначный номер порта, принятый в качестве аргумента, в требуемый формат, путем вращения влево битов значения с помощью
                                                                       // инструкции ЦП rol.
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);                       // Библиотечная функция htonl() преобразует переданный ей аргумент с помощью инструкции ЦП bswap и возвращает его.
                                                                       // Мы задаем IP-адрес как INADDR_ANY, что позволяет серверу принимать соединение клиента на любом интерфейсе в том случае,
                                                                       // если узел сервера имеет несколько интерфейсов.
   Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));                  // Связывание адреса сервера с сокетом. Функция bind() использует системный вызов bind, который имеет номер
                                                                       // 49 для 64-битной ОС Linux и номер 361 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Когда сокет создается с помощью socket(), он существует в пространстве имен (семействе адресов), но ему не присвоен адрес.  
                                                                       // Функция bind() присваивает адрес, указанный параметром servaddr, сокету, на который ссылается файловый дескриптор listenfd.
                                                                       // sizeof(servaddr) определяет размер в байтах структуры адреса, на которую указывает servaddr. Традиционно эта операция     
                                                                       // называется «присвоение имени сокету». 
                                                                       // Обычно, сокету типа SOCK_STREAM нужно назначить локальный адрес с помощью bind() до того, как он сможет принимать соединения.
   Listen(listenfd, LISTENQ);                                          // Преобразование сокета в прослушиваемый, т.е. такой, на котором ядро принимает входящие соединения от клиетов.
                                                                       // Функция listen() использует системный вызов listen, который имеет номер 50 для 64-битной ОС Linux и номер 363 для 32-битной
                                                                       // ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Вызов listen() помечает сокет, указанный в listenfd как пассивный, то есть как сокет, который будет использоваться для приёма
                                                                       // запросов входящих соединений с помощью accept(). Аргумент listenfd является файловым дескриптором, который ссылается на сокет
                                                                       // типа SOCK_STREAM.
                                                                       // Константа LISTENQ задает максимальное количество клиентских соединений, которые ядро ставит в очередь на прослушиваемом
                                                                       // сокете. Если приходит запрос на соединение, а очередь полна, то клиент может получить ошибку с указание ECONNREFUSED или,
                                                                       // если низлежащий протокол поддерживает повторную передачу, запрос может быть игнорирован, чтобы попытаться соединиться
                                                                       // позднее.
                                                                       // Эти три этапа, socket, bind и listen, обычны для любого сервера TCP при создании того, что мы называем
                                                                       // прослушиваемым дескриптором (listening descriptor) (в нашем случае это переменная listenfd). 
   
   Signal(SIGCHLD, sig_chld);           /* нужно вызвать waitpid() */                                                                   
   printf("Наш сервер работает!\n");
   printf("Наш сервер имеет PID = %d\n", getpid());
   printf("Наш сервер имеет номер порта = %d\n", ntohs(servaddr.sin_port));
   printf("Наш сервер имеет IPv4-адрес = %d\n", servaddr.sin_addr.s_addr);
   servlen = sizeof(servaddr);
   printf("Наш сервер имеет прослушиваемый сокет listenfd = %d\n", listenfd);
   if(getsockname(listenfd, (SA *)&servaddr, &servlen) < 0)
      printf("Не удалось получить локальный адрес протокола.\n");
   printf("Наш сервер, на прослушиваемом сокете listenfd, имеет номер порта (по getsockname()) = %d\n", htons(servaddr.sin_port));                                                                
   for(;;){
      printf("Цикл нашего сервера работает! (До вызова accept())\n");
      clilen = sizeof(cliaddr); 
      if( (connfd = Accept(listenfd, (SA *) &cliaddr, &clilen)) < 0){  // Прием клиентского соединения. Функция accept() использует системный вызов accept, который имеет номер
                                                                       // 43 для 64-битной ОС Linux, а в 32-битной ОС LINUX его нет, а в других ОС какой-то другой номер.
                                                                       // Системный вызов accept используется с типами сокетов, основанными на соединении (SOCK_STREAM, SOCK_SEQPACKET).
                                                                       // Он извлекает первый запрос на соединение из очереди ожидающих соединений для прослушивающего сокета, listenfd, создаёт новый
                                                                       // подключенный сокет и возвращает новый файловый дескриптор, указывающий на сокет. Новый сокет более не находится в слушающем
                                                                       // состоянии. Исходный сокет listenfd не изменяется при этом системном вызове.
                                                                       // Аргумент listenfd - сокет, который был создан с помощью socket(), привязанный к локальному адресу с помощью bind() и
                                                                       // прослушивающий соединения после listen().
                                                                       // Обычно процесс сервера блокируется при вызове функции accept(), ожидая принятия подключения клиента. Для установки 
                                                                       // TCP-соединения используется трех-этапное рукопожатие (three-way- handshake). Когда рукопожатие состоялось, функция accept()
                                                                       // возвращает значение, и это значение является новым дескриптором (connfd), который называется присоединенным дескриптором
                                                                       // (connected descriptior). Этот новый дескриптор используется для связи с новым клиентом. Новый дескриптор возвращается
                                                                       // функцией accept() для каждого клиента, соединяющегося с нашим сервером.
         if(errno == EINTR)
             continue;                      /* назад к for() */
         else
             err_sys("accept error");
     }
     printf("Наш сервер имеет присоединенный сокет connfd = %d\n", connfd);                                                                  
     printf("connection from %s, port %d\n", 
            inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), 
     ntohs(cliaddr.sin_port));
     
     if( (childpid = fork()) == 0){  /* дочерний процесс */
       printf("Данный дочерний процесс имеет PID = %d\n", getpid());
       if(getsockname(connfd, (SA *)&servaddr, &servlen) < 0)
           printf("Не удалось получить локальный адрес протокола.\n");
       printf("Наш сервер, на присоединенном сокете connfd, имеет номер порта (по getsockname()) = %d\n", htons(servaddr.sin_port));
       Close(listenfd);             /* закрываем прослушиваемый сокет */
       str_echo(connfd);            /* обрабатываем запрос */
       exit(0);
     }
     Close(connfd);                 /* родительский процесс закрывает присоединенный сокет */
      
   }
}
 
 
