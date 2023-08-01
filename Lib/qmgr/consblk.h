/* cons.h: console library routines - private */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/consblk.h,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: consblk.h,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 *
 */

struct cons_opblk {
	struct cons_opblk *next, *prev;
	int fd;
	int id;
	IFP	fnx;
};

struct cons_opblk *newcopblk ();
struct cons_opblk *find_copblk ();
void free_copblk ();
