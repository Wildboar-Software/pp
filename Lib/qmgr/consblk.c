/* consblk.c: ROS calling routines */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/consblk.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/consblk.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: consblk.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "consblk.h"

static struct cons_opblk Heads, *Head;
static int first_time_only;

struct cons_opblk *newcopblk (fd, id, fnx)
int fd, id;
IFP	fnx;
{
	struct cons_opblk *op;

	if (first_time_only == 0) {
		first_time_only = 1;
		Head = &Heads;
		Head -> next = Head -> prev = Head;
	}

	op = (struct cons_opblk *) smalloc (sizeof *op);
	op -> fd = fd;
	op -> id = id;
	op -> fnx = fnx;
	insque (op, Head -> prev);
	return op;
}

struct cons_opblk *find_copblk (fd, id)
int fd, id;
{
	struct cons_opblk *op;

	for (op = Head -> next; op != Head; op = op -> next) {
		if (op -> fd == fd && op -> id == id)
			return op;
	}
	return NULL;
}

void free_copblk (op)
struct cons_opblk *op;
{
	remque (op);
	free ((char *)op);
}
