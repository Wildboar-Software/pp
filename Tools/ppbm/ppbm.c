/* ppbm.c - measures PP throughput on a filesystem */

static char Rcsid[] = "$Header: /xtel/pp/pp-beta/Tools/ppbm/RCS/ppbm.c,v 6.0 1991/12/18 20:32:20 jpo Rel $";

/*
 * $Header: /xtel/pp/pp-beta/Tools/ppbm/RCS/ppbm.c,v 6.0 1991/12/18 20:32:20 jpo Rel $
 *
 * $Log: ppbm.c,v $
 * Revision 6.0  1991/12/18  20:32:20  jpo
 * Release 6.0
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <dirent.h>
#ifndef SYS5
#include <sys/file.h>
#include <sys/resource.h>
#endif
#include <errno.h>

#define NOTOK	(-1)
#define MAXPATHLENGTH	1024
#define hertz 60		/* the (times(2/3) manual page says this
				   is a constant but is it really ? */

#ifndef SYS5
struct rusage rusage;
#endif
int addr_file_size = 600;	/* how much to put in the address file */
int create_message_sleep = 0;	
int delete_message_sleep = 0;
int batch_sleep = 0;
int usesubdirs = 0;
#ifdef LOCK_EX
int do_flock = 1;		/* should we flock things? */
#else
int do_flock = 0;
#endif
int do_sync = 1;		/* should files be fsync(2)ed ? */
int n_batch = 3;		/* # batches per child */
int message_batch = 10;		/* # messages per batch */
int n_child = 5;
int verbose = 0;
int mypid, childno;

char * base_dir  = ".";		/* where to create the test messages */
char buf[60000];

void create_message(), rm_tree();

/* serror */
serror (routine, what)
char *routine, *what;
{
	extern int errno;
	extern char *strerror(int);
	char sbuf[BUFSIZ];
	char *p;

	p = strerror(errno);
	(void) sprintf (sbuf, "%s %s %s\n", routine, what ? what : "", p);
	(void) write (2, sbuf, strlen(sbuf));
}

/* main worker function */
child(cno)
int cno;
{
    int i, j;
    mypid = getpid ();
    childno = cno;

    for (i = 0; i < n_batch; i++) {
	for (j = 0; j < message_batch; j++) {
	    create_message(j);
	    if (create_message_sleep) sleep(create_message_sleep);
	}
	for (j = 0; j < message_batch; j++) {
	    delete_message(j);
	    if (delete_message_sleep) sleep(delete_message_sleep);
	}
	if (batch_sleep) sleep(batch_sleep);
    }
}

/* create a message with address file + 1 header + 1 body part */
void create_message(i)
int i;
{
	char msg[MAXPATHLENGTH];
	char base[MAXPATHLENGTH];
	char file[MAXPATHLENGTH];
	int adr, fd;

	if (usesubdirs)
		(void) sprintf (msg, "%s/%d/msg.%d-%d",
				base_dir, (childno+i) % usesubdirs, mypid, i);
	else
		(void) sprintf(msg, "%s/msg.%d-%d", base_dir, mypid, i);
	if (mkdir(msg, 0777) == NOTOK) {
		serror ("mkdir", msg);
		return;
	}
	(void) sprintf(file, "%s/addr", msg);
	if ((adr = open(file, O_CREAT | O_WRONLY, 0666)) == NOTOK) {
		serror("open", file);
		return;
	}
#ifdef LOCK_EX
	if (do_flock && flock (adr, LOCK_EX) == NOTOK)
		serror ("flock", file);
#endif
	if (write(adr, buf, addr_file_size) != addr_file_size) {
		serror("write", file);
		(void) close (adr);
		return;
	}

	(void) sprintf(base, "%s/base", msg);
	if (mkdir(base, 0777) == NOTOK) {
		serror ("mkdir", base);
		return;
	}
	(void) sprintf(file, "%s/hdr.822", base);
	if ((fd = open(file, O_CREAT | O_WRONLY, 0666)) == NOTOK) {
		serror("open", file);
		(void) close (adr);
		return;
	}
	if (write(fd, buf, header_size(i)) != header_size(i)) {
		serror ("write", file);
		(void) close (fd);
		(void) close (adr);
		return;
	}
	    
	if (do_sync && fsync(fd) == NOTOK)
		serror ("fsync", file);
	if (close(fd) == NOTOK)
		serror ("close", file);
	(void) sprintf(file, "%s/1.ia5", base);
	if ((fd = open(file, O_CREAT | O_WRONLY, 0666)) == NOTOK)
		serror ("open", file);
	else {
		if (write(fd, buf, message_size(i)) != message_size(i))
			serror ("write", file);
		if (do_sync && fsync(fd) == NOTOK)
			serror ("fsync", file);
		if (close(fd) == NOTOK)
			serror ("close", file);
	}
	if (do_sync && fsync(adr) == NOTOK)
		serror ("fsync", "addr");
	if (close(adr) == NOTOK)
		serror ("close", "addr");
}

header_size(i)
int i;
{
    return(600);
}

message_size(i)
int i;
{
    return(6875);	/* average of a couple of days on slough */
}

delete_message(i)
int i;
{
    char msg[MAXPATHLENGTH];
    
    if (usesubdirs)
	    (void) sprintf(msg, "%s/%d/msg.%d-%d", base_dir,
			   (childno + i) % usesubdirs, mypid, i);
    else
	    (void) sprintf(msg, "%s/msg.%d-%d", base_dir, mypid, i);
    rm_tree(msg);
}

void rm_tree(dir)
char * dir;
{
	char file[MAXPATHLENGTH];
	DIR *d = opendir(dir);
	struct dirent * e;
	struct stat st;

	if (d == NULL) {
		serror ("opendir", dir);
		return;
	}

	while (e = readdir(d)) {
		if (strcmp (e -> d_name, ".") == 0 ||
		    strcmp (e -> d_name, "..") == 0)
			continue;
		(void) sprintf(file, "%s/%s", dir, e->d_name);
		if (stat(file, &st) == NOTOK) {
			serror ("stat", file);
			continue;
		}
		if ((st.st_mode&S_IFMT) == S_IFREG) {
			int fd;
			if ((fd = open(file, O_RDONLY, 0)) != NOTOK) {
				if (read (fd, buf, st.st_size) != st.st_size)
					serror ("read", file);
				(void) close(fd);
			}
			else
				serror ("open", file);
			if (verbose)
				printf ("Unlink file %s\n", file);
			if (unlink(file) == NOTOK)
				serror ("unlink", file);
		} else if ((st.st_mode & S_IFMT) == S_IFDIR)
			rm_tree(file);
	}
	if (verbose)
		printf ("rmdir %s\n", dir);
	if (closedir(d) == NOTOK)
		serror ("closedir", dir);
	if (rmdir(dir) == NOTOK)
		serror ("rmdir", dir);
}

makesubdirs ()
{
	char buf[BUFSIZ];
	int i;

	for (i = 0; i < usesubdirs; i++) {
		(void) sprintf (buf, "%s/%d", base_dir, i);
		if (mkdir (buf, 0777) == NOTOK && errno != EEXIST)
			serror ("mkdir", buf);
	}
}

main(argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	int i, c;
	struct tms tms;
#ifndef SYS5
	struct timeval startt, endt, tdiff;
	struct rusage rus;
	double mps;
#else
	time_t startt, endt;
#endif
#ifndef UNIONWAIT
	int wt;
#else
	union wait wt;
#endif

	while ((c = getopt(argc, argv, "c:d:fFn:m:s:S:v")) != EOF) {
		switch (c) {

		    case 'c':
			n_child = atoi(optarg);
			break;

		    case 'd':
			base_dir = optarg;
			break;

		    case 'f':
			do_sync = 0;
			break;

		    case 'F':
			do_flock = 0;
			break;

		    case 'n':
			n_batch = atoi(optarg);
			break;

		    case 'm':
			message_batch = atoi(optarg);
			break;

		    case 's':
			batch_sleep = atoi(optarg);
			break;
		    case 'S':
			usesubdirs = atoi(optarg);
			break;
		    case 'v':
			verbose = 1;
			break;

		    case '?':
		    default:
			fprintf(stderr,
"usage: %s [-vf] [-c children] [-n iterations] [-m messages-per-iterations] [-d base_dir]\n",
				argv[0]);
			exit(1);
		}
	}

	if (usesubdirs)
		makesubdirs ();
#ifndef SYS5
	gettimeofday (&startt, (struct timezone *)0);
#else
	startt = time((time_t *)0);
#endif
	(void) fflush(stdout);
	for (i = 0; i < n_child; i++) {
		switch(fork()) {

		    case 0:
			child(i);
			exit(0);
	
		    case -1:
			perror("fork");
			exit(1);
		}
	}
#ifndef SYS5
	while (wait3(
#ifdef UNIONWAIT
		&wt.w_status,
#else
		&wt,
#endif
		 0, &rus) != -1) {
		gettimeofday (&endt, (struct timezone *)0);
		add_ru (&rus);
	}
#else
	while (wait(&wt.w_status) != -1)
		;
	times(&tms);
	endt = time((time_t *)0);
#endif
	printf ("parameters: flock=%d, fsync=%d, verb=%d\n",
		do_flock, do_sync, verbose);
	printf("  children=%d, iterations=%d, msgs/iterations=%d = %d msgs\n",
	       n_child, n_batch, message_batch,
	       n_child * n_batch * message_batch);

#define ELAPASED	"elapsed time"
#define SYST		"system time"
#define USERT		"user time"
#ifndef SYS5
	tvsub (&tdiff, &endt, &startt);
	printf ("%-14s %d.%06d\n", ELAPASED, tdiff.tv_sec, tdiff.tv_usec);
	printf ("%-14s %d.%06d\n", USERT, rusage.ru_utime.tv_sec,
		rusage.ru_utime.tv_usec);
	printf ("%-14s %d.%06d\n", SYST, rusage.ru_stime.tv_sec,
		rusage.ru_stime.tv_usec);
	printf ("I/O blocks in=%d, blocks out=%d\n", rusage.ru_inblock,
		rusage.ru_oublock);
	mps = tdiff.tv_sec + (double)tdiff.tv_usec / 1000000.0;
	mps = (n_child * n_batch * message_batch) / mps;
	printf ("Gives messages/sec=%.2f\n", mps);
#else
	printf("%-14s %d\n", ELAPASED, endt - startt);
	printf ("%-14s %.2f\n", USERT, (float)tms.tms_cutime / hertz);
	printf ("%-14s %.2f\n", SYST, (float)tms.tms_cstime / hertz);
	printf ("Gives messages-per-sec=%.2f\n", 
		n_child * n_batch * message_batch / (float) (endt - startt));
#endif
}


#ifndef SYS5
add_ru (ru)
struct rusage *ru;
{
        tvadd(&rusage.ru_utime, &ru->ru_utime);
        tvadd(&rusage.ru_stime, &ru->ru_stime);
#define addru(x) rusage.x += ru -> x;
        addru(ru_maxrss);
        addru(ru_ixrss);
        addru(ru_idrss);
        addru(ru_isrss);
        addru(ru_minflt);
        addru(ru_majflt);
        addru(ru_nswap);
        addru(ru_inblock);
        addru(ru_oublock);
        addru(ru_msgsnd);
        addru(ru_msgrcv);
        addru(ru_nsignals);
        addru(ru_nvcsw);
        addru(ru_nivcsw);
#undef addru
}

tvsub (tdiff, t1, t0)
register struct timeval *tdiff,
                        *t1,
                        *t0;
{

	tdiff -> tv_sec = t1 -> tv_sec - t0 -> tv_sec;
	tdiff -> tv_usec = t1 -> tv_usec - t0 -> tv_usec;
	if (tdiff -> tv_usec < 0)
		tdiff -> tv_sec--, tdiff -> tv_usec += 1000000;
}

tvadd (td, tplus)
struct timeval *td, *tplus;
{
        td -> tv_sec += tplus -> tv_sec;
        td -> tv_usec += tplus -> tv_usec;
        if (td -> tv_usec > 1000000)
                td -> tv_usec -= 1000000, td -> tv_sec ++;
}

#endif
