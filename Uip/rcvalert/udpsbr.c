/* udpsbr.c: support routines for the UDP protocol notification stuff */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/udpsbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/udpsbr.c,v 6.0 1991/12/18 20:39:41 jpo Rel $
 *
 * $Log: udpsbr.c,v $
 * Revision 6.0  1991/12/18  20:39:41  jpo
 * Release 6.0
 *
 */



#include <stdio.h>
#include <isode/internet.h>
#include <sys/types.h>

#include "data.h"

int	getdata (fd, buf, len, user, from)
int	fd;
char	*buf;
int	*len;
char	*user;
char	*from;
{
	struct data thedata;
	struct data *ds = &thedata;
	struct sockaddr_in sin;
	int	slen = sizeof sin;
	int	retval = 0;
	int	n;

	if((n = recvfrom (fd, (char *)ds, sizeof *ds, 0,
			  (struct sockaddr *)&sin, &slen)) <= 0 ||
	   n != sizeof *ds)
		return retval;
	if ((*len = strlen(ds->data)) > (sizeof ds -> data) )
		*len = sizeof (ds -> data);
	bcopy (ds -> data, buf, *len);
	buf[*len] = 0;
	if ((n = strlen (ds->from)) > (sizeof ds -> from))
		n = sizeof (ds -> from);
	bcopy (ds -> from, from, n);
	from[n] = 0;
	if (checkid (ds, &sin, user))
		retval = 1;
	ack (&sin);
	return retval;
}


struct ref {
	long	refid;
	u_long	addr;
};
#define MAXREFS	10

checkid (ds, sp, user)
struct data *ds;
struct sockaddr_in *sp;
char	*user;
{
	static struct ref refs[MAXREFS];
	static int	cur;
	struct ref *rp;
	int	refid = (int) ntohl(ds->refid);

	if (strcmp (user, ds -> user) != 0)
		return 0;

	for (rp = refs; rp < &refs[MAXREFS]; rp ++)
		if (rp -> refid == refid &&
		    sp -> sin_addr.s_addr == rp -> addr)
			return 0;

	rp = &refs[cur];
	rp -> refid = refid;
	rp -> addr = sp -> sin_addr.s_addr;
	cur = (cur + 1) % MAXREFS;
	return 1;
}

static short theport;
static int replysock;
char	alertfilename[BUFSIZ];

ack (sp)
struct sockaddr_in *sp;
{
	if( sendto (replysock, "ok", 2, 0,
		    (struct sockaddr *)sp, sizeof *sp) < 0)
		perror ("sendto");
}

udp_start (port, file, home)
short port;
char	*file, *home;
{
	int	s;
	struct sockaddr_in sin;
	int	len;

	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror ("socket failed");
		exit (1);
	}
	bzero ((char *)&sin, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	if (bind (s, (struct sockaddr *)&sin, sizeof sin) < 0) {
		perror ("bind failed");
		exit (1);
	}

	if ((replysock = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror ("socket failed");
		exit (1);
	}
	len = sizeof sin;
	if (getsockname (s, (struct sockaddr *)&sin, &len) == -1) {
		perror ("getsockname");
		exit (1);
	}
        theport = ntohs (sin.sin_port);
	makeredirfile (file, home);
	return s;
}


makeredirfile (file, home)
char	*file;
char	*home;
{
	FILE	*fp;
	char	hostname[128];

	if (gethostname (hostname, sizeof hostname) == -1) {
		perror ("Can't fetch hostname");
		return;
	}
	if (*file == '/' || strncmp (file, "./", 2) == 0 ||
	    strncmp (file, "../", 3) == 0)
		(void) strcpy (alertfilename, file);
	else {
		(void) sprintf (alertfilename, "%s/%s",
				home, file);
	}


	if ((fp = fopen (alertfilename, "w")) == NULL) {
		perror (alertfilename);
		return;
	}
	fprintf (fp, "%s %d\n", hostname, theport);
	(void) fclose (fp);
}

udp_cleanup ()
{
	if (alertfilename[0])
		(void) unlink (alertfilename);
	if (replysock)
		(void) close (replysock);
}
