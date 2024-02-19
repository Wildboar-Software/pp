/* loc_proto.c: subroutines to help with local delivery */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Chans/slocal/RCS/loc_proto.c,v 6.0 1991/12/18 20:12:09 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Chans/slocal/RCS/loc_proto.c,v 6.0 1991/12/18 20:12:09 jpo Rel $
 *
 * $Log: loc_proto.c,v $
 * Revision 6.0  1991/12/18  20:12:09  jpo
 * Release 6.0
 *
 */



#include "util.h"
#include "retcode.h"
#include <sys/wait.h>
#include "sys.file.h"
#include "loc_user.h"
#include <isode/usr.dirent.h>
#include <sys/stat.h>
#ifdef masscomp
#include <sys/file.h>
#endif

#define P_READ	0
#define P_WRITE 1
#define NULLDIR ((DIR*)0)
#define NULLDCT ((struct dirent *)0)

static char	*next_message();
static int 	fdin = NOTOK, fdout = NOTOK;
static int 	pid, sysreply;
static FILE	*cur_fp = NULL;
static DIR	*mb_dp	= NULL;
static int 	copy(), loc_set(), send_pac(), do_child_stuff(), do_packet(), val(), isdir();

static struct loc_packet {
	int		command;
#define LP_OPEN			1
#define LP_CLOSE		2
#define LP_CD			3
#define LP_WRITE		4
#define LP_MOVE			5
#define LP_MKDIR		6
#define LP_OPENDIR		7
#define LP_CLOSEDIR		8
#define LP_SYSTEM		9
#define LP_NEXTMESS		10
#define LP_ISDIR		11
	char		s1 [MAXPATHLENGTH];
	char		s2 [MAXPATHLENGTH];
	int		size;
};



/* -------------------  Begin Routines  ------------------------------------- */


loc_init (loc)
LocUser	*loc;
{
	int p1[2], p2[2];

	if (fdin != NOTOK || fdout != NOTOK)
		loc_end();

	if (pipe(p1) == NOTOK)
		return RP_BAD;

	if (pipe (p2) == NOTOK) {
		(void) close (p1[P_READ]);
		(void) close (p1[P_WRITE]);
		return RP_BAD;
	}

	ll_close (pp_log_norm);
	if ((pid = tryfork()) == NOTOK) {
		(void) close (p1[P_READ]);
		(void) close (p1[P_WRITE]);
		(void) close (p2[P_READ]);
		(void) close (p2[P_WRITE]);
		return RP_BAD;
	}

	if (pid) {
		(void) close (p1[P_READ]);
		fdout = p1[P_WRITE];
		(void) close (p2[P_WRITE]);
		fdin = p2[P_READ];
		return RP_OK;
	}

	(void) close (p1[P_WRITE]);
	(void) close (p2[P_READ]);

	fdin = p1[P_READ];
	fdout = p2[P_WRITE];

	if ((loc -> username &&
	     initgroups (loc -> username, loc -> gid) == -1) ||
	    setregid (loc -> gid, loc -> gid) == -1 ||
	    setreuid (loc -> uid, loc -> uid) == -1) {
		PP_SLOG (LLOG_EXCEPTIONS, "user",
		      ("Can't set uid/gid"));
		_exit(1);
	}

	if (chdir (loc -> directory) == NOTOK) {
		PP_SLOG (LLOG_EXCEPTIONS, "chdir",
		      ("Can't change directory to %s", loc -> directory));
		_exit (1);
	}
		
	do_child_stuff();

	/* NOTREACHED */
}


int	loc_cd (dir, rp)
char	*dir;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_CD;
	(void) strcpy (packet.s1, dir);
	return send_pac (&packet, rp);
}


int	loc_mkdir (dir, mode, rp)
char	*dir;
int	mode;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_MKDIR;
	(void) strcpy (packet.s1, dir);
	packet.size = mode;
	return send_pac (&packet, rp);
}


int	loc_write (buf, size, rp)
char	*buf;
int	size;
RP_Buf	*rp;
{
	struct loc_packet packet;
	packet.command = LP_WRITE;
	packet.size = size;
	if (write (fdout, (char *)&packet, sizeof packet) != sizeof packet)
		return loc_set (rp, RP_FIO, "Can't write");
	if (write (fdout, buf, size) != size)
		return loc_set (rp, RP_FIO, "Can't write");
	if (read (fdin, (char *)rp, sizeof *rp) != sizeof *rp)
		return loc_set (rp, RP_LIO, "Can't get response");
	return loc_set (rp, RP_OK, "OK");
}


int	loc_close (rp)
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_CLOSE;
	return send_pac (&packet, rp);
}


int	loc_open (file, mode, flag, rp)
char	*file;
char	*mode;
int	flag;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_OPEN;
	(void) strcpy (packet.s1, file);
	(void) strcpy (packet.s2, mode);
	packet.size = flag;
	return send_pac (&packet, rp);
}


loc_move (s1, s2, rp)
char	*s1, *s2;
RP_Buf	*rp;
{
	struct loc_packet packet;
	packet.command = LP_MOVE;
	(void) strcpy (packet.s1, s1);
	(void) strcpy (packet.s2, s2);
	return send_pac (&packet, rp);
}



int	loc_opendir (dir, rp)
char	*dir;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_OPENDIR;
	(void) strcpy (packet.s1, dir);
	return send_pac (&packet, rp);
}


int	loc_closedir (rp)	/* Close previous OPENDIR */
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_CLOSEDIR;
	return send_pac (&packet, rp);
}



int	loc_system (str, rp)
char	*str;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_SYSTEM;
	(void) strcpy (packet.s1, str);
	return send_pac (&packet, rp);
}


int	loc_isdir (dir, rp)
char	*dir;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_ISDIR;
	(void) strcpy (packet.s1, dir);
	return send_pac (&packet, rp);
}



int	loc_nextmess (dir, rp)
char	*dir;
RP_Buf	*rp;
{
	struct loc_packet packet;

	packet.command = LP_NEXTMESS;
	(void) strcpy (packet.s1, dir);
	return send_pac (&packet, rp);
}


loc_end()
{
	int	cpid;

	if (fdout != NOTOK) {
		(void) close (fdout);
		fdout = NOTOK;
	}
	if (fdin != NOTOK) {
		(void) close (fdin);
		fdin = NOTOK;
	}

	while ((cpid = wait ( (union wait *)0 )) != pid && cpid != -1)
		continue;
	return RP_OK;
}



/* -------------------  Static Routines  ------------------------------------ */




static int loc_set (rp, val, str)
RP_Buf	*rp;
int	val;
char	*str;
{
	rp -> rp_val = val;
	(void) strcpy (rp -> rp_line, str);
	return val;
}


static int send_pac (pp, rp)
struct loc_packet *pp;
RP_Buf	*rp;
{
	if (write (fdout, (char *)pp, sizeof *pp) != sizeof *pp)
		return loc_set (rp, RP_FIO, "Can't write");
	if (read (fdin, (char *)rp, sizeof *rp) != sizeof *rp)
		return loc_set (rp, RP_LIO, "Can't get response");
	return rp -> rp_val;
}


static do_child_stuff()
{
	struct loc_packet packet;
	struct loc_packet *pp = &packet;
	RP_Buf	rps, *rp = &rps;

	while (read (fdin, (char *)pp, sizeof (*pp)) == sizeof (*pp)) {
		(void) do_packet (pp, rp);
		if (write (fdout, (char *)rp, sizeof *rp) != sizeof *rp) {
			PP_SLOG (LLOG_EXCEPTIONS, "write",
			      ("Write error on packet"));
			exit (1);
		}
	}
	exit (0);
}



static int do_packet (pp, rp)
struct loc_packet *pp;
RP_Buf *rp;
{
	switch (pp->command) {
	    case LP_OPEN:
		if (cur_fp != NULL)
			(void) fclose (cur_fp);
		if ((cur_fp = fopen(pp -> s1, pp -> s2)) == NULL) {
			PP_SLOG (LLOG_EXCEPTIONS, "fopen",
			      ("Can't open file %s mode %s", 
				pp -> s1, pp -> s2));
			return loc_set (rp, RP_FOPN, "Can't open file");
		}
		if (pp -> size) {
			if (flock (fileno (cur_fp), LOCK_EX) == NOTOK) {
				PP_SLOG (LLOG_EXCEPTIONS, "flock",
				      ("Can't lock file %s", pp -> s1))
				(void) fclose (cur_fp);
				cur_fp = NULL;
				return loc_set (rp, RP_LOCK, "Can't lock file");
			}
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_CLOSE:
		if (cur_fp != NULL) {
			(void) fclose (cur_fp);
			cur_fp = NULL;
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_CD:
		if (chdir (pp -> s1) == NOTOK) {
			PP_SLOG (LLOG_EXCEPTIONS, "chdir",
			      ("Can't change directory to %s", pp -> s1));
			return loc_set (rp, RP_LIO, "Can't chdir");
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_WRITE:
		if (cur_fp == NULL)
			return loc_set (rp, RP_FIO, "No file open");
		return copy (fdin, cur_fp, pp -> size, rp);

	    case LP_MOVE:
		if (rename (pp -> s1, pp-> s2) == NOTOK) {
			PP_SLOG (LLOG_EXCEPTIONS, "rename",
			      ("Can't move %s to %s", pp -> s1, pp -> s2));
			return loc_set (rp, RP_LIO, "Can't move file");
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_MKDIR:
		if (mkdir (pp -> s1, pp -> size) == NOTOK) {
			PP_SLOG (LLOG_EXCEPTIONS, "mkdir",
			      ("Can't make directory %s mode %o",
			       pp->s1, pp->size));
			return loc_set (rp, RP_LIO, "Can't make directory");
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_OPENDIR:
		if (mb_dp != NULL)
			(void) closedir (mb_dp);
		if ((mb_dp = opendir(pp -> s1)) == NULL) {
			PP_SLOG (LLOG_EXCEPTIONS, "opendir",
			      ("Can't open directory %s", pp->s1));
			return loc_set (rp, RP_LIO, "Can't open directory");
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_CLOSEDIR:
		if (mb_dp != NULL) {
			(void) closedir (mb_dp);
			mb_dp = NULL;
		}
		return loc_set (rp, RP_OK, "OK");

	    case LP_SYSTEM:
		if ((sysreply = system(pp -> s1)) ) {
			PP_SLOG (LLOG_EXCEPTIONS, "system", 
			("system() call returns value %d",sysreply));
		}

		/* I'm not sure if I should do this */
		return loc_set (rp, sysreply, "OK");	

	    case LP_NEXTMESS:
		if (mb_dp == NULL) {
			return loc_set (rp, RP_FIO, "No directory open");
		}
		return loc_set (rp, RP_OK, next_message (mb_dp, pp -> s1) );

	    case LP_ISDIR:
		if (!isdir (pp -> s1)) {
			return loc_set (rp, RP_MECH, "Not a directory");
		}
		return loc_set (rp, RP_OK, "OK");

	    default:
		PP_LOG (LLOG_EXCEPTIONS, ("Unknown command %d"));
		return loc_set (rp, RP_PARM, "Unknown command");
	}
}


static int copy (in, out, size, rp)
int	in;
FILE	*out;
int	size;
RP_Buf	*rp;
{
	char buffer[BUFSIZ];
	int	n;

	(void) loc_set (rp, RP_OK, "OK");
	while (size > 0) {
		n = read (in, buffer,
			  size > sizeof buffer ? sizeof buffer : size);
		if (n <= 0)
			return loc_set (rp, RP_FIO, "Read error");
		if (out)
			if (fwrite (buffer, 1, n, out) != n)
				(void) loc_set (rp, RP_FIO, "Write error");
		size -= n;
	}
	return rp -> rp_val;
}



/* --- generate next message number for this directory --- */
static char*  next_message(mb_dp,dir) 
DIR     *mb_dp;
char    *dir;
{
        struct  dirent *dp;
        int     i,biggest=0;
        char    fullname[MAXPATHLENGTH];

        for(dp = readdir(mb_dp); dp != NULLDCT; dp = readdir(mb_dp)) {

                sprintf(fullname,"%s/%s",dir,dp->d_name);
                if (isdir(fullname)) {
                        i = val(dp->d_name);
                        if (i > biggest)
                                biggest=i;
                }
        }
        PP_DBG (("loc_proto/next_message number in %s = %d",dir,biggest+1));
	sprintf(fullname,"%d", (biggest + 1) );
        return(fullname);
}



/* return value of s iff composed of digits, else 0 */
static val(s)  
char    *s;
{
        int     i,n;

        for(i=0,n=0; s[i] != NULL ; ++i) {
                if (s[i] >= '0' && s[i] <= '9')
                        n = 10 * n + s[i] - '0';
                else {
                        n=0;
                        break;
                }
        }
        PP_DBG (("loc_proto/val(%s)=%d", s,n));
        return(n);
}


static isdir (name)
char    *name;
{
        struct stat     st_rec;

        if (!isstr (name))
                return (FALSE);

        if (stat (name, &st_rec) == NOTOK) {
                PP_SLOG (LLOG_EXCEPTIONS, name, ("isdir: Unable to stat"));
                return (FALSE);
        }

        switch (st_rec.st_mode & S_IFMT) {
        case S_IFDIR:
                PP_DBG (("loc_proto/isdir (%s = TRUE)", name));
                return (TRUE);
        default:
                PP_DBG (("loc_proto/isdir (%s = FALSE)", name));
                return (FALSE);
        }
}
