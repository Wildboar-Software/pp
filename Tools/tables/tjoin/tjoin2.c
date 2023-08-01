/* tjoin2.c: */ 

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/tjoin2.c,v 6.0 1991/12/18 20:35:29 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/tjoin2.c,v 6.0 1991/12/18 20:35:29 jpo Rel $
 *
 * $Log: tjoin2.c,v $
 * Revision 6.0  1991/12/18  20:35:29  jpo
 * Release 6.0
 *
 */



#include        <stdio.h>
#include	"tjoin.h"

extern	char	*av0;
extern	int	Debug, PrintRoute, DirectFirst, ComplexOutput;
extern	MTA	*MtaTree;
extern	CHAN	*ValidChanList;
extern char	*FastInChanList();

static	MTST	*MtaStore = NULL;
static	CHST	*ChanStore = NULL;

static	char	*MailRoute();
static  void	DoDomainOutput(), PrintOutList(), PrintThing(),
		FreePrintList();
static	int	HighestCost(), CountAts();

PRINT		*DoChanOrder(), *InsertPrint(), *DeleteItem();


/* ------------------------  Begin  Routines  ------------------------------- */


WalkTree (tree, func)
register MTA	*tree;
int		(*func)();
{
	if (tree == NULL)
		return;
	if (tree -> mta_left != NULL)
		WalkTree (tree -> mta_left, func);

	(*func) (tree);

	if (tree -> mta_right != NULL)
		WalkTree (tree -> mta_right, func);
}


DebugTree (tree, indent)
register MTA	*tree;
int		indent;
{
	int	i; 

	if (tree == NULL)
		return;

	if (tree -> mta_left != NULL)
		DebugTree (tree -> mta_left, indent - 3);

	for (i=0; i < indent; i++)  printf (" ");
	printf ("%s\n", tree -> mta_name ? tree -> mta_name  : "null");
	fflush (stdout);

	if (tree -> mta_right != NULL)
		DebugTree (tree -> mta_right, indent + 3);
}



PrintNode (node)
register MTA	*node;
{
	register PRINT	*print, *list2, *p2, *p3;
	register int	cost, i;
	
	if (node == NULL)
		return;

	if (Debug == 1)
		printf ("PrintNode - %s\n", 
			node -> mta_name ? node -> mta_name : "null");

	if (node -> mta_domain != NULL) {
		DoDomainOutput (node);
		return;
	}

	if ((print = DoChanOrder (node)) == NULL)
		return;

	if (DirectFirst != 0) {
		/* Sort List2 by cost (Hop Count) */
		list2 = NULL;
		cost = HighestCost (print);
		for (i = 0; i <= cost; i++)
			for (p2 = print; p2 != NULL; p2 = p3) {
				p3 = p2 -> pr_next;
				if (p2 -> pr_cost == i) {
					print = DeleteItem (print, p2);
					list2 = InsertPrint (list2, p2);
				}
			}
		print = list2;
	}

	PrintOutList (print, node -> mta_name);
	FreePrintList (print);
}

MTA *InsertMta (tree, mta)
register MTA	*tree, *mta;
{
	register int	x;
	
	if (tree == NULL)
		return (mta);

	if (Debug == 1)
		printf ("InsertMTA - %s  %s\n",
			tree -> mta_name ? tree -> mta_name : "null", 
			mta -> mta_name ? mta -> mta_name : "null");

	if ((x = strcmp (mta -> mta_name, tree -> mta_name)) == 0) {
		/* --- Found the same node - should not happen --- */
		fprintf (stderr, "%s: InsertMta - Fatal Error\n", av0);
		exit (1);
	}
	if (x < 0)
		tree -> mta_left = InsertMta (tree -> mta_left, mta);
	else
		tree -> mta_right = InsertMta (tree -> mta_right, mta);
	return (tree);
}



MTA *NameToMta (name)
char	*name;
{
	register MTA	*mta;
	register int	x;

	if (Debug == 1)
		printf ("NameToMTA - %s\n", name ? name : "null");
	
	mta = MtaTree;

	while (mta != NULL) {
		if ((x = strcmp (name, mta -> mta_name)) == 0)
			break;
		if (x < 0)
			mta = mta -> mta_left;
		else
			mta = mta -> mta_right;
	}
	return (mta);
}


CHAN *GetChan()
{
	register CHST	*chst;
	register CHAN	*chan;
	
	if (ChanStore == NULL) {			
		/* Starting up */
		if ((ChanStore = (CHST *) malloc (sizeof (CHST))) == NULL) {
			fprintf (stderr, "%s: GetChan - No Memory [1]\n", av0);
			exit (1);
		}

		if (Debug == 1) 
			printf ("GetChan Start - Malloc %x\n", ChanStore);

		bzero ((char *)ChanStore, sizeof(*ChanStore));
	}


	if (ChanStore -> chs_index >= CHANSTORESIZE) {			
		/* Get another bunch */
		if ((chst = (CHST *) malloc (sizeof (CHST))) == NULL) {
			fprintf (stderr, "%s: GetChan - No Memory [2]\n", av0);
			exit (1);
		}

		if (Debug == 1) 
			printf ("GetChan GetBuff - Malloc %x\n", chst);

		bzero ((char *)chst, sizeof(*chst));
		chst -> chs_next = ChanStore;
		ChanStore = chst;
	}

	chan = &ChanStore -> chs_buffer [ChanStore -> chs_index++];
	chan -> ch_next = NULL;
	chan -> ch.name = NULL;
	return (chan);
}


SBUFF *InitStringStore()
{
	register SBUFF	*str;
	
	if ((str = (SBUFF *) malloc (sizeof (SBUFF))) == NULL) {
		fprintf (stderr, "%s: InitStringStore - No Memory\n", av0);
		exit (1);
	}

	if (Debug == 1)
		printf ("InitStringStore - Malloc %x\n", str);

	bzero ((char *)str, sizeof(*str));
	return (str);
}



char *StoreStr (store, string)
register SBUFF	*store;
char		*string;
{
	register int	length;
	char		*ptr;

	if (Debug == 1)
		printf ("StoreStr - %s\n", string ? string : "null");

	if (string == NULL || *string == '\0')
		return ((char *)0);
	
	length = strlen (string) + 1;

	if (length > STRINGSTORESIZE) {
		/* This is FATAL */
		fprintf (stderr, "%s: StoreStr - Can't store %s - Ignored\n",
			 av0, string);
		exit (1);
	}

	while (length > STRINGSTORESIZE - store -> str_index) {
		if (store -> str_next != NULL) {
			store = store -> str_next;
			continue;
		}

		store -> str_next = (SBUFF *) malloc (sizeof (SBUFF));

		if (store -> str_next == NULL) {
			fprintf (stderr, "%s: StoreStr - No Memory\n", av0);
			exit (1);
		}

		if (Debug == 1) 
			printf ("Malloc %x - StoreStr\n", store -> str_next);

		store = store -> str_next;
		bzero ((char *)store, sizeof(*store));
		break;
	}

	ptr = &store -> str_buffer [store -> str_index];
	(void) strcpy (ptr, string);
	store -> str_index += length;
	return (ptr);
}



MTA *GetMta()
{
	register MTST	*mtst;
	register MTA	*mta;
	
	if (MtaStore == NULL) {			
		/* Starting up */
		if ((MtaStore = (MTST *) malloc (sizeof (MTST))) == NULL) {
			fprintf (stderr, "%s: GetMta - No Memory [1]\n", av0);
			exit (1);
		}

		if (Debug == 1) 
			printf ("GetMta Start - Malloc %x \n", MtaStore);

		bzero ((char *)MtaStore, sizeof(*MtaStore));
	}

	if (MtaStore -> mts_index >= MTASTORESIZE) {			
		/* Get another bunch */
		if ((mtst = (MTST *) malloc (sizeof (MTST))) == NULL) {
			fprintf (stderr, "%s: GetMta - No Memory [2]\n", av0);
			exit (1);
		}
		if (Debug == 1) 
			printf ("GetMta GetBuffer - Malloc %x \n", mtst);

		bzero ((char *)mtst, sizeof(*mtst));
		mtst -> mts_next = MtaStore;
		MtaStore = mtst;
	}

	mta = &MtaStore -> mts_buffer [MtaStore -> mts_index++];
	mta -> mta_left = NULL;
	mta -> mta_right = NULL;
	mta -> mta_chan = NULL;
	mta -> mta_arlist = NULL;
	mta -> mta_domain = NULL;
	mta -> mta_name = NULL;
	return (mta);
}



FreeStringStore (str)
register SBUFF	*str;
{
	register SBUFF	*startstr = str,
			*s2;

	if (Debug == 1) 
		printf ("FreeStringStore - %x\n", str);
	
	for (; str != NULL; str = s2) {
		s2 = str -> str_next;
		if (str)	free ((char *)str);
	}

	startstr = NULL;
}



/* ------------------------  Static Routines  ------------------------------- */





static void DoDomainOutput (node)
register MTA	*node;
{
	register CHAN	*chan;
	register int	flag = 0;
	
	if (Debug == 1)
		printf ("DoDomainOutput - %s\n", 
			node -> mta_name ? node -> mta_name : "null");

	printf ("%s:", node -> mta_name);

	if (ComplexOutput != 0)
		printf ("(");

	for (chan = ValidChanList; chan != NULL; chan = chan -> ch_next)
		if (FastInChanList (node->mta_domain, chan->ch.name) != NULL) {
			if (flag == 1)
				printf (",");
			if (ComplexOutput == 0)
				printf ("(%s)", chan -> ch.name);
			else
				printf ("%s", chan -> ch.name);
			flag = 1;
		}

	if (ComplexOutput != 0)
		printf (")");

	printf ("\n");
}


static int HighestCost (print)
register PRINT	*print;
{
	register int	cost;

	cost = 0;
	for (; print != NULL; print = print -> pr_next)
		if (print -> pr_cost > cost)
			cost = print -> pr_cost;
	return (cost);
}


static void PrintOutList (print, mtaname)
register PRINT	*print;
register char	*mtaname;
{
	register char	*last;

	if (Debug == 1)
		printf ("PrintOutList - %s\n", mtaname ? mtaname : "null");
	
	printf ("%s:", mtaname);

	last = "";

	for (; print != NULL; print = print -> pr_next) {

		if (PrintRoute == 0) {
			PrintThing (print -> pr_nexthop, last);
			if (ComplexOutput != 0)
				last = print -> pr_nexthop;
		}
		else {
			PrintThing (&print -> pr_relay [0], last);
			if (ComplexOutput != 0)
				last = &print -> pr_relay [0];
		}

		printf ("%s", print -> pr_chan);

		if (print -> pr_next == NULL)
			if (ComplexOutput == 0)
				printf (")\n");
			else
				printf (")\n");
		else
			if (ComplexOutput == 0)
				printf ("),");
	}
}



static void PrintThing (host, last)
register char	*host, *last;
{
	if (Debug == 1)
		printf ("PrintThing - %s\n", host ? host : "null");

	if (ComplexOutput == 0)
		printf ("%s(", host);
	else
		if (*last == '\00')
			printf ("%s(", host);
		else
			if (strcmp (last, host) != 0)
				printf ("),%s(", host);
			else
				printf (",");
}


static void FreePrintList (print)
register PRINT	*print;
{
	register PRINT	*startprint = print,
			*p2;
	
	for (; print != NULL; print = p2) {
		p2 = print -> pr_next;
		if (print)	free ((char *)print);
	}

	startprint = NULL;
}


static PRINT *DoChanOrder (node)
register MTA	*node;
{
	register CHAN	*chan;
	char		*ptr, buff [LOADS];
	PRINT		*print, *p2;
	int		space;
	if (Debug == 1)
		printf ("DoChanOrder - %s\n", 
			node -> mta_name ? node -> mta_name : "null");

	print = NULL;

	for (chan = ValidChanList; chan != NULL; chan = chan -> ch_next) {
		space = sizeof(buff);
		if ((ptr = MailRoute (node, chan, &buff [0], &space)) != NULL) {
			if ((p2 = (PRINT *) malloc (sizeof (PRINT))) == NULL) {
				fprintf (stderr, "%s: DoChanOrder - No ", av0);
				fprintf (stderr, "memory\n");
				exit (1);
			}

			bzero ((char *)p2, sizeof(*p2));
			(void) strcpy (&p2 -> pr_relay [0], &buff [1]);
			p2 -> pr_nexthop = ptr;
			p2 -> pr_chan = chan -> ch.name;
			p2 -> pr_cost = CountAts (&buff [1]);
			print = InsertPrint (print, p2);
		}
	}
	return (print);
}


static int CountAts (str)
register char	*str;
{
	register int	count;
	
	if (str == NULL)
		return (0);
	count = 0;
	for (; *str != '\00'; str++)
		if (*str == '@')
			count++;
	return (count);
}



static PRINT *DeleteItem (list, item)
register PRINT	*list, *item;
{
	register PRINT	*p1;
	
	if (Debug == 1)	printf ("DeleteItem\n");

	if (list == NULL) {
		fprintf (stderr, "%s: DeleteItem - List is NULL\n", av0);
		return (NULL);
	}

	if (list == item) {
		p1 = list -> pr_next;
		list -> pr_next = NULL;
		return (p1);
	}

	for (p1 = list; p1 -> pr_next != NULL; p1 = p1 -> pr_next)
		if (item == p1 -> pr_next) {
			p1 -> pr_next = item -> pr_next;
			item -> pr_next = NULL;
			return (list);
		}

	fprintf (stderr, "%s: DeleteItem - Item not in List\n", av0);
	return (list);
}


static PRINT *InsertPrint (list, item)
register PRINT	*list, *item;
{
	register PRINT	*p1;
	
	if (Debug == 1)	printf ("InsertPrint\n");

	if (list == NULL)
		return (item);
	for (p1 = list; p1 -> pr_next != NULL; p1 = p1 -> pr_next)
		continue;
	p1 -> pr_next = item;
	return (list);
}



static char *MailRoute (mta, chan, str, space)
register MTA	*mta;
register CHAN	*chan;
register char	*str;
int		*space;
{
	register CHAN	*c1;
	register MTA	*m1;
	register char	*ptr;
	
	if (Debug == 1)	printf ("MailRoute\n");

	if ((*space -= ((int)strlen(mta -> mta_name) + 1)) < 1) {
		fprintf(stderr, 
			"Possible AR loop detected containing mta '%s' (buffer overrun)\n",
			mta->mta_name);
		return (NULL);
	}

	(void) sprintf (str, "@%s", mta -> mta_name);

	if (FastInChanList (mta -> mta_chan, chan -> ch.name) != NULL)
		return (mta -> mta_name);

	for (c1 = mta -> mta_arlist; c1 != NULL; c1 = c1 -> ch_next) {
		if ((m1 = c1 -> ch.mta) == NULL)
			continue;
		if ((ptr = MailRoute (m1, chan, &str [strlen (str)], space)) != NULL)
			return (ptr);
	}
	return (NULL);
}


