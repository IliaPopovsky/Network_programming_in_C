// echo_UDP_server_IPv4.c
#include	"unp.h"

#include	<stdarg.h>		/* ANSI C header file */
#include	<syslog.h>		/* for syslog() */

int		daemon_proc;		/* set nonzero by daemon_init() */

static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
	int		errno_save, n;
	char	buf[MAXLINE + 1];

	errno_save = errno;		/* value caller might want printed */
#ifdef	HAVE_VSNPRINTF
	vsnprintf(buf, MAXLINE, fmt, ap);	/* safe */
#else
	vsprintf(buf, fmt, ap);					/* not safe */
#endif
	n = strlen(buf);
	if (errnoflag)
		snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));
	strcat(buf, "\n");

	if (daemon_proc) {
		syslog(level, buf);
	} else {
		fflush(stdout);		/* in case stdout and stderr are the same */
		fputs(buf, stderr);
		fflush(stderr);
	}
	return;
}

void err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_quit(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}


static int		sockfd;

#define	QSIZE	   8		/* size of input queue */  /* размер входной очереди */
#define	MAXDG	4096		/* max datagram size */    /* максимальный размер дейтаграммы */

typedef struct {
  void		*dg_data;		/* ptr to actual datagram */        /* указатель на текущую дейтаграмму */
  size_t	dg_len;			/* length of datagram */            /* длина дейтаграммы */
  struct sockaddr  *dg_sa;	/* ptr to sockaddr{} w/client's address */  /* указатель на sockaddr{} с адресом клиента */
  socklen_t	dg_salen;		/* length of sockaddr{} */          /* длина sockaddr{} */
} DG;
static DG	dg[QSIZE];			/* queue of datagrams to process */   /* очередь дейтаграмм для обработки */
static long	cntread[QSIZE+1];	/* diagnostic counter */                      /* диагностический счетчик */

static int	iget;		/* next one for main loop to process */               /* следующий элемент для обработки в основном цикле */
static int	iput;		/* next one for signal handler to read into */        /* следующий элемент для считывания обработчиком сигналов */
static int	nqueue;		/* # on queue for main loop to process */             /* количество дейтаграмм в очереди на обьработку в основном цикле */
static socklen_t clilen;/* max length of sockaddr{} */                                /* максимальная длина sockaddr{} */   

static void	sig_io(int);
static void	sig_hup(int);

void dg_echo(int sockfd_arg, SA *pcliaddr, socklen_t clilen_arg)
{
	int i;
	const int on = 1;
	sigset_t zeromask, newmask, oldmask;

	sockfd = sockfd_arg;
	clilen = clilen_arg;

	for (i = 0; i < QSIZE; i++) {	/* init queue of buffers */   /* инициализация очереди */
		dg[i].dg_data = malloc(MAXDG);
		dg[i].dg_sa = malloc(clilen);
		dg[i].dg_salen = clilen;
	}
	iget = iput = nqueue = 0;

	signal(SIGHUP, sig_hup);
	signal(SIGIO, sig_io);
	fcntl(sockfd, F_SETOWN, getpid());
	ioctl(sockfd, FIOASYNC, &on);
	ioctl(sockfd, FIONBIO, &on);

	sigemptyset(&zeromask);		/* init three signal sets */    /* инициализация трех наборов сигналов */
	sigemptyset(&oldmask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGIO);	/* signal we want to block */   /* сигнал, который хотим блокировать */

	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	for ( ; ; ) {
		while (nqueue == 0)
			sigsuspend(&zeromask);	/* wait for datagram to process */  /* ждем дейтаграмму для обработки */

			/* 4unblock SIGIO */       /* разблокирование SIGIO */
		sigprocmask(SIG_SETMASK, &oldmask, NULL);

		sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0,
			   dg[iget].dg_sa, dg[iget].dg_salen);

		if (++iget >= QSIZE)
			iget = 0;

			/* 4block SIGIO */         /* блокировка SIGIO */
		sigprocmask(SIG_BLOCK, &newmask, &oldmask);
		nqueue--;
	}
}


static void sig_io(int signo)
{
	ssize_t	len;
	int nread;
	DG *ptr;

	for (nread = 0; ; ) {
		if (nqueue >= QSIZE)
			err_quit("receive overflow");

		ptr = &dg[iput];
		ptr->dg_salen = clilen;
		len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0,
					   ptr->dg_sa, &ptr->dg_salen);
		if (len < 0) {
			if (errno == EWOULDBLOCK)
				break;		/* all done; no more queued to read */  /* все сделано; очередь на чтение отсутствует */
			else
				err_sys("recvfrom error");
		}
		ptr->dg_len = len;

		nread++;
		nqueue++;
		if (++iput >= QSIZE)
			iput = 0;

	}
	cntread[nread]++;		/* histogram of # datagrams read per signal */  /* гистаграмма количества дейтаграмм, считанных для каждого сигнала */
}


static void sig_hup(int signo)
{
	int i;

	for (i = 0; i <= QSIZE; i++)
		printf("cntread[%d] = %ld\n", i, cntread[i]);
}


int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
}


