// daytimetcpclient_MOD_PARAMS.c
#include "unp.h"
#include <stdarg.h>
#include <syslog.h>		                                                /* for syslog() */

#include	<netinet/tcp.h>		/* определения констант TCP_xxx */

int daemon_proc;		                                                /* set nonzero by daemon_init() */

static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
	int errno_save, n;
	char buf[MAXLINE];

        //printf("errno = %d\n", errno);
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

void err_ret(const char *fmt, ...)
{
       va_list ap;
       va_start(ap, fmt);
       err_doit(1, errno, fmt, ap);
       va_end(ap);
}

char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char		portstr[7];
    static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
/* end sock_ntop */

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
		}
		return(str);
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

			/* OK to have no pathname bound to the socket: happens on
			   every connect() unless client calls bind() first. */
		if (unp->sun_path[0] == 0)
			strcpy(str, "(no pathname bound)");
		else
			snprintf(str, sizeof(str), "%s", unp->sun_path);
		return(str);
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

		if (sdl->sdl_nlen > 0)
			snprintf(str, sizeof(str), "%*s",
					 sdl->sdl_nlen, &sdl->sdl_data[0]);
		else
			snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
		return(str);
	}
#endif
	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}


union val {
  int				i_val;
  long				l_val;
  char				c_val[10];
  struct linger		linger_val;
  struct timeval	timeval_val;
} val;

static char	*sock_str_flag(union val *, int);
static char	*sock_str_int(union val *, int);
static char	*sock_str_linger(union val *, int);
static char	*sock_str_timeval(union val *, int);

struct sock_opts {
  char	   *opt_str;
  int		opt_level;
  int		opt_name;
  char   *(*opt_val_str)(union val *, int);
} sock_opts[] = {
	"SO_RCVBUF",		SOL_SOCKET,	SO_RCVBUF,		sock_str_int,
	"SO_SNDBUF",		SOL_SOCKET,	SO_SNDBUF,		sock_str_int,
	"TCP_MAXSEG",		IPPROTO_TCP,TCP_MAXSEG,		sock_str_int,
	"TCP_NODELAY",		IPPROTO_TCP,TCP_NODELAY,	sock_str_flag,
	NULL,				0,			0,				NULL
};



int main(int argc, char **argv)
{
   int sockfd, n;
   char recvline[MAXLINE + 1];
   int counter_reads = 0;
   struct sockaddr_in servaddr, cliaddr;                                        // Структура адреса сервера.
   
   socklen_t len;
   struct sock_opts *ptr;
   
   if(argc != 3)
       err_quit("usage: a.out <IPaddress> and number port");
   
   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)                  // Создание сокета, функция socket() создает объект сокета в ядре ОС с помощью системного вызова socket, который имеет номер
       err_sys("socket error");                                        // 41 для 64-битной ОС Linux и номер 359 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Системный вызов socket() создаёт конечную точку соединения и возвращает файловый дескриптор, указывающий на эту точку. 
                                                                       // Возвращаемый при успешном выполнении файловый дескриптор будет иметь самый маленький номер, который не используется 
                                                                       // процессом.                                                                             
   bzero(&servaddr, sizeof(servaddr));                                 // Инициализируем всю структуру адреса сервера нулями. Функция bzero() имеет свою реализацию в некоторых ОС. У нас она
                                                                       // имеет вид функционального макроса, который вызывает библиотечную функцию memset() и располагается в файле unp.h
                                                                       // на случай, если в какой-то ОС не будет своей реализации функции bzero().
   bzero(&cliaddr, sizeof(cliaddr));                                                                 
   servaddr.sin_family = AF_INET;                                      // Присваиваем значение полю структуры адреса сервера, отвечающему за семейство адресов.     
   servaddr.sin_port = htons(atoi(argv[2]));  /* сервер времени и даты */         // Присваиваем значение полю структуры адреса сервера, отвечающему за номер порта. Библиотечная функция htons() преобразует
                                                                       // двузначный номер порта, принятый в качестве аргумента, в требуемый формат, путем вращения влево битов значения с помощью
                                                                       // инструкции ЦП rol.
   if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)            // Присваиваем значение полю структуры адреса сервера, отвечающему за IP-адрес. Библиотечная функция inet_pton() преобразует
      err_quit("inet_pton error for %s", argv[1]);                     // аргумент командной строки в символах ASCII(например 206.168.112.96) в двоичный формат.
      
   printf("Наш клиент имеет PID = %d\n", getpid());
   printf("Наш клиент обращается к серверу с номером порта = %d\n", htons(servaddr.sin_port));
   
   // Получение параметров сокета до вызова функции connect()
   printf("Получение параметров сокета до вызова функции connect():\n");
   for (ptr = sock_opts; ptr->opt_str != NULL; ptr++) {
		printf("%s: ", ptr->opt_str);
		if (ptr->opt_val_str == NULL)
			printf("(undefined)\n");
		else {
		        
                       len = sizeof(val);
			if (getsockopt(sockfd, ptr->opt_level, ptr->opt_name,
						   &val, &len) == -1) {
				err_ret("getsockopt error");
			} else {
				printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
			}
	       }
   }
   
   if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)          // Функция connect() устанавливает соединение нашего сокета sockfd с сервером, адрес сокета которого содержится в структуре
      err_sys("connect error");                                        // адреса сервера servaddr. Функция connect() использует системный вызов connect, который имеет номер 42 для 64-битной 
                                                                       // ОС Linux и номер 362 для 32-битной ОС LINUX, а в других ОС какой-то другой номер. 
                                                                       // Системный вызов connect() устанавливает соединение с сокетом, заданным файловый дескриптором sockfd, ссылающимся
                                                                       // на адрес servaddr. Аргумент sizeof(servaddr) определяет размер servaddr. Формат адреса в servaddr определяется адресным 
                                                                       // пространством сокета sockfd. 
   
   // Получение параметров сокета после вызова функции connect()
   printf("Получение параметров сокета после вызова функции connect():\n");
   for (ptr = sock_opts; ptr->opt_str != NULL; ptr++) {
		printf("%s: ", ptr->opt_str);
		if (ptr->opt_val_str == NULL)
			printf("(undefined)\n");
		else {
		        
                       len = sizeof(val);
			if (getsockopt(sockfd, ptr->opt_level, ptr->opt_name,
						   &val, &len) == -1) {
				err_ret("getsockopt error");
			} else {
				printf("%s\n", (*ptr->opt_val_str)(&val, len));
			}
	       }
   }
                                                                     
   cliaddr.sin_family = AF_INET;                                                                  
   socklen_t len_cli = sizeof(cliaddr);  // = MAXSOCKADDR;                                                                   
   if(getsockname(sockfd, (SA *)&cliaddr, &len_cli) < 0)
      printf("Не удалось получить локальный адрес протокола.\n");
   printf("Наш клиент имеет локальный IP-адрес (по getsockname()) = %s\n", sock_ntop((SA *)&cliaddr, len)); 
   printf("Наш клиент имеет номер порта (по getsockname()) = %d\n", ntohs(cliaddr.sin_port));                                                                      
   
   while((n = read(sockfd, recvline, MAXLINE)) > 0)                    // Ответ сервера приходит в наш сокет sockfd, из него мы читаем в наш массив recvline с помощью функции read(), вызывая её 
   {                                                                   // циклически, т.к. по протоколу TCP данные могут придти не в виде одного сегмента, а в виде нескольких сегментов, поэтому 
                                                                       // нужно несколько раз вызывать функцию read() в цикле.
      counter_reads++;                                                 // counter_reads показывает на сколько сегментов был разделен изначальный объем информации, передаваемой по протоколу TCP.
      recvline[n] = 0; /* завершающий нуль */                          // Обычно возвращается один сегмент, но при больших объемах данных нельзя расчитывать, что ответ сервера будет получен с 
      if(fputs(recvline, stdout) == EOF)                               // помощью одного вызова функции read().
          err_sys("fputs error");
   }
   if(n < 0)
      err_sys("read error");
   printf("\ncounter_reads = %d\n", counter_reads); 
   exit(0);
}

static char strres[128];

static char *sock_str_flag(union val *ptr, int len)
{
/* *INDENT-OFF* */
	if (len != sizeof(int))
		snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
	else
		snprintf(strres, sizeof(strres),
				 "%s", (ptr->i_val == 0) ? "off" : "on");
	return(strres);
/* *INDENT-ON* */
}

static char *sock_str_int(union val *ptr, int len)
{
	if (len != sizeof(int))
		snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
	else
		snprintf(strres, sizeof(strres), "%d", ptr->i_val);
	return(strres);
}

static char *sock_str_linger(union val *ptr, int len)
{
	struct linger	*lptr = &ptr->linger_val;

	if (len != sizeof(struct linger))
		snprintf(strres, sizeof(strres),
				 "size (%d) not sizeof(struct linger)", len);
	else
		snprintf(strres, sizeof(strres), "l_onoff = %d, l_linger = %d",
				 lptr->l_onoff, lptr->l_linger);
	return(strres);
}

static char *sock_str_timeval(union val *ptr, int len)
{
	struct timeval	*tvptr = &ptr->timeval_val;

	if (len != sizeof(struct timeval))
		snprintf(strres, sizeof(strres),
				 "size (%d) not sizeof(struct timeval)", len);
	else
		snprintf(strres, sizeof(strres), "%d sec, %d usec",
				 tvptr->tv_sec, tvptr->tv_usec);
	return(strres);
}
