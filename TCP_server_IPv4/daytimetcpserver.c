// daytimetcpserver.c
#include "unp.h"
#include <time.h>

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

int main(int argc, char **argv)
{
   int listenfd, connfd;
   struct sockaddr_in servaddr;
   char buff[MAXLINE];
   time_t ticks;
   
   listenfd = Socket(AF_INET, SOCK_STREAM, 0);                         // Создание сокета. Функция socket() создает объект сокета в ядре ОС с помощью системного вызова socket, который имеет номер
                                                                       // 41 для 64-битной ОС Linux и номер 359 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Системный вызов socket() создаёт конечную точку соединения и возвращает файловый дескриптор, указывающий на эту точку. 
                                                                       // Возвращаемый при успешном выполнении файловый дескриптор будет иметь самый маленький номер, который не используется 
                                                                       // процессом.                                     
   bzero(&servaddr, sizeof(servaddr));                                 // Инициализируем всю структуру адреса сервера нулями. Функция bzero() имеет свою реализацию в некоторых ОС. У нас она
                                                                       // имеет вид функционального макроса, который вызывает библиотечную функцию memset() и располагается в файле unp.h
                                                                       // на случай, если в какой-то ОС не будет своей реализации функции bzero().
   servaddr.sin_family = AF_INET;                                      // Присваиваем значение полю структуры адреса сервера, отвечающему за семейство адресов.
   servaddr.sin_port = htons(13);    /* сервер времени и даты */       // Присваиваем значение полю структуры адреса сервера, отвечающему за номер порта. Библиотечная функция htons() преобразует
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
   printf("Наш сервер работает!\n");
   for(;;){
      printf("Цикл нашего сервера работает! (До вызова accept())\n"); 
      connfd = Accept(listenfd, (SA *) NULL, NULL);                    // Прием клиентского соединения. Функция accept() использует системный вызов accept, который имеет номер
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
      ticks = time(NULL);                                              
      snprintf(buff, sizeof(buff), "%.24s\er\en", ctime(&ticks));
      Write(connfd, buff, strlen(buff));                               // Результат передается клиенту функцией write(). Функция write() использует системный вызов write, который имеет номер
                                                                       // 1 для 64-битной ОС Linuxи и номер 4 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
      
      Close(connfd);                                                   // Закрытие соединения с клиентом. Функция close() использует системный вызов close, который имеет номер
                                                                       // 3 для 64-битной ОС Linuxи и номер 6 для 32-битной ОС LINUX, а в других ОС какой-то другой номер. Функция close() закрывает
                                                                       // файловый дескриптор, так что он больше не ссылается ни на какой файл и может быть использован повторно.   
                                                                       // Сервер закрывает соединение с клиентом, вызывая функцию close(). Это инициирует обычную последовательность прерырвания
                                                                       // соединения TCP: пакет FIN посылается в обоих направлениях, и каждый пакет FIN распознается на другом конце соединения. 
      printf("Цикл нашего сервера работает!\n");                                                                 
   }
}
   
   
