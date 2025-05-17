// echo_UDP_server_IPv4_return_flags.c
#include	"unp.h"

#include	"unpifi.h"
#include	<sys/param.h>		/* ALIGN macro for CMSG_NXTHDR() macro */
#include	<stdarg.h>		/* ANSI C header file */
#include	<syslog.h>		/* for syslog() */

#undef	MAXLINE
#define	MAXLINE	20		/* to see datagram truncation */

int		daemon_proc;		/* set nonzero by daemon_init() */

static void
err_doit(int errnoflag, int level, const char *fmt, va_list ap)
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

void
err_quit(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

void
err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

char *
sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char		portstr[8];
    static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
/* end sock_ntop */

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		str[0] = '[';
		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
			return(str);
		}
		return (str + 1);
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
			snprintf(str, sizeof(str), "%*s (index %d)",
					 sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
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

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
	char	*ptr;

	if ( (ptr = sock_ntop(sa, salen)) == NULL)
		err_sys("sock_ntop error");	/* inet_ntop() sets errno */
	return(ptr);
}

ssize_t
recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp,
			   SA *sa, socklen_t *salenptr, struct in_pktinfo *pktp)
{
	struct msghdr	msg;
	struct iovec	iov[1];
	ssize_t			n;

#ifdef	HAVE_MSGHDR_MSG_CONTROL
	struct cmsghdr	*cmptr;
	union {
	  struct cmsghdr	cm;
	  char				control[CMSG_SPACE(sizeof(struct in_addr)) +
								CMSG_SPACE(sizeof(struct in_pktinfo))];
	} control_un;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
	msg.msg_flags = 0;
#else
	bzero(&msg, sizeof(msg));	/* make certain msg_accrightslen = 0 */
#endif

	msg.msg_name = sa;
	msg.msg_namelen = *salenptr;
	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if ( (n = recvmsg(fd, &msg, *flagsp)) < 0)
		return(n);

	*salenptr = msg.msg_namelen;	/* pass back results */
	if (pktp)
		bzero(pktp, sizeof(struct in_pktinfo));	/* 0.0.0.0, i/f = 0 */
/* end recvfrom_flags1 */

/* include recvfrom_flags2 */
#ifndef	HAVE_MSGHDR_MSG_CONTROL
	*flagsp = 0;					/* pass back results */
	return(n);
#else

	*flagsp = msg.msg_flags;		/* pass back results */
	if (msg.msg_controllen < sizeof(struct cmsghdr) ||
		(msg.msg_flags & MSG_CTRUNC) || pktp == NULL)
		return(n);

	for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL;
		 cmptr = CMSG_NXTHDR(&msg, cmptr)) {

#ifdef	IP_RECVDSTADDR
		if (cmptr->cmsg_level == IPPROTO_IP &&
			cmptr->cmsg_type == IP_RECVDSTADDR) {

			memcpy(&pktp->ipi_addr, CMSG_DATA(cmptr),
				   sizeof(struct in_addr));
			continue;
		}
#endif

#ifdef	IP_RECVIF
		if (cmptr->cmsg_level == IPPROTO_IP &&
			cmptr->cmsg_type == IP_RECVIF) {
			struct sockaddr_dl	*sdl;

			sdl = (struct sockaddr_dl *) CMSG_DATA(cmptr);
			pktp->ipi_ifindex = sdl->sdl_index;
			continue;
		}
#endif
		err_quit("unknown ancillary data, len = %d, level = %d, type = %d",
				 cmptr->cmsg_len, cmptr->cmsg_level, cmptr->cmsg_type);
	}
	return(n);
#endif	/* HAVE_MSGHDR_MSG_CONTROL */
}
/* end recvfrom_flags2 */

ssize_t
Recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp,
			   SA *sa, socklen_t *salenptr, struct in_pktinfo *pktp)
{
	ssize_t		n;

	n = recvfrom_flags(fd, ptr, nbytes, flagsp, sa, salenptr, pktp);
	if (n < 0)
		err_quit("recvfrom_flags error");

	return(n);
}

void
dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int						flags;
	const int				on = 1;
	socklen_t				len;
	ssize_t					n;
	char					mesg[MAXLINE], str[INET6_ADDRSTRLEN],
							ifname[IFNAMSIZ];
	struct in_addr			in_zero;
	struct in_pktinfo	pktinfo;

#ifdef	IP_RECVDSTADDR
	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on)) < 0)
		err_ret("setsockopt of IP_RECVDSTADDR");
#endif
#ifdef	IP_RECVIF
	if (setsockopt(sockfd, IPPROTO_IP, IP_RECVIF, &on, sizeof(on)) < 0)
		err_ret("setsockopt of IP_RECVIF");
#endif
	bzero(&in_zero, sizeof(struct in_addr));	/* all 0 IPv4 address */

	for ( ; ; ) {
		len = clilen;
		flags = 0;
		n = Recvfrom_flags(sockfd, mesg, MAXLINE, &flags,
						   pcliaddr, &len, &pktinfo);
		printf("%d-byte datagram from %s", n, Sock_ntop(pcliaddr, len));
		if (memcmp(&pktinfo.ipi_addr, &in_zero, sizeof(in_zero)) != 0)
			printf(", to %s", inet_ntop(AF_INET, &pktinfo.ipi_addr,
										str, sizeof(str)));
		if (pktinfo.ipi_ifindex > 0)
			printf(", recv i/f = %s",
				   if_indextoname(pktinfo.ipi_ifindex, ifname));
#ifdef	MSG_TRUNC
		if (flags & MSG_TRUNC)	printf(" (datagram truncated)");
#endif
#ifdef	MSG_CTRUNC
		if (flags & MSG_CTRUNC)	printf(" (control info truncated)");
#endif
#ifdef	MSG_BCAST
		if (flags & MSG_BCAST)	printf(" (broadcast)");
#endif
#ifdef	MSG_MCAST
		if (flags & MSG_MCAST)	printf(" (multicast)");
#endif
		printf("\n");

		sendto(sockfd, mesg, n, 0, pcliaddr, len);
	}
}

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

	dg_echo(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
}


