// checkopts.c
#include	"unp.h"
#include	<netinet/tcp.h>		/* определения констант TCP_xxx */

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

void err_ret(const char *fmt, ...)
{
 va_list ap;
 va_start(ap, fmt);
 err_doit(1, errno, fmt, ap);
 va_end(ap);
}

int Socket(int family, int type, int protocol)                                 // Наша собственная функция-обертка для функции socket.
{
  int n;
  if( (n = socket(family, type, protocol)) < 0)
       err_sys("socket error");
  return (n);
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
	"SO_BROADCAST",		SOL_SOCKET,	SO_BROADCAST,	sock_str_flag,
	"SO_DEBUG",			SOL_SOCKET,	SO_DEBUG,		sock_str_flag,
	"SO_DONTROUTE",		SOL_SOCKET,	SO_DONTROUTE,	sock_str_flag,
	"SO_ERROR",			SOL_SOCKET,	SO_ERROR,		sock_str_int,
	"SO_KEEPALIVE",		SOL_SOCKET,	SO_KEEPALIVE,	sock_str_flag,
	"SO_LINGER",		SOL_SOCKET,	SO_LINGER,		sock_str_linger,
	"SO_OOBINLINE",		SOL_SOCKET,	SO_OOBINLINE,	sock_str_flag,
	"SO_RCVBUF",		SOL_SOCKET,	SO_RCVBUF,		sock_str_int,
	"SO_SNDBUF",		SOL_SOCKET,	SO_SNDBUF,		sock_str_int,
	"SO_RCVLOWAT",		SOL_SOCKET,	SO_RCVLOWAT,	sock_str_int,
	"SO_SNDLOWAT",		SOL_SOCKET,	SO_SNDLOWAT,	sock_str_int,
	"SO_RCVTIMEO",		SOL_SOCKET,	SO_RCVTIMEO,	sock_str_timeval,
	"SO_SNDTIMEO",		SOL_SOCKET,	SO_SNDTIMEO,	sock_str_timeval,
	"SO_REUSEADDR",		SOL_SOCKET,	SO_REUSEADDR,	sock_str_flag,
#ifdef	SO_REUSEPORT
	"SO_REUSEPORT",		SOL_SOCKET,	SO_REUSEPORT,	sock_str_flag,
#else
	"SO_REUSEPORT",		0,			0,				NULL,
#endif
	"SO_TYPE",			SOL_SOCKET,	SO_TYPE,		sock_str_int,
#ifdef	SO_USELOOPBACK
	"SO_USELOOPBACK",	SOL_SOCKET,	SO_USELOOPBACK,	sock_str_flag,
#else
     	"SO_USELOOPBACK",		0,			0,				NULL,
#endif
	"IP_TOS",			IPPROTO_IP,	IP_TOS,			sock_str_int,
	"IP_TTL",			IPPROTO_IP,	IP_TTL,			sock_str_int,
	"TCP_MAXSEG",		IPPROTO_TCP,TCP_MAXSEG,		sock_str_int,
	"TCP_NODELAY",		IPPROTO_TCP,TCP_NODELAY,	sock_str_flag,
	NULL,				0,			0,				NULL
};


int main(int argc, char **argv)
{
	int fd;
	socklen_t len;
	struct sock_opts *ptr;

	

	for (ptr = sock_opts; ptr->opt_str != NULL; ptr++) {
		printf("%s: ", ptr->opt_str);
		if (ptr->opt_val_str == NULL)
			printf("(undefined)\n");
		else {
		        switch(ptr->opt_level){
		           case SOL_SOCKET:
		           case IPPROTO_IP:
		           case IPPROTO_TCP: fd = Socket(AF_INET, SOCK_STREAM, 0);
		                             break;
#ifdef IPV6
                          case IPPROTO_IPV6: fd = Socket(AF_INET6, SOCK_STREAM, 0); 
                                             break;
#endif
#ifdef IPPROTO_SCTP
                          case IPPROTO_SCTP: fd = Socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
                                             break;
#endif
                          default: err_quit("Can't create fd for level %d\n", ptr->opt_level); 
                       }   
                                 		                            
			len = sizeof(val);
			if (getsockopt(fd, ptr->opt_level, ptr->opt_name,
						   &val, &len) == -1) {
				err_ret("getsockopt error");
			} else {
				printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
			}
			close(fd);
		}
	}
	exit(0);
}

static char	strres[128];

static char	*
sock_str_flag(union val *ptr, int len)
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

static char	*
sock_str_int(union val *ptr, int len)
{
	if (len != sizeof(int))
		snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
	else
		snprintf(strres, sizeof(strres), "%d", ptr->i_val);
	return(strres);
}

static char	*
sock_str_linger(union val *ptr, int len)
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

static char	*
sock_str_timeval(union val *ptr, int len)
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
