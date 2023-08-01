/* ut_aparse.c: utility routines for the aparse structure */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_aparse.c,v 6.0 1991/12/18 20:49:15 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/pp/RCS/ut_aparse.c,v 6.0 1991/12/18 20:49:15 jpo Rel $
 *
 * $Log: ut_aparse.c,v $
 * Revision 6.0  1991/12/18  20:49:15  jpo
 * Release 6.0
 *
 */

#include "aparse.h"

static void aparse_init();

Aparse_ptr aparse_new ()
{
	Aparse_ptr ret;

	ret = (Aparse_ptr) (smalloc (sizeof(Aparse)));
	aparse_init(ret);
	return ret;
}
	
static void aparse_init(ap)
Aparse_ptr	ap;
{
	bzero((char *) ap, sizeof (*ap));
	ap->orname = (ORName *) calloc(1, sizeof(ORName));
}

void aparse_free (ap)
Aparse_ptr	ap;
{
	if (ap->ap_tree)
		(void) ap_sqdelete (ap->ap_tree, NULLAP);
	if (ap->r822_str)
		free(ap->r822_str);
	if (ap->r822_local)
		free(ap->r822_local);
	if (ap->r822_error)
		free(ap->r822_error);

	if (ap->orname)
		ORName_free(ap->orname);
	if (ap->x400_dn)
		free(ap->x400_dn);
	if (ap->x400_str)
		free(ap->x400_str);
	if (ap->x400_local)
		free(ap->x400_local);
	if (ap->x400_error)
		free(ap->x400_error);

	if (ap->aliases)
		aparse_freeAliasList(ap->aliases);

	if (ap->local_hub_name)
		free(ap->local_hub_name);
}
	
/*  */

void aparse_rewindx400(ap)
Aparse_ptr	ap;
{
	if (ap->orname->on_or) {
		or_free(ap->orname->on_or);
		ap->orname->on_or = NULLOR;
	}
	if (ap->orname->on_dn) {
		;
	}

	if (ap->x400_dn) {
		free (ap->x400_dn);
		ap->x400_dn = NULLCP;
	}
	if (ap->x400_str) {
		free (ap->x400_str);
		ap->x400_str = NULLCP;
	}
	if (ap->x400_local) {
		free (ap->x400_local);
		ap->x400_local = NULLCP;
	}
}

void aparse_rewindr822(ap)
Aparse_ptr	ap;
{
	if (ap->ap_tree) {
		(void) ap_sqdelete (ap->ap_tree, NULLAP);
		ap->ap_tree = NULLAP;
		ap->ap_group = NULLAP;
		ap->ap_name = NULLAP;
		ap->ap_local = NULLAP;
		ap->ap_domain = NULLAP;
		ap->ap_route = NULLAP;
	}

	if (ap->r822_str) {
		free (ap->r822_str);
		ap->r822_str = NULLCP;
	}
	if (ap->r822_local) {
		free (ap->r822_local);
		ap->r822_local = NULLCP;
	}
}


void aparse_rewindlocal(ap)
Aparse_ptr	ap;
{
	if (ap->local_hub_name) {
		free(ap->local_hub_name);
		ap->local_hub_name=NULLCP;
	}
}

void aparse_copy_for_recursion(to, from)
Aparse_ptr	to, from;
{
	to->dmnorder = from->dmnorder;
	to->full = from->full;
	to->percents = from->percents;
	to->recurse_count = from->recurse_count+1;
	to->aliases = aparse_aliasDup(from->aliases);
	to->normalised = from->normalised;
	to->dont_use_aliases = from->dont_use_aliases;
}
	
/*  */

/* alias list stuff */

void aparse_freeAliasList(list)
aliasList       *list;
{
        if (list->alias) free(list->alias);
        if (list->next) aparse_freeAliasList(list->next);
        free((char *) list);
}

static aliasList        *newAliasList(alias)
char    *alias;
{
        aliasList       *ret = (aliasList *) calloc(1, sizeof(*ret));
        ret->alias = strdup(alias);
        return ret;
}

void aparse_addToAliasList(alias, plist)
char    *alias;
aliasList	**plist;
{
        aliasList       *tmp = newAliasList(alias);
        if (*plist == NULL)
                *plist = tmp;
        else {
                tmp->next = *plist;
                *plist = tmp;
        }
}

int aparse_inAliasList(alias, list)
char    *alias;
aliasList	*list;
{
        aliasList       *ix = list;

        while (ix != NULL && strcmp(ix->alias, alias) != 0)
                ix = ix->next;

        if (ix != NULL)
                return OK;
        return NOTOK;
}

aliasList *aparse_aliasDup(list)
aliasList	*list;
{
	aliasList	*ret = NULL;

	if (list) {
		ret = newAliasList(list->alias);
		ret->next = aparse_aliasDup(list->next);
	}
	return ret;
}
	
