/* prm.h: submit parameter structure */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/prm.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: prm.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_PRM
#define _H_PRM


/*
When talking to submit this is the initial structure that is passed down.
All the work should be done by mm_winit() to which is passed the relevent
structure
*/

struct  prm_vars {
	char    *prm_logfile;           /* log file */
	int     prm_loglevel;           /* level of logging */
	int     prm_opts;               /* various options*/
/* prm_opts is a bit field:  (more to follow) */
#define PRM_OPTS_TOTAL          2
#define PRM_NONE		0x0
#define PRM_ACCEPTALL		0x1
#define PRM_NOTRACE		0x2
	char	*prm_passwd;
};

extern void	prm_init ();
extern void	prm_free ();
#endif
