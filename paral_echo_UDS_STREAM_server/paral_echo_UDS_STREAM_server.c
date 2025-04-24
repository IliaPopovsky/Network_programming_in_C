// paral_echo_UDS_STREAM_server.c
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

void sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		printf("child %d terminated\n", pid);
	}
	return;
}

Sigfunc *
signal(int signo, Sigfunc *func)
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

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}

void str_echo(int sockfd)
{
   ssize_t n;
   char buf[MAXLINE];
   
   for(;;){
     if( (n = read(sockfd, buf, MAXLINE)) == 0)
        return;            /* соединение закрыто с другого конца */
        
     Writen(sockfd, buf, n); 
     printf("Ответ сервера отправлен!\n"); 
   }
}


int main(int argc, char **argv)
{
   int listenfd, connfd;
   pid_t childpid;
   socklen_t clilen;
   struct sockaddr_un cliaddr, servaddr;
   void sig_chld(int);
   
   listenfd = Socket(AF_LOCAL, SOCK_STREAM, 0);                        // Создание сокета. Функция socket() создает объект сокета в ядре ОС с помощью системного вызова socket, который имеет номер
                                                                       // 41 для 64-битной ОС Linux и номер 359 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Системный вызов socket() создаёт конечную точку соединения и возвращает файловый дескриптор, указывающий на эту точку. 
                                                                       // Возвращаемый при успешном выполнении файловый дескриптор будет иметь самый маленький номер, который не используется 
                                                                       // процессом.   
   unlink(UNIXSTR_PATH);                                               // Мы вызываем функцию unlink(), чтобы удалить файл в том случае, если он уже существует. Если его не существует, функция
                                                                       // unlink() возвращает ошибку, которую мы игнорируем. Функция unlink() использует системный unlink, который имеет номер
                                                                       // 87 для 64-битной ОС Linux и номер 10 для 32-битной ОС LINUX, а в других ОС какой-то другой номер. unlink() удаляет имя из
                                                                       // файловой системы. Если это имя было последней ссылкой на файл и больше нет процессов, которые держат этот файл открытым,
                                                                       // данный файл удаляется и место, которое он занимает, освобождается для дальнейшего использования.
                                                                       // Если имя было последней ссылкой на файл, но какие-либо процессы всё ещё держат этот файл открытым, файл будет оставлен, пока
                                                                       // последний файловый дескриптор, указывающий на него, не будет закрыт.
                                                                       // Если имя указывает на символьную ссылку, то будет удалена ссылка.
                                                                       // Если имя указывает на сокет, FIFO или устройство, то имя будет удалено, но процессы, которые открыли любой из этих объектов
                                                                       // могут продолжать его использовать.
   bzero(&servaddr, sizeof(servaddr));                                 // Инициализируем всю структуру адреса сервера нулями. Функция bzero() имеет свою реализацию в некоторых ОС. У нас она
                                                                       // имеет вид функционального макроса, который вызывает библиотечную функцию memset() и располагается в файле unp.h
                                                                       // на случай, если в какой-то ОС не будет своей реализации функции bzero().
   servaddr.sun_family = AF_LOCAL;                                     // Присваиваем значение полю структуры адреса сервера, отвечающему за семейство адресов.
   strcpy(servaddr.sun_path, UNIXSTR_PATH);
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
   Signal(SIGCHLD, sig_chld);                                                                
   printf("Наш сервер работает!\n");
   printf("Наш сервер имеет PID = %d\n", getpid());
   printf("Наш сервер имеет прослушиваемый сокет listenfd = %d\n", listenfd);
                                                                
   for(;;){
      printf("Цикл нашего сервера работает! (До вызова accept())\n");
      clilen = sizeof(cliaddr); 
      if( (connfd = Accept(listenfd, (SA *) &cliaddr, &clilen)) < 0){  // Прием клиентского соединения. Функция accept() использует системный вызов accept, который имеет номер
          if(errno == EINTR)                                           // 43 для 64-битной ОС Linux, а в 32-битной ОС LINUX его нет, а в других ОС какой-то другой номер.
              continue;  /* назад в for() */                           // Системный вызов accept используется с типами сокетов, основанными на соединении (SOCK_STREAM, SOCK_SEQPACKET).
          else                                                         // Он извлекает первый запрос на соединение из очереди ожидающих соединений для прослушивающего сокета, listenfd, создаёт новый
              err_sys("accept error");                                 // подключенный сокет и возвращает новый файловый дескриптор, указывающий на сокет. Новый сокет более не находится в слушающем
      }                                                                // состоянии. Исходный сокет listenfd не изменяется при этом системном вызове.
                                                                       // Аргумент listenfd - сокет, который был создан с помощью socket(), привязанный к локальному адресу с помощью bind() и
                                                                       // прослушивающий соединения после listen().
                                                                       // Обычно процесс сервера блокируется при вызове функции accept(), ожидая принятия подключения клиента. Для установки 
                                                                       // TCP-соединения используется трех-этапное рукопожатие (three-way- handshake). Когда рукопожатие состоялось, функция accept()
                                                                       // возвращает значение, и это значение является новым дескриптором (connfd), который называется присоединенным дескриптором
                                                                       // (connected descriptior). Этот новый дескриптор используется для связи с новым клиентом. Новый дескриптор возвращается
                                                                       // функцией accept() для каждого клиента, соединяющегося с нашим сервером.
     printf("Наш сервер имеет присоединенный сокет connfd = %d\n", connfd);                                                                  
     if( (childpid = fork()) == 0){  /* дочерний процесс */
       Close(listenfd);              /* закрываем прослушиваемый сокет */
       str_echo(connfd);             /* обрабатываем запрос */
       exit(0);
     }
     Close(connfd);                  /* родительский процесс закрывает присоединенный сокет */
  }
}
 
 
