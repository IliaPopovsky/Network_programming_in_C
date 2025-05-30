// daytimetcpclient.c
#include "unp.h"
#include <stdarg.h>
#include <syslog.h>		                                                /* for syslog() */

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



int main(int argc, char **argv)
{
   int sockfd, n;
   char recvline[MAXLINE + 1];
   int counter_reads = 0;
   struct sockaddr_in servaddr;                                        // Структура адреса сервера.
   
   if(argc != 2)
       err_quit("usage: a.out <IPaddress>");
   
   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)                  // Создание сокета, функция socket() создает объект сокета в ядре ОС с помощью системного вызова socket, который имеет номер
       err_sys("socket error");                                        // 41 для 64-битной ОС Linux и номер 359 для 32-битной ОС LINUX, а в других ОС какой-то другой номер.
                                                                       // Системный вызов socket() создаёт конечную точку соединения и возвращает файловый дескриптор, указывающий на эту точку. 
                                                                       // Возвращаемый при успешном выполнении файловый дескриптор будет иметь самый маленький номер, который не используется 
                                                                       // процессом.                                                                             
   bzero(&servaddr, sizeof(servaddr));                                 // Инициализируем всю структуру адреса сервера нулями. Функция bzero() имеет свою реализацию в некоторых ОС. У нас она
                                                                       // имеет вид функционального макроса, который вызывает библиотечную функцию memset() и располагается в файле unp.h
                                                                       // на случай, если в какой-то ОС не будет своей реализации функции bzero().
   servaddr.sin_family = AF_INET;                                      // Присваиваем значение полю структуры адреса сервера, отвечающему за семейство адресов.     
   servaddr.sin_port = htons(48881/*9999*//*13*/);  /* сервер времени и даты */         // Присваиваем значение полю структуры адреса сервера, отвечающему за номер порта. Библиотечная функция htons() преобразует
                                                                       // двузначный номер порта, принятый в качестве аргумента, в требуемый формат, путем вращения влево битов значения с помощью
                                                                       // инструкции ЦП rol.
   if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)            // Присваиваем значение полю структуры адреса сервера, отвечающему за IP-адрес. Библиотечная функция inet_pton() преобразует
      err_quit("inet_pton error for %s", argv[1]);                     // аргумент командной строки в символах ASCII(например 206.168.112.96) в двоичный формат.
      
   printf("Наш клиент имеет PID = %d\n", getpid());
   printf("Наш клиент обращается к серверу с номером порта = %d\n", htons(servaddr.sin_port));
   if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)          // Функция connect() устанавливает соединение нашего сокета sockfd с сервером, адрес сокета которого содержится в структуре
      err_sys("connect error");                                        // адреса сервера servaddr. Функция connect() использует системный вызов connect, который имеет номер 42 для 64-битной 
                                                                       // ОС Linux и номер 362 для 32-битной ОС LINUX, а в других ОС какой-то другой номер. 
                                                                       // Системный вызов connect() устанавливает соединение с сокетом, заданным файловый дескриптором sockfd, ссылающимся
                                                                       // на адрес servaddr. Аргумент sizeof(servaddr) определяет размер servaddr. Формат адреса в servaddr определяется адресным 
                                                                       // пространством сокета sockfd.                                                                       
   
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
