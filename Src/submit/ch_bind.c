/* ch_bind.c - binds reformatting channels to a recipient address */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Src/submit/RCS/ch_bind.c,v 6.0 1991/12/18 20:28:02 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Src/submit/RCS/ch_bind.c,v 6.0 1991/12/18 20:28:02 jpo Rel $
 *
 * $Log: ch_bind.c,v $
 * Revision 6.0  1991/12/18  20:28:02  jpo
 * Release 6.0
 *
 */



#include "head.h"
#include "q.h"
#include "ch_bind.h"


extern void		err_abrt();
extern LIST_BPT		*bodies_all, *headers_all;


/* -- static variables -- */
static FILTER		*filterlist = NULL;
static TYPES		*typelist;
static MAT		*mainmatrix;
static int		matsize;
static FLAT		*flattenlist = NULL, *unflatlist = NULL;


/* -- local routines -- */
int			ch_bind();
void			fmt_init();

static FILTER		*getfilter();
static FILTER		*name2filter();
static FLAT		*add2flatlist();
static FLAT		*flttype2ptr();
static LIST		*additem();
static LIST		*copylist();
static LIST		*getitem();
static LIST		*inslist();
static MAT		*initmat();
static TYPES		*addchantypes();
static TYPES		*name2type();
static int		isinlist();
static void		addmat();
static void		do_calc();
static void		flatten();
static void		format();
static void		freelist();
static void		get_formatters();
static void		initform();
static void		unflat();




/* ---------------------  Begin  Routines --------------------------------- */


/* --- *** Start Description *** ---
   
   ch_bind (ad, p, pfmts, pcost, punknown)
   ADDR            *ad;
   LIST_RCHAN	*p;
   LIST_RCHAN	**pfmts;
   int		*pcost, *punknown;
   
   Parameters required:
   - The channel calling submit
   - The flatness of the channel
   - The channel/mta pair used to get to the Next Hop hostname ( p )
   - The list of Encoded Information Types within the message
   (Qstruct.encodedinfo.eit_types)
   - A pointer to a list of Reformatting channels (ad->ad_fmtchan)

   Example:
   ch_inbound           =  x400in
   content_in		=  "rfc" or "p2"
   p		     =  li_mta = ukc.ac.uk     li_chan => JANET
   ( or li_mta = stl.stc.co.uk li_chan => PSS )
   Qstruct.encodedinfo
   .eit_types    =  ia5Text, voice
   ad->ad_fmtchan       =  unflattenp2, voice2text, flattenrfc
   
   Routine:
   - Calculates the reformatting channels.
   - Updates ad_fmtchan.
   - Returns OK or NOTOK.
   
   
   fmt_init ()
   - Initialise reformatting stuff
   
   --- *** End Description *** --- */




/* ------------------------------------------------------------------------ */





void fmt_init ()
{
	PP_TRACE (("ch_bind/fmt_init ()"));
	
	initform ();
	get_formatters ();
	do_calc (mainmatrix);
}

static FLAT *add2flatlist (list, name, type)
FLAT    *list;
char    *name, *type;
{
	FLAT    *ptr;
	
	ptr = (FLAT *) smalloc (sizeof (FLAT));
	ptr->flt_type = strdup (type);
	ptr->flt_name = strdup ( name);
	ptr->flt_next = list;
	return (ptr);
}

static void unflat (name, type)	/* Add an unflattening channel to the list */
char    *name, *type;           /* of unflattening channels */
{
	PP_TRACE (("ch_bind/unflat (%s, %s)", name, type));
	
	if (flttype2ptr (unflatlist, type) != NULL)
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("ch_bind/UNFLAT type %s is in use already", type));
		return;
	}
	unflatlist = add2flatlist (unflatlist, name, type);
}

static void flatten (name, type) /* Add a Flattening channel to the list */
char    *name, *type;           /* of flattening channels */
{
	PP_TRACE (("ch_bind/flatten (%s, %s)", name, type));
	
	if (flttype2ptr (flattenlist, type) != NULL)
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("ch_bind/FLATTEN type %s is in use already\n",
			 type));
		return;
	}
	flattenlist = add2flatlist (flattenlist, name, type);
}

static FLAT *flttype2ptr (list, type)
char    *type;
FLAT    *list;
{
	PP_DBG (("ch_bind/flttype2ptr (list, %s)", type));
	
	if (type == NULLCP)
		return NULL;
	for (; list != NULL; list = list->flt_next)
		if (strcmp (list->flt_type, type) == 0)
			return (list);
	PP_TRACE (("ch_bind/flttype2ptr - NO match"));
	return (NULL);
}

static FILTER *name2filter (name) /* Format Channel name to pointer to struct */
char    *name;
{
	FILTER  *ptr;
	
	PP_TRACE (("ch_bind/name2filter (%s)", name));
	
	for (ptr = filterlist; ptr != NULL; ptr = ptr->fil_next)
		if (strcmp (name, ptr->fil_name) == 0)
			return (ptr);
	return (NULL);
}


static LIST *inslist (list, new)
LIST    *list, *new;
{
	
	if (new == NULL)
	{
		PP_TRACE (("ch_bind/inslist (NULL)"));
		return (list);
	}
	
	PP_TRACE (("ch_bind/inslist (%s)", new -> li_name ));
	list = inslist (list, new->li_next);
	/* SEK - should replace with for loop */
	if (!isinlist (list, new))
	{
		new->li_next = list;
		list = new;
	}
	return (list);
}

static int isinlist (list, item)
LIST    *list, *item;
{
	
	for (; list != NULL; list = list->li_next)
		if (strcmp (list->li_name, item->li_name) == 0)
			return (1);
	return (0);
}

static TYPES *addchantypes (atypelist, punknown)
LIST_BPT        *atypelist;
int		*punknown;
{
	register TYPES  *list, *all, *storenum;
	
	PP_TRACE (("ch_bind/addchantypes (atypelist)"));
	*punknown = FALSE;
	list = NULL;
	for (; atypelist != NULL; atypelist = atypelist->li_next)
	{
		if ((storenum = name2type (atypelist->li_name)) == NULL)
		{
			PP_LOG (LLOG_EXCEPTIONS,
				("ch_bind/%s is not a defined type",
				 atypelist->li_name));
			*punknown = TRUE;
			continue;
		}
		all = (TYPES *) smalloc (sizeof (TYPES));
		all->ty_next = list;
		list = all;
		all->ty_name = atypelist->li_name;
		all->ty_number = storenum->ty_number;
	}
	return (list);
}


static TYPES *name2type (str)
char    *str;
{
	TYPES   *ptr;
	
	PP_TRACE (("ch_bind/name2type (%s)", str));
	for (ptr = typelist; ptr != NULL; ptr = ptr->ty_next)
		if (strcmp (str, ptr->ty_name) == 0)
			return (ptr);
	return (NULL);
}

static void format (fname, fcost, ffrom, fto)
int     fcost;
char    *fname, *ffrom, *fto;   /* Don't use FNAME, pass the CHAN pointer */
{
	int     x, y;
	FILTER  *ptr;
	TYPES   *type;
	
	PP_TRACE (("ch_bind/format (%s, %d, %s, %s)",
		   fname, fcost, ffrom, fto));
	
	if ((type = name2type (ffrom)) == NULL)
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("ch_bind/%s is not a defined type", ffrom));
		return;
	}
	x = type->ty_number;
	if ((type = name2type (fto)) == NULL)
	{
		PP_LOG (LLOG_EXCEPTIONS,
			("ch_bind/%s is not a defined type", fto));
		return;
	}
	y = type->ty_number;
	addmat (mainmatrix, x, y, fcost, fname);
	ptr = getfilter ();
	ptr->fil_next = filterlist;
	filterlist = ptr;
	ptr->fil_name = fname;
	ptr->fil_from = ffrom;
	ptr->fil_to = fto;
}

static LIST *copylist (list)
register LIST   *list;
{
	register LIST   *tmpitem, *ptr;
	
	PP_DBG (("ch_bind/copylist (list)"));
	
	for (tmpitem = NULL; list != NULL; list = list->li_next)
	{
		ptr = getitem ();
		ptr->li_name = list->li_name;
		tmpitem = additem (tmpitem, ptr);
	}
	return (tmpitem);
}


static LIST *additem (list, item)   /* Adds LIST item onto end of LIST list */
register LIST   *list;
LIST            *item;
{
	PP_DBG (("ch_bind/additem (list, item)"));
	
	if (list == NULL)
		return (item);
	list->li_next = additem (list->li_next, item);
	return (list);
}

static void freelist (list)
register LIST   *list;
{
	PP_TRACE (("ch_bind/freelist (list)"));
	
	if (list == NULL)
		return;
	freelist (list->li_next);
	free ((char *)list);
}

static void do_calc (mat)
MAT     mat [];
{
	register MAT    *ptr1, *ptr2, *ptr3;
	register int    i, j, k;
	
	PP_TRACE (("ch_bind/do_calc (mat)"));
	for (i = 0; i < matsize; i++)
		for (j = 0; j < matsize; j++)
		{
			ptr3 = &mat [j * matsize + i];
			if (ptr3->m_int != 0)
				for (k = 0; k < matsize; k++)
				{
					ptr2 = &mat [i * matsize + k];
					if (ptr2->m_int != 0)
					{
						ptr1 = &mat [j * matsize + k];
						if (ptr1->m_int == 0)
						{
							ptr1->m_int = ptr2->m_int + ptr3->m_int;
							ptr1->m_list = additem (ptr1->m_list, copylist (ptr3->m_list));
							ptr1->m_list = additem (ptr1->m_list, copylist (ptr2->m_list));
						}
						else
						{
							if (ptr2->m_int + ptr3->m_int < ptr1->m_int)
							{
								freelist (ptr1->m_list);
								ptr1->m_list = copylist (ptr3->m_list);
								ptr1->m_list = additem (ptr1->m_list, copylist (ptr2->m_list));
								ptr1->m_int = ptr2->m_int + ptr3->m_int;
							}
						}
					}
				}
		}
}

static MAT *initmat (size)
int     size;
{
	MAT     *mat;
	int     i, j;
	
	PP_TRACE (("ch_bind/initmat (%d)", size));
	matsize = size;
	mat = (MAT *) smalloc (size * size * sizeof (MAT));
	
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
		{
			mat [i * matsize + j].m_int = (i == j) ? 1 : 0;
			mat [i * matsize + j].m_list = NULL;
		}
	return (mat);
}

static void addmat (mat, x, y, cost, name)
MAT     mat [];
int     x, y, cost;
char    *name;                          /* Should be Pointer to CHAN struct */
{
	register MAT    *tmp;
	register LIST   *ptr;
	
	PP_TRACE (("ch_bind/addmat (mat, %d, %d, %d, %s)", x, y, cost, name));
	ptr = getitem ();
	ptr->li_name = name;
	tmp = &mat [x * matsize + y];
	if (tmp -> m_list == NULL) {
		tmp->m_list = ptr;
		tmp->m_int = cost;
	}
	else {
		PP_TRACE(("Duplicate reformatter %s (clashes with %s)",name,
			tmp -> m_list -> li_name));
		if (cost < tmp -> m_int) {
			free ((char *)tmp->m_list);
			tmp -> m_list = ptr;
			tmp -> m_int = cost;
		}
		else
			free ((char *)ptr);
	}
}

static FILTER *getfilter ()
{
	register FILTER *ptr;
	
	PP_DBG (("ch_bind/getfilter ()"));
	ptr = (FILTER *) smalloc (sizeof (FILTER));
	
	ptr->fil_next = NULL;
	return (ptr);
}

static LIST *getitem ()
{
	register LIST   *ptr;
	
	PP_DBG (("ch_bind/getitem ()"));
	ptr = (LIST *) smalloc (sizeof (LIST));
	ptr->li_name = NULL;
	ptr->li_next = NULL;
	return (ptr);
}

/*              initform
 *      This routine goes through the body parts
 */


static void initform ()
{
	register LIST_BPT       *bptptr;
	TYPES                   *all, *list;
	int                     count;
	
	PP_TRACE (("ch_bind/initform ()"));
	
	list = NULL;
	count = 0;
	for (bptptr = headers_all; bptptr != NULLIST_BPT;
	     bptptr = bptptr -> li_next)
	{
		PP_TRACE (("ch_bind/doing body part %s",
			   bptptr -> li_name));
		all = (TYPES *) smalloc (sizeof (TYPES));
		all->ty_next = list;
		list = all;
		all->ty_name = bptptr->li_name;
		all->ty_number = count++;
	}
	for (bptptr = bodies_all; bptptr != NULLIST_BPT;
	     bptptr = bptptr -> li_next)
	{
		PP_TRACE (("ch_bind/doing body part %s",
			   bptptr -> li_name));
		all = (TYPES *) smalloc (sizeof (TYPES));
		all->ty_next = list;
		list = all;
		all->ty_name = bptptr->li_name;
		all->ty_number = count++;
	}
	mainmatrix = initmat (count);
	typelist = list;
}

/*              get_formatters
 * This routine will go through the list of Channels and put the
 * formatters, flatteners and un-flatteners into the matrix.
 */

static void get_formatters ()
{
	register CHAN           **chanptr;
	PP_TRACE (("ch_bind/get_formatters"));
	for (chanptr = ch_all; (*chanptr) != NULL; chanptr++)
	{
		if ((*chanptr)->ch_chan_type != CH_SHAPER)
			continue;
		if ((*chanptr)->ch_bpt_in == NULL 
		    && (*chanptr)->ch_bpt_out == NULL
		    && (*chanptr)->ch_hdr_out == NULL
		    && (*chanptr)->ch_hdr_in == NULL) {
			/* Flatten/Unfflatten */
			if ((*chanptr)->ch_content_out == NULL
			    && (*chanptr)->ch_content_in == NULL)
			{
				PP_LOG (LLOG_EXCEPTIONS,
					("ch_bind/no content definitions for %s",
					 (*chanptr)-> ch_name));
			}
			else if ((*chanptr)->ch_content_in != NULL)
			{       /* This is Unflattener */
				unflat ((*chanptr)->ch_name,
					(*chanptr)->ch_content_in);
			}
		        else
			{       /* This is a flattener */
				flatten ((*chanptr)->ch_name,
					 (*chanptr)->ch_content_out);
			}
		}
		
		else if ((*chanptr)->ch_bpt_in != NULL 
			 && (*chanptr)->ch_bpt_out != NULL)
			
		{       /* This is Formatter */
			LIST_BPT       	*ix = (*chanptr)->ch_bpt_in;
			for (;ix != NULLIST_BPT; ix = ix -> li_next)
				format ((*chanptr)->ch_name,
					(*chanptr)->ch_cost + 2,
					ix -> li_name,
					(*chanptr)->ch_bpt_out->li_name);
			if ((*chanptr)->ch_bpt_out->li_next != NULLIST_BPT) 
				PP_LOG(LLOG_EXCEPTIONS,
				       ("ch_bind mulitple bptouts specified for shaper channel '%s'",
					(*chanptr)->ch_name));
			
		}
		
		else if ((*chanptr)->ch_hdr_in != NULL 
			 && (*chanptr)->ch_hdr_out != NULL)
			
		{       /* This is Formatter */
			LIST_BPT       	*ix = (*chanptr)->ch_hdr_in;
			for (;ix != NULLIST_BPT; ix = ix -> li_next)
				format ((*chanptr)->ch_name,
					(*chanptr)->ch_cost + 2,
					ix->li_name,
					(*chanptr)->ch_hdr_out->li_name);
			if ((*chanptr)->ch_hdr_out->li_next != NULLIST_BPT) 
				PP_LOG(LLOG_EXCEPTIONS,
				       ("ch_bind mulitple hdrouts specified for shaper channel '%s'",
					(*chanptr)->ch_name));
		}
		
		else
		{
			PP_LOG (LLOG_EXCEPTIONS,
				("ch_bind/ erroneous definitions for %s",
				 (*chanptr)-> ch_name));
		}
		
	}
}

extern int numBps, forwMsg;
extern char	*cont_822;

int ch_bind (qp, ad, p, pfmts, pbpts, pcost, punknown)
Q_struct *qp;
ADDR    	*ad;
LIST_RCHAN	*p;
LIST_RCHAN	**pfmts;
LIST_BPT	**pbpts;
int		*pcost, *punknown;
{
	MAT     *mat;
	CHAN    *out_chan;
	TYPES   *chan, *messty, *outty, *hdrty, *t2, *ty = NULL;
	LIST    *list = NULL, *worklist;
	LIST_BPT	*new_bpt = NULLIST_BPT;
	LIST_RCHAN	*new_chan;
	FILTER  *filt;
	FLAT	*flat;
	char    *filtername, *in_content;
	int     cost, ind, unknownOutBP;
	
	*pfmts = NULLIST_RCHAN;
	*pcost = 0;
	*pbpts = NULLIST_BPT;
	
	PP_TRACE (("ch_bind ()"));
	
	
	/*	in_content = ch_inbound->ch_content_in; */
	in_content = qp -> cont_type;
	if (p == NULLIST_RCHAN)
		err_abrt (RP_MECH, "No channel in address for %s",
			  ad -> ad_value);
	out_chan = p->li_chan;
	
	/* Messty - EITs within the message */
	messty = addchantypes (qp -> encodedinfo.eit_types, 
			       punknown);
	
#if PP_DEBUG > 1
	if (pp_log_norm -> ll_events & LLOG_DEBUG) {
		/* Testing Blurb */
		if (out_chan->ch_hdr_out) {
			chan = addchantypes (out_chan->ch_hdr_out,
					     &unknownOutBP);
			if (unknownOutBP == TRUE)
				PP_NOTICE(("ch_bind/Channel %s has an undeclared outbound header(s)",
					   out_chan->ch_name));
			PP_DBG (("ch_bind/Channel %s supports headers ->", out_chan->ch_name));
			for (; chan != NULL; chan = chan->ty_next)
				PP_DBG (("\t`%s'", chan->ty_name));
			PP_DBG (("EOL"));
		}
		if (out_chan->ch_bpt_out) {
			chan = addchantypes (out_chan->ch_bpt_out,
					     &unknownOutBP);
			if (unknownOutBP == TRUE)
				PP_NOTICE(("ch_bind/Channel %s has an undeclared outbound bodypart(s)",
					   out_chan->ch_name));
			
			PP_DBG (("ch_bind/Channel %s supports bodyparts ->", out_chan->ch_name));
			for (; chan != NULL; chan = chan->ty_next)
				PP_DBG (("\t`%s'", chan->ty_name));
			PP_DBG (("EOL"));
		}
		/* End of Testing Blurb */
	}
#endif
	worklist = NULL;
	if (out_chan -> ch_hdr_out != NULLIST_BPT ||
	    out_chan -> ch_bpt_out != NULLIST_BPT) {
		if (out_chan -> ch_bpt_out != NULLIST_BPT) {
			outty = addchantypes (out_chan->ch_bpt_out,
					      &unknownOutBP);
			if (unknownOutBP == TRUE)
				PP_NOTICE(("ch_bind/Channel %s has an undeclared outbound bodypart(s)",
					   out_chan->ch_name));
		} else 
			outty = NULL;
		if (out_chan -> ch_hdr_out != NULLIST_BPT) {
			hdrty = addchantypes (out_chan->ch_hdr_out,
					      &unknownOutBP);
			if (unknownOutBP == TRUE)
				PP_NOTICE(("ch_bind/Channel %s has an undeclared outbound header(s)",
					   out_chan->ch_name));

		} else
			hdrty = NULL;

		for (chan = messty; chan != NULL; chan = chan->ty_next)
		{
			cost = 0;

			/* SEK addchantypes is called 4 times
			   once on each channel - this seems excessive */

			/* SEK this loop initialises for inbound  */
			for (t2 = outty; t2 != NULL; t2 = t2->ty_next)
			{
				ind = chan->ty_number * matsize + t2->ty_number;
				mat = &mainmatrix[ind];
				if ((cost == 0 && mat->m_int != 0) ||
				    (cost != 0 && cost > mat->m_int
				     && mat->m_int != 0))
				{
					list = mat->m_list;
					cost = mat->m_int;
					ty = t2;
				}
			}

			if (cost == 0) {
				for (t2 = hdrty; t2 != NULL; t2 = t2->ty_next)
				{
					ind = chan->ty_number * matsize + t2->ty_number;
					mat = &mainmatrix[ind];
					if ((cost == 0 && mat->m_int != 0) ||
					    (cost != 0 && cost > mat->m_int
					     && mat->m_int != 0))
					{
						list = mat->m_list;
						cost = mat->m_int;
						ty = t2;
					}
				}
			}

			if (cost == 0) {
				PP_LOG (LLOG_NOTICE,
					("ch_bind/Can't convert %s", chan->ty_name));
				if (worklist) freelist(worklist);
				return NOTOK;
			}
			else {
				if (list_bpt_find (new_bpt, ty->ty_name) ==
				    NULLIST_BPT)
					list_bpt_add (&new_bpt,
						      list_bpt_new (ty->ty_name));
				if (cost == 1) {
					PP_TRACE (("Cost = 1"));
					continue;
				}
				else {
					worklist = inslist (worklist, 
							    copylist(list));
				}
			}
		}
	}

	/* if NO EITS, NO unflatten! */
	if (qp -> encodedinfo.eit_types != NULL) {
		/* unflatten message if
		 *	submitted with a content and
		 * 	either converting EITS or
		 * 	there is a change in content type (or to none)
		 */
		if (isstr(in_content) &&
		    (worklist != NULL ||
		     !isstr(out_chan->ch_content_out) ||
		     strcmp (out_chan->ch_content_out,
			     in_content) != 0)) {
			flat = flttype2ptr (unflatlist, in_content);
			if (flat != NULL) {
				filtername = flat -> flt_name;
				PP_DBG (("ch_bind/UNFLATTEN with %s",
					 filtername));
				new_chan = list_rchan_new (NULLCP, filtername);
				*pcost += new_chan -> li_chan -> ch_cost;
				list_rchan_add (pfmts, new_chan);
			}
			else
				PP_LOG (LLOG_EXCEPTIONS,
					("Can't find exploder for %s",
					 in_content));
		}
	}
	/* SEK - calculate real work */
	for (list = worklist; list != NULL; list = list->li_next)
	{
		filt = name2filter (list->li_name);
		if (filt == NULL) {
			PP_LOG (LLOG_EXCEPTIONS, ("No filter defined for %s",
						  list -> li_name));
			continue;
		}
		PP_DBG (("ch_bind/%s  [%s to %s]\n", filt->fil_name,
			 filt->fil_from, filt->fil_to));
		new_chan = list_rchan_new (NULLCP, filt -> fil_name);
		*pcost += new_chan -> li_chan -> ch_cost;
		list_rchan_add (pfmts, new_chan);
	}

	/* No EITS, No flatten */
	if (qp -> encodedinfo.eit_types != NULL) {
		/*
		 * flatten message if
		 *	outchan has a content and
		 *	message isn't straight 822 format and
		 *	either converting EITS or
		 *	changing content type (or from none)
		 */
		if (isstr(out_chan -> ch_content_out) &&
		    (lexequ(out_chan->ch_content_out, cont_822) != 0
		     || isstr(in_content)
		     || numBps > 2 || forwMsg > 0) &&
		    (worklist != NULL || !isstr(in_content) ||
		     strcmp (in_content, out_chan->ch_content_out) != 0)) {
			flat = flttype2ptr (flattenlist,
					    out_chan->ch_content_out);
			if (flat != NULL) {
				filtername = flat -> flt_name;
				PP_DBG (("ch_bind/FLATTEN with %s",
					 filtername));
				new_chan = list_rchan_new (NULLCP, filtername);
				*pcost += new_chan -> li_chan -> ch_cost;
				list_rchan_add (pfmts, new_chan);
			}
			else
				PP_LOG (LLOG_EXCEPTIONS,
					("Can't find flattener for %s",
					 out_chan -> ch_content_out));
		}
	}
	if (ad -> ad_content == NULLCP
	    && isstr (in_content) && isstr (out_chan -> ch_content_out) &&
	    strcmp (in_content, out_chan -> ch_content_out) != 0)
		ad -> ad_content = strdup (out_chan -> ch_content_out);
	else if (ad -> ad_content == NULLCP && 
		 isstr(out_chan -> ch_content_out))
		ad -> ad_content = strdup (out_chan -> ch_content_out);

        *pbpts = new_bpt;
	if (worklist)  freelist(worklist);
	return OK;
}
