/* tjoin1.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/tjoin1.c,v 6.0 1991/12/18 20:35:29 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/tjoin1.c,v 6.0 1991/12/18 20:35:29 jpo Rel $
 *
 * $Log: tjoin1.c,v $
 * Revision 6.0  1991/12/18  20:35:29  jpo
 * Release 6.0
 *
 */



#include        <stdio.h>
#include	"tjoin.h"

extern	char	*av0;
extern	int	yylineno, Debug;
extern	HOST	*HostList;
extern	CHAN	*ChanList;
extern char	*FastInChanList();



FreeChanInfo (list)
register CHAN	*list;
{
	register CHAN	*startlist = list,
			*chptr;

	for (; list != NULL; list = chptr) {
		chptr = list -> ch_next;
		if (Debug == 1)  	
			printf ("FreeChanInfo %s\n", list->ch.name);
		if (list)
			free ((char *)list);
	}

	startlist = NULL;
}



RejectHost (host)
register HOST	*host;
{
	register CHAN	*c1;

	fprintf (stderr,
"%s: Channel present, but LHS & RHS hosts differ at line %d\n",
		 av0, yylineno - 1);

	for (c1 = host -> hst_chan; c1 != NULL; c1 = c1 -> ch_next) {
		if (Debug == 1) 
			printf ("Free %x - RejectHost ChanName\n", c1->ch.name);

		if (c1 -> ch.name) {
			free (c1 -> ch.name);
			c1 -> ch.name = NULL;
		}
	}

	if (Debug == 1) 
		printf ("Free %x - RejectHost HostName\n", host -> hst_name);

	if (host -> hst_name) {
		free (host -> hst_name);
		host -> hst_name = NULL;
	}
}


StartChanList (str)
char	*str;
{
	if (ChanList != NULL) {
		fprintf (stderr, "%s: StartChanList - ChanList not NULL\n", 
			 av0);
		exit (1);
	}

	if ((ChanList = (CHAN *) malloc (sizeof (CHAN))) == NULL) {
		fprintf (stderr, "%s: StartChanList - No Memory\n", av0);
		exit (1);
	}

	if (Debug == 1)
		printf ("StartChanList - %s\n", str ? str : "null");

	bzero ((char *)ChanList, sizeof(*ChanList));
	ChanList -> ch.name = str;
}


AddToChanList (str)
char	*str;
{
	register CHAN	*chan;

	if ((chan = (CHAN *) malloc (sizeof (CHAN))) == NULL) {
		fprintf (stderr, "%s: AddToChanList - No Memory\n");
		exit (1);
	}

	if (Debug == 1) 
		printf ("AddToChanList - %s\n", str ? str : "null");

	bzero ((char *)chan, sizeof(*chan));
	chan -> ch_next = ChanList;
	ChanList = chan;
	chan -> ch.name = str;
}


StartHostList (str)
char	*str;
{
	if (HostList != NULL) {
		fprintf (stderr, "%s: StartHostList - HostList not NULL\n", 
			 av0);
		exit (1);
	}

	if ((HostList = (HOST *) malloc (sizeof (HOST))) == NULL) {
		fprintf (stderr, "%s: StartHostList - No Memory\n", av0);
		exit (1);
	}

	if (Debug == 1) 
		printf ("StartHostList - %s\n", str ? str : "null");

	bzero ((char *)HostList, sizeof(*HostList));
	HostList -> hst_name = str;
	HostList -> hst_chan = ChanList;
}


AddToHostList (str)
char	*str;
{
	register HOST	*host;

	if ((host = (HOST *) malloc (sizeof (HOST))) == NULL) {
		fprintf (stderr, "%s: AddToHostList - No Memory\n");
		exit (1);
	}

	if (Debug == 1) 
		printf ("AddToHostList - %s\n", str  ?  str : "null");

	bzero ((char *)host, sizeof(*host));
	host -> hst_next = HostList;
	HostList = host;
	host -> hst_name = str;
	host -> hst_chan = ChanList;
}


char *FastInChanList (list, name)
register CHAN	*list;
char		*name;
{
	if (name == NULL)
		return (NULL);
	for (; list != NULL; list = list -> ch_next)
		if (list -> ch.name == name)
			return (list -> ch.name);
	return (NULL);
}


char *InChanList (list, name)
register CHAN	*list;
char		*name;
{
	if (name == NULL)
		return (NULL);
	for (; list != NULL; list = list -> ch_next)
		if (strcmp (list -> ch.name, name) == 0)
			return (list -> ch.name);
	return (NULL);
}


CHAN *InsertChan (list, item)
register CHAN	*list, *item;
{
	register CHAN	*c1;

	if (list == NULL)
		return (item);
	for (c1 = list; c1 -> ch_next != NULL; c1 = c1 -> ch_next)
		continue;
	c1 -> ch_next = item;
	return (list);
}
