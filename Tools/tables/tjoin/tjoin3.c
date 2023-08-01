/* tjoin3.c: */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/tjoin3.c,v 6.0 1991/12/18 20:35:29 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/tjoin3.c,v 6.0 1991/12/18 20:35:29 jpo Rel $
 *
 * $Log: tjoin3.c,v $
 * Revision 6.0  1991/12/18  20:35:29  jpo
 * Release 6.0
 *
 */



#include        <stdio.h>
#include        "tjoin.h"

extern  int     yylineno;

int     	PrintNode(), CheckARs();
SBUFF   	*InitStringStore();
MTA     	*NameToMta(), *InsertMta(), *GetMta();
CHAN    	*InsertChan(), *GetChan();
char    	*StoreStr(), *InChanList();

extern  char    *av0, *MyName;
extern  HOST    *HostList;
extern  CHAN    *ChanList;
extern  CHAN    *ValidChanList;
extern  MTA     *MtaTree;
extern  SBUFF   *ARStringStore, *HostStringStore;
extern  int     Debug, PrintRoute, DirectFirst, ComplexOutput;




/* ----------------------  Begin  Routines  --------------------------------- */



Usage()
{
	fprintf (stderr, "Usage: %s [flags] Hostname ChannelList\n", av0);
	fprintf (stderr, "\t-r\tPrint Routes\n");
	fprintf (stderr, "\t-d\tMost Direct Route First\n");
	fprintf (stderr, "\t-c\tComplex Output\n");
	fprintf (stderr, "\t-h\tThis Message\n");
	exit (1);
}



DoFlag (str)
register char   *str;
{
	for (; *str != '\00'; str++) {

		if (*str == 'h')
			Usage();

		if (*str == 'd') {
			DirectFirst = 1;
			continue;
		}

		if (*str == 'c') {
			ComplexOutput = 1;
			continue;
		}

		if (*str == 'r') {
			PrintRoute = 1;
			continue;
		}

		fprintf (stderr, "%s: Unknown flag %c\n", av0, *str);
		exit (1);
	}
}



CheckARs (mta)
register MTA    *mta;
{
	register CHAN   *chan;
	int             lineno;
	char            buff [500];     /* --- Lex Buffer Size --- */


	/* --- Go along the list of AR names. --- */
	/* --- These are of form "Name Lineno" --- */

	for (chan = mta->mta_arlist; chan != NULL; chan = chan->ch_next) {
		if (sscanf (chan->ch.name, "%s %d", &buff [0], &lineno) != 2) {
			/* --- Internal error - Horrible --- */
			fprintf (stderr, "%s: CheckARs - AAARGH!!\n", av0);
			exit (1);
		}

		if ((chan->ch.mta = NameToMta (&buff [0])) == NULL) {
			fprintf (stderr, "%s: AR %s ", av0, &buff [0]);
			fprintf (stderr, "does not exist at line %d\n", lineno);
		}
	}
}



DefineChan (str)
char    *str;
{
	register CHAN   *chan;

	chan = GetChan();
	chan->ch.name = str;	/* --- Leave this data where it is --- */
	chan->ch_next = NULL;
	ValidChanList = InsertChan (ValidChanList, chan);
}


/* --- *** ---
This routine does all checking Possible. It can not check for
validity of AR hosts, but can check that channels exist. If there are
any problems with the channels, the line is discarded. This routine
also start to modify the data structures and free the memory used by
the LEX part of the program.
--- *** --- */

int CheckHost (hostname)
char    *hostname;
{
	register CHAN   *c1;
	register char   *ptr;
	register HOST   *host;
	int             valid;

	valid = 0;

	for (host = HostList; host != NULL; host = host->hst_next) {

		/* --- Check name against hostname --- */

		if (host->hst_chan != NULL)
			if (strcmp (hostname, host->hst_name) != 0) {
				RejectHost (host);
				continue;
			}
			else {
				if (Debug == 1) 
				       printf ("CheckHost HostName - Free %s\n",
						host->hst_name ? 
						host->hst_name : "null");

				if (host -> hst_name) {
					free (host->hst_name);
					host->hst_name = NULL;
				}
			}
		else {
			/* ---  *** ---
			Check That I am not the AR here. If I am
			then just zap it and say nothing
			--- *** --- */
			if (strcmp (MyName, host->hst_name) == 0) {
				if (Debug == 1) 
					printf ("CheckHost AR HostName\n", 
						host->hst_name ? 
						host->hst_name : "null");

				if (host->hst_name) {
					free (host->hst_name);
					host->hst_name = NULL;
				}

				continue;
			}

			/* --- Check the MTA does not have AR of itself --- */
			if (strcmp (hostname, host->hst_name) == 0) {
				/* --- What a dummy --- */
				fprintf (stderr, "%s: CheckHost - Line ", av0);
				fprintf (stderr, "%d has AR of ", yylineno - 1);
				fprintf (stderr, "itself - Ignored\n");

				if (Debug == 1) 
				   printf ("CheckHost AR HostName - Free %s\n",
						host->hst_name ? 
						host->hst_name : "null");

				if (host->hst_name) {
					free (host->hst_name);
					host->hst_name = NULL;
				}
				continue;
			}

			valid = 1;      /* --- Application Relay --- */
			continue;
		}

		for (c1 = host->hst_chan; c1 != NULL; c1 = c1->ch_next) {
			/* --- c1->ch.name points at a LEX buffer --- */
			ptr = InChanList (ValidChanList, c1->ch.name);
			if (ptr == NULL) {
				if (Debug == 2) {
				  fprintf (stderr, "%s: Invalid Channel ", av0);
				  fprintf (stderr, "%s at line ", c1->ch.name);
				  fprintf (stderr, "%d - ", yylineno - 1);
				  fprintf (stderr, "Ignored\n");
				}
			}
			else
				valid = 1;

			if (Debug == 1) 
			   printf ("CheckHost - ChanName Free %s\n",
					c1->ch.name ? c1->ch.name : "null");

			if (c1->ch.name)
				free (c1->ch.name);

			c1->ch.name = ptr;
		}
	}

	return (valid);
}



/* --- Check that Channels exist --- */

CheckChan (c1)
register CHAN   *c1;
	{
	int     valid;
	char    *ptr;

	for (valid = 0; c1 != NULL; c1 = c1->ch_next) {

		/* --- c1->ch.name points at a LEX buffer --- */
		ptr = InChanList (ValidChanList, c1->ch.name);
		if (ptr == NULL) {
			if (Debug == 2) {
				fprintf (stderr, "%s: Invalid Channel ", av0);
				fprintf (stderr, "%s at line ", c1->ch.name);
				fprintf (stderr, "%d - Ignored\n", yylineno- 1);
			}
		}
		else
			valid = 1;


		if (Debug == 1) 
		   printf ("CheckChan - ChanName Free %s\n",
				c1->ch.name ? c1->ch.name : "null");

		if (c1->ch.name)
			free (c1->ch.name);

		c1->ch.name = ptr;
	}

	return (valid);
}




/* --- *** ---
This routine takes the valid data in the input line structure
and merges it into the internal database format. It also frees up all
storeage used by the line input routines.
--- *** --- */


StoreHost (hostname)
char    *hostname;
{
	register MTA    *mta;

	if ((mta = NameToMta (hostname)) == NULL) {
		mta = GetMta();
		mta->mta_name = StoreStr (HostStringStore, hostname);
		MtaTree = InsertMta (MtaTree, mta);
	}

	if (mta->mta_domain != NULL) {
		/* --- Very Serious - Don't know what to do --- */
		fprintf (stderr, "%s: StoreHost - Line %d ", av0, yylineno - 1);
		fprintf (stderr, "uses mixed types - Ignored\n");
		FreeHostInfo (HostList);
		return;
	}

	MergeData (mta, HostList);
}



StoreChans (name)
char    *name;
{
	register MTA    *mta;

	if ((mta = NameToMta (name)) == NULL) {
		mta = GetMta();
		mta->mta_name = StoreStr (HostStringStore, name);
		MtaTree = InsertMta (MtaTree, mta);
	}

	if (mta->mta_arlist != NULL || mta->mta_chan != NULL) {
		/* --- Very Serious - Don't know what to do --- */
		fprintf (stderr, "%s: StoreHost - Line %d ", av0, yylineno - 1);
		fprintf (stderr, "uses mixed types - Ignored\n");
		FreeChanInfo (ChanList);
		return;
	}

	MergeChanData (mta, ChanList);
}



FreeHostInfo (host)
register HOST   *host;
{
	register HOST   *h2;
	register CHAN   *c1, *c2;

	for (; host != NULL; host = h2) {
		h2 = host->hst_next;

		for (c1 = host->hst_chan ; c1 != NULL; c1 = c2) {

			c2 = c1->ch_next;

			if (Debug == 1) 
		   		printf ("FreeHostInfo - Chan Free %s\n",
					c1->ch.name ? c1->ch.name : "null");

			free ((char *)c1);
			c1 = NULL;
		}

		if (host->hst_name != NULL) {
			if (Debug == 1) 
		   		printf ("FreeHostInfo - HostName Free %s\n",
				host->hst_name ? host->hst_name : "null");
			free (host->hst_name);
			host->hst_name = NULL;
		}

		if (Debug == 1) 
			printf ("FreeHostInfo - Host Free %x\n", host);

		if (host) {
			free ((char *)host);
			host = NULL;
		}
	}
}


yyerror (str)
char    *str;
{
        fprintf (stderr, "%s at line number %d\n", str, yylineno);
}




/* ----------------------  Static Routines  --------------------------------- */




/* --- *** --- 
This routine adds the new data to existing data. It is also
responsible for freeing the space used by AR lines and HOST and
CHAN buffers.
--- *** --- */

static MergeData (mta, host)
register MTA    *mta;
register HOST   *host;
{
	register HOST   *h2;
	register CHAN   *c1;

	for (; host != NULL; host = h2) {
		h2 = host->hst_next;
		if ((c1 = host->hst_chan) != NULL)
			DoDirectChans (mta, c1);
		    else
			ProcessAR (mta, host->hst_name);

		if (Debug == 1) 
			printf ("MergeData - Host Free %s\n", 
			host->hst_name ? host -> hst_name : "null");

		if (host) {
			free ((char *)host);
			host = NULL;
		}

	}
}


static ProcessAR (mta, hostname)
register MTA    *mta;
register char   *hostname;
{
	register CHAN   *chan;
	char            localHost[BUFSIZ]; /* --- To SPRINTF a number into --- */

	if (hostname == NULL)	/* --- I was the AR --- */
		return;

	if (InChanList (mta->mta_arlist, hostname) == NULL) {

		chan = GetChan();

		/* --- Store the Line number with AR in case of errors --- */
		(void) sprintf (localHost, "%s %d", hostname, yylineno - 1);

		chan->ch.name = StoreStr (ARStringStore, localHost);

		if (Debug == 1) 
			printf ("ProcessAR - HostName Free %s\n", 
			hostname ? hostname : "null");

		if (hostname) {
			free (hostname);
			hostname = NULL;
		}

		mta->mta_arlist = InsertChan (mta->mta_arlist, chan);
	}
}



static MergeChanData (mta, c1)
register MTA    *mta;
register CHAN   *c1;
{
	register CHAN   *c2, *chan;

	for (; c1 != NULL; c1 = c2) {
		c2 = c1->ch_next;
		if (c1->ch.name != NULL)
			if (InChanList (mta->mta_domain, c1->ch.name) == NULL) {
				chan = GetChan();
				chan->ch.name = c1->ch.name;
				mta->mta_domain = InsertChan (mta->mta_domain,
									chan);
			}

		if (Debug == 1) 
			printf ("MergeChanData - Chan Free %s\n", 
			c1->ch.name ? c1->ch.name : "null");	

		free ((char *)c1);
		c1 = NULL;
	}
}


static DoDirectChans (mta, c1)
register MTA    *mta;
register CHAN   *c1;
{
	register CHAN   *c2, *chan;

	for (; c1 != NULL; c1 = c2) {
		c2 = c1->ch_next;
		if (c1->ch.name != NULL)
			if (InChanList (mta->mta_chan, c1->ch.name) == NULL) {
				chan = GetChan();
				chan->ch.name = c1->ch.name;
				mta->mta_chan = InsertChan (mta->mta_chan,chan);
			}

		if (Debug == 1) 
			printf ("DoDirectChans - Chan Free %s\n", 
			c1->ch.name ? c1->ch.name : "null");	

		free ((char *)c1);
		c1 = NULL;
	}
}
