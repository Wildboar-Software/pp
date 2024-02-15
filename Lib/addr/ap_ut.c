/* ap_ut.c: address parser utility routines. */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_ut.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_ut.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_ut.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*
Standard routines for handling address list element nodes
*/


/*
< 1978  B. Borden       Wrote initial version of parser code
78-80   D. Crocker      Reworked parser into current form
Apr 81  K. Harrenstein  Hacked for SRI
Jun 81  D. Crocker      Back in the fold.  Finished v7 conversion
			minor cleanups.
			repackaging into more complete set of calls
Jul 81  D. Crocker      ap_free & _alloc check for not 0 or -1
			malloc() error causes jump to ap_init error
*/



#include "util.h"
#include "ap.h"


#ifdef AP_DEBUG
extern char ap_debug;
extern char *typtab[];
#endif


int                             (*ap_gfunc)(); /* -- ptr to char get fn -- */
extern int                             ap_peek; /* -- basic parse state info -- */
extern int                      ap_perlev;
extern int                      ap_grplev;
AP_ptr                          ap_pstrt,
				ap_pcur;


/* ---------------------  Begin  Routines  -------------------------------- */



AP_ptr ap_alloc()  /* -- create node, return pointer to it -- */
{
	AP_ptr  ap;

	ap = (AP_ptr) malloc (sizeof (struct ap_node));

	if (ap == BADAP)
		return (BADAP);

	ap_ninit (ap);
	return (ap);
}



void ap_ninit (ap)
register AP_ptr         ap;
{
	bzero((char *)ap, sizeof(*ap));
	ap -> ap_obtype         = AP_NIL;
	ap -> ap_ptrtype        = AP_PTR_NIL;
}



void ap_free (ap)
register AP_ptr         ap;
{
	register char   *obvalue;

	/* -- get rid of node, if have one -- */
	switch ((int)ap) {
	case OK:
	case NOTOK:
		/* -- nothing to free -- */
		break;

	default:
		if (ap->ap_normalised == TRUE) {
			if (ap->ap_localhub)
				free(ap->ap_localhub);
			if (ap->ap_chankey)
				free(ap->ap_chankey);
			if (ap->ap_error)
				free(ap->ap_error);
		}
		/* -- get rid of its data string -- */
		switch ((int)(obvalue = ap -> ap_obvalue)) {
		case OK:
		case NOTOK:
			/* -- nothing to free -- */
			break;

		default:
			free (obvalue);
		}

		free ((char *) ap);
	}
}



/* -- add data to node at end of chain -- */
void ap_fllnode (ap, obtype, obvalue)
register AP_ptr         ap;
char                    obtype;
register char           *obvalue;
{

	ap -> ap_obtype = obtype;
	ap -> ap_obvalue = (obvalue == NULLCP) ? NULLCP : strdup (obvalue);

#ifdef AP_DEBUG
	if (ap_debug)
		PP_DBG (("(%s/'%s')", typtab[obtype], obvalue));
#endif
}



AP_ptr ap_new (obtype, obvalue)  /* -- alloc & fill node -- */
char                    obtype;
char                    *obvalue;
{
	register AP_ptr nap;

	nap = ap_alloc();
	ap_fllnode (nap, obtype, obvalue);
	return (nap);
}



void ap_insert (cur, ptrtype, new)  /* -- create, fill or insert node in list -- */
register AP_ptr         cur;            /* -- where to insert after -- */
char                    ptrtype;        /* -- more or new address ? -- */
register AP_ptr         new;            /* -- where to insert after -- */
{

	/* -- Now copy linkages from current node -- */

	new -> ap_ptrtype = cur -> ap_ptrtype;
	new -> ap_next = cur -> ap_next;

	/* -- now point current node at inserted node -- */

	cur -> ap_ptrtype = ptrtype;
	cur -> ap_next = new;
}



AP_ptr ap_sqinsert (cur, type, new)  /* -- insert in sequence -- */
register AP_ptr         cur;
int                     type;
register AP_ptr         new;
{
	AP_ptr          oldptr;
	int             otype;

	switch ((int)new) {
	case OK:
	case NOTOK:
		return (NULLAP);
	}

	oldptr = cur -> ap_next;
	otype = cur -> ap_ptrtype;
	cur -> ap_next = new;
	cur -> ap_ptrtype = type;

	while (new -> ap_ptrtype != AP_PTR_NIL &&
	       new -> ap_next != NULLAP &&
	       new -> ap_next -> ap_obtype != AP_NIL)
			new = new -> ap_next;

	if (new -> ap_next && new -> ap_next -> ap_obtype == AP_NIL)
		ap_delete (new);

	new -> ap_next = oldptr;
	new -> ap_ptrtype = otype;
	return (new);
}



void ap_delete (ap)  /* -- remove next node in sequence -- */
register AP_ptr         ap;
{
	register AP_ptr next;

	if (ap != NULLAP && ap -> ap_ptrtype != AP_PTR_NIL 
	    && ap -> ap_next != NULLAP) {
		/* -- only if there is something there -- */
		/* -- link around one to be removed -- */
		next = ap -> ap_next;
		ap -> ap_ptrtype = next -> ap_ptrtype;
		ap -> ap_next = next -> ap_next;
		ap_free (next);
	}
}



AP_ptr ap_append (ap, obtype, obvalue)  /* -- alloc, fill or insert node -- */
register AP_ptr         ap;     /* -- node to insert after -- */
char                    obtype;
char                    *obvalue;
{
	register AP_ptr nap;

	nap = ap_alloc();
	ap_fllnode (nap, obtype, obvalue);
	ap_insert (ap, AP_PTR_MORE, nap);
	return (nap);
}



AP_ptr ap_add (ap, obtype, obvalue)  /* -- append data to current node -- */
register AP_ptr         ap;
char                    obtype;
register char           *obvalue;
{
	register char       *ovalue;

	if (ap -> ap_obtype != obtype)
		return (ap_append (ap, obtype, obvalue));
	else {
		/* -- same type or empty => can append -- */

		/* -- no data to add -- */
		if (obvalue == NULLCP)
			return (OK);

		if ((ovalue = ap -> ap_obvalue) == NULLCP)
			ap_fllnode (ap, obtype, obvalue);
		else {
			/* -- add to existing data -- */
			ovalue = ap -> ap_obvalue;
			ap -> ap_obvalue = multcat (ovalue, " ",
						    obvalue, NULLCP);
			free (ovalue);
		}

#ifdef AP_DEBUG
		if (ap_debug)
			PP_DBG (("(%d/'%s')", obtype, obvalue));
#endif
	}

	return (OK);
}



AP_ptr ap_sqdelete (strt_node, end_node)
register AP_ptr         strt_node;
register AP_ptr         end_node;
{
	/* -- remove nodes, through end node -- */

	switch ((int)strt_node) {
	case OK:
	case NOTOK:
		return (NULLAP);
	}

	while (strt_node -> ap_ptrtype != AP_PTR_NIL) {
		if (strt_node -> ap_next == end_node) {
			/* -- last one requested -- */
			ap_delete (strt_node);
			return (strt_node -> ap_next);
		}

		ap_delete (strt_node);
	}

	/* -- end of chain -- */
	return (NULLAP);
}



AP_ptr ap_1delete (ap)  /* -- remove all nodes of address to NXT -- */
register AP_ptr         ap;     /* -- starting node -- */
{
	while (ap -> ap_ptrtype != AP_PTR_NIL) {
		if (ap -> ap_ptrtype == AP_PTR_NXT)
			return (ap -> ap_next);

		ap_delete (ap);
	}

	/* -- end of chain -- */
	return (NULLAP);
}



void ap_sqtfix (strt, end, obtype)  /* -- alter obtype of a node subsequence -- */
register AP_ptr         strt;
register AP_ptr         end;
register char           obtype;
{
	for ( ; ; strt = strt -> ap_next) {
		if (strt -> ap_obtype != AP_COMMENT)
			strt -> ap_obtype = obtype;

		if (strt == end || strt -> ap_ptrtype == AP_PTR_NIL)
			break;
	}
}



AP_ptr ap_move (to, from)  /* -- move node after from to be after to -- */
register AP_ptr         to,
			from;
{
    register AP_ptr     nodeptr;

	/* -- quiet failure -- */
	if (from -> ap_ptrtype == AP_PTR_NIL || from -> ap_next == NULLAP)
		return (from);

	nodeptr = from -> ap_next;

	from -> ap_next = nodeptr -> ap_next;
	from -> ap_ptrtype = nodeptr -> ap_ptrtype;

	ap_insert (to, AP_PTR_MORE, nodeptr);

	/* -- next in chain, now -- */
	return (from);
}



AP_ptr ap_sqmove (to, from, endtype)  /* -- move sequence -- */
register AP_ptr         to,
			from;
register char           endtype;  /* -- copy only COMMENT and this -- */
{
	switch ((int)from) {
	case OK:
	case NOTOK:
		return (NULLAP);
	}

	while (from -> ap_ptrtype != AP_PTR_NIL &&
	       from -> ap_next != NULLAP) {

		if (endtype != (char) AP_NIL)
			if (from -> ap_obtype != AP_COMMENT &&
			    from -> ap_obtype != endtype)
				break;
		to = ap_move (to, from);
	}

	/* -- end of chain -- */
	return (to);
}



void ap_iinit (gfunc)  /* -- input function initialization -- */
int     (*gfunc)();
{
	ap_gfunc = gfunc;             /* -- set character fetch func -- */
	ap_peek = -1;                 /* -- no lex peek char -- */
}



void ap_clear()  /* -- clear out the parser state -- */
{
	ap_grplev = 0;      /* -- zero group nesting depth -- */
	ap_perlev = 0;      /* -- zero <> nesting depth -- */
}



AP_ptr ap_pinit (gfunc)  /* -- init, alloc & set start node -- */
int     (*gfunc)();
{
	ap_iinit (gfunc);
	return (ap_pstrt = ap_pcur = ap_alloc());
}






/*
These echo the basic list manipuation primitives, but use ap_pcur
for the pointer and any insert will cause ap_pcur to be updated
to point to the new node.
*/


void ap_palloc()  /* -- alloc, insert after pcur -- */
{
	ap_pnsrt (ap_alloc(), AP_PTR_MORE);
}



void ap_pfill (obtype, obvalue)  /* -- add data to node at end of chain -- */
char            obtype;
register char   *obvalue;
{
	ap_pcur -> ap_obtype = obtype;
	ap_pcur -> ap_obvalue =
			(obvalue == NULLCP) ? NULLCP : strdup (obvalue);

#ifdef AP_DEBUG
	if (ap_debug)
		PP_DBG (("(%s/'%s')", typtab[obtype], obvalue));
#endif
}



void ap_pnsrt (ap, ptrtype)  /* -- add node to end of parse chain -- */
register AP_ptr         ap;
char                    ptrtype;
{
    register AP_ptr     rap_pcur;

	if ((rap_pcur = ap_pcur) -> ap_obtype == AP_NIL) {
		/* -- current one can be used -- */
		rap_pcur -> ap_obtype = ap -> ap_obtype;
		rap_pcur -> ap_obvalue = ap -> ap_obvalue;
		ap -> ap_obvalue = NULLCP;
		ap_free (ap);
	}
	else {
		/* -- really do the insert -- */
		rap_pcur -> ap_ptrtype = ptrtype;
		rap_pcur -> ap_next = ap;
		ap_pcur = ap;
	}
}



void ap_pappend (obtype, obvalue)  /* -- alloc, fill, append at end -- */
char    obtype;
char   *obvalue;
{
	ap_palloc();    /* -- will update pcur -- */
	ap_fllnode (ap_pcur, obtype, obvalue);
}



void ap_padd (obtype, obvalue)  /* -- try to append data to current node -- */
char                    obtype;
char                    *obvalue;
{
	register AP_ptr     nap;

	nap = ap_add (ap_pcur, obtype, obvalue);

	if (nap != OK)                /* -- created new node -- */
		ap_pcur = nap;
}
