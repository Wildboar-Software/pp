/* t-tracecheck: small wrap around to test checking of traces */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/t-tracecheck.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/t-tracecheck.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: t-tracecheck.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "prm.h"
#include "dr.h"

#define MAX_TRACE_HOPS  15

static int inTraceList (tp, list)
Trace	*tp, *list;
{
	int	retval = NOTOK;

	while (retval == NOTOK && list != NULL) {
		if (trace_equ(tp, list) == 0)
			retval = OK;
		else
			list = list -> trace_next;
	}
	return retval;
}

static void addToTraceList(this, to)
Trace	*this, **to;
{
	Trace	*next;
	if (this == NULL)
		return;
	next = this -> trace_next;
	this -> trace_next = NULL;
	trace_add(to, trace_dup(this));
	this -> trace_next = next;
}
      

void tracecheck_mgt (qp)
Q_struct	*qp;
{
	register Trace	*tp;
	Trace		*list = NULL;
	int		nhops = 0;
	char		buf[BUFSIZ];
	PP_DBG (("submit/tracecheck_mgt (qp)"));

	for (tp = qp->trace; tp; tp = tp -> trace_next) {
		/* -- too many hops -- */

		if (++nhops > MAX_TRACE_HOPS) {
			(void) sprintf (buf,
				"%s%s %d",
				"This Message has travelled a long time, ",
				"there are too many Trace hops", nhops);
/*			message_failure (DRR_UNABLE_TO_TRANSFER,
					 DRD_LOOP_DETECTED, buf);*/
			if (list != NULL) trace_free(list);
			return;
		}

		if (tp -> trace_mta != NULLCP) {
			/* can only check against those with mta specified */
			if (inTraceList(tp, list) == OK) {
				(void) sprintf (buf, "%s%s",
						"A Loop has been detected in the Trace ",
						"field for this Message");
/*				message_failure (DRR_UNABLE_TO_TRANSFER,
						 DRD_LOOP_DETECTED, buf);*/
				if (list != NULL) trace_free(list);
				return;
			} else
				addToTraceList(tp, &list);
		}
	}
	if (list != NULL) trace_free (list);
}

main (argc, argv)
int  argc;
char **argv;
{
	struct prm_vars	prm;
	Q_struct	que;
	ADDR		*sender = NULL;
	ADDR		*recips = NULL;
	int 		rcount;
	if (argc < 2) {
		printf("usage : %s msg", argv[0]);
		exit(0);
	}
	sys_init(argv[0]);

	if (rp_isbad(rd_msg(argv[1],&prm,&que,&sender,&recips,&rcount))) {
		printf("rd_msg err: '%s'",argv[1]);
		rd_end();
		exit(0);
	}
	tracecheck_mgt (&que);
	rd_end();
}
