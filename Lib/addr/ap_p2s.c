/* ap_p2s.c: convert parts to a string */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_p2s.c,v 6.0 1991/12/18 20:21:24 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/addr/RCS/ap_p2s.c,v 6.0 1991/12/18 20:21:24 jpo Rel $
 *
 * $Log: ap_p2s.c,v $
 * Revision 6.0  1991/12/18  20:21:24  jpo
 * Release 6.0
 *
 */



/*
Format one address from pointers to constitutents, in a tree
	Returns:    pointer to string if successful or
	NOTOK if error

	Using ap_p2s as output for ap_t2s has the general problem
	of losing comments.   Perhaps ap_t2s should be  separate?
*/



#include "util.h"
#include "chan.h"
#include "ap.h"

static void ap_compress_local();

extern int      ap_outtype;

extern void  	ad_val2str ();

/* ---------------------  Begin  Routines  -------------------------------- */

static char	*ap_p2s_aux();

char *ap_p2s(group, name, local, domain, route)
AP_ptr	group,
	name,
	local,
	domain,
	route;
{
	return ap_p2s_aux (group, name, local, domain, route, TRUE);
}

char *ap_p2s_nc(group, name, local, domain, route)
AP_ptr	group,
	name,
	local,
	domain,
	route;
{
	return ap_p2s_aux (group, name, local, domain, route, FALSE);
}


static char *ap_p2s_aux (group, name, local, domain, route, comments)
AP_ptr                  group,          /* -- start of group name -- */
			name,           /* -- start of person name -- */
			local,          /* -- start of local-part -- */
			domain,         /* -- basic domain ref -- */
			route;          /* -- start of 733 forw routing -- */
int			comments;
{

	AP_ptr          last_ptr;
	char            *route_ptr,     /* -- 822 -> 733 route ptr */
			*flip_ptr, *subdom = NULLCP,
			*dref_ptr,      /* -- ptr to output str -- */
			tmpbuf [BUFSIZ],
			buf[BUFSIZ];  /* -- buf for tb_getdomain -- */
	int             in_person,
			in_group,
			last,
			after_comment;
	register AP_ptr cur_ptr = NULLAP;
	register char   *s_ptr,         /* -- the str we are building -- */
			*c_ptr;


	PP_DBG (("Lib/adr/ap_p2s()"));

	PP_DBG (("Lib/addr/ap_p2s/AP_PARSE_822 %s",
				ap_outtype & AP_PARSE_822?"on":"off"));

	PP_DBG (((ap_outtype & AP_PARSE_BIG) ?
				"AP_PARSE_BIG on" : "AP_PARSE_LITTLE on"));


	in_person       = FALSE;
	in_group        = FALSE;
	s_ptr           = strdup ("");
	route_ptr       = strdup ("");
	last		= AP_NIL;
	after_comment	= FALSE;

	if (!comments)
		ap_compress_local(group,name,local,domain,route,comments);
		
	if (group != NULLAP) {

		PP_DBG (("Lib/addr/ap_p2s:  group is '%s'",
				group -> ap_obvalue));

		for (cur_ptr = group; cur_ptr != NULLAP;
		     cur_ptr = cur_ptr->ap_next) {

			if (cur_ptr == name || cur_ptr == local ||
			    cur_ptr == domain || cur_ptr == route)
				break;

			/* -- print munged addr -- */

			switch (cur_ptr -> ap_obtype) {
			default:
			case AP_NIL:
				break;

			case AP_COMMENT:
				if (comments == TRUE) {
					/* -- output value as comment -- */
					ap_val2str (tmpbuf,
						    cur_ptr -> ap_obvalue,
						    AP_COMMENT);

					c_ptr = multcat (s_ptr,
							 (s_ptr[0]?" ":""),
							 "(",
							 tmpbuf,
							 ")",
							 NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					s_ptr = c_ptr;
					after_comment = TRUE;
				}
				continue;

			case AP_GROUP_NAME:
				in_group = TRUE;

			case AP_GROUP_START:
				ap_val2str (tmpbuf,
					cur_ptr -> ap_obvalue,
					AP_GROUP_START);
				c_ptr = multcat (s_ptr, 
						 (((s_ptr[0]&&last==AP_GROUP_START) || after_comment == TRUE)?" ":""),
						 tmpbuf, NULLCP);
				free (s_ptr);
				if (c_ptr == NULLCP)
					return ((char *)NOTOK);
				s_ptr = c_ptr;
				last = AP_GROUP_START;
				after_comment = FALSE;
				continue;
			}

			break;
		}


		if (in_group) {
			c_ptr = multcat (s_ptr, ":", NULLCP);
			free (s_ptr);
			if (c_ptr == NULLCP)
				return ((char *)NOTOK);
			s_ptr = c_ptr;
			/* need space after this yuch */
			after_comment = TRUE;
		}
	}



	if (name != NULLAP) {
		PP_DBG (("LIb/addr/ap_p2s:  name is '%s'",
				name -> ap_obvalue));

		for (cur_ptr = name; cur_ptr != NULLAP;
		     cur_ptr = cur_ptr -> ap_next) {
			
			if (cur_ptr == group || cur_ptr == local ||
			    cur_ptr == domain || cur_ptr == route)
				break;

			/* -- print munged addr -- */
			switch (cur_ptr -> ap_obtype) {
			default:
			case AP_NIL:
				break;

			case AP_COMMENT:
				if (comments == TRUE) {
					/* -- output value as comment -- */
					ap_val2str (tmpbuf,
						    cur_ptr -> ap_obvalue,
						    AP_COMMENT);

					c_ptr = multcat (s_ptr,
							 (s_ptr[0]?" " : ""),
							 "(",
							 tmpbuf,
							 ")",
							 NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					s_ptr = c_ptr;
					after_comment = TRUE;
				}
				continue;

			case AP_PERSON_NAME:
				in_person = TRUE;

			case AP_PERSON_START:
				ap_val2str (tmpbuf,
					cur_ptr -> ap_obvalue,
					AP_PERSON_START);
				c_ptr = multcat (s_ptr, 
						 (((s_ptr[0] && last == AP_PERSON_START) || after_comment == TRUE)?" ":""),
						 tmpbuf, NULLCP);
				free (s_ptr);
				if (c_ptr == NULLCP)
					return ((char *)NOTOK);
				s_ptr = c_ptr;
				last = AP_PERSON_START;
				after_comment = FALSE;
				continue;
			}

			break;
		}
	}



	if (in_person) {
		c_ptr = multcat (s_ptr, " <", NULLCP);
		free (s_ptr);
		if (c_ptr == NULLCP)
			return ((char *)NOTOK);
		s_ptr = c_ptr;
		after_comment = FALSE;
	}



	if (route != NULLAP) {
		/* -- routing info exits -- */
		PP_DBG (("Lib/addr/ap_p2s:  route is '%s'",
				route -> ap_obvalue));

		for (last_ptr = cur_ptr = route; ;
		     cur_ptr = cur_ptr -> ap_next) {

			/* -- grot grot !!!!!!! -- */
			if (cur_ptr == NULLAP)
				goto defcase1;

			switch (cur_ptr -> ap_obtype) {
			case AP_PERSON_END:
				continue;

			defcase1:;
				/* yeuch !! */
			default:

			case AP_NIL:
				if ((ap_outtype & AP_PARSE_822) ==
				     AP_PARSE_822) {
					/* -- piece of cake -- */
					c_ptr = multcat (s_ptr, ":", NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					s_ptr = c_ptr;
				}
				break;

			case AP_COMMENT:
				if (comments == TRUE) {
					/* -- output value as comment -- */
					ap_val2str (tmpbuf,
						    cur_ptr -> ap_obvalue,
						    AP_COMMENT);
					after_comment = TRUE;
					c_ptr = multcat (s_ptr,
							 (s_ptr[0]?" ":""),
							 "(",
							 tmpbuf,
							 ")",
							 NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					s_ptr = c_ptr;
				}
				continue;


			case AP_DOMAIN_LITERAL:
			case AP_DOMAIN:
				ap_val2str (tmpbuf,
					cur_ptr -> ap_obvalue,
					cur_ptr -> ap_obtype);
				flip_ptr = NULLCP;
				dref_ptr = tmpbuf;

				/* -- only flip valid domains -- */
				if ((ap_outtype & AP_PARSE_BIG) ==
				     AP_PARSE_BIG &&
				     tb_getdomain (tmpbuf, NULLCP,
						   buf,
#ifdef UKORDER
						   CH_UK_PREF,
#else
						   CH_USA_PREF,
#endif
						   &subdom) == OK) {
					/* -- usa ordering preferred -- */
					flip_ptr = ap_dmflip (buf);
					dref_ptr = flip_ptr;
				}
				if (subdom != NULLCP) {	
					free(subdom);subdom=NULLCP;
				}
				if ((ap_outtype & AP_PARSE_822) ==
				     AP_PARSE_822) {
					/* -- piece of cake -- */
					c_ptr = multcat (
						s_ptr,
/*						(((s_ptr[0] && last == AP_DOMAIN) || after_comment == TRUE)?" ":""),*/
						((after_comment == TRUE)?" ":""),
						(cur_ptr!=last_ptr?",@":"@"),
						dref_ptr,
						NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					last = AP_DOMAIN;
					after_comment = FALSE;
					s_ptr = c_ptr;
				}
				else {
					if (route_ptr[0] == '\0')
						c_ptr = multcat (
							"@",
							dref_ptr,
							NULLCP);
					else
						c_ptr = multcat (
							"%",
							dref_ptr,
							route_ptr,
							NULLCP);
					free (route_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					route_ptr = c_ptr;
				}

				if (flip_ptr != NULLCP)
					free (flip_ptr);
				continue;
			}

			break;
		}
	}


	if (local != NULLAP) {
		PP_DBG (("LIb/addr/ap_p2s:  local is '%s'",
				local -> ap_obvalue));

		for (cur_ptr = local; cur_ptr != NULLAP;
		     cur_ptr = cur_ptr -> ap_next) {

			if (cur_ptr == group || cur_ptr == name ||
			    cur_ptr == domain || cur_ptr == route)
				break;
			/* -- print munged addr -- */
			switch (cur_ptr -> ap_obtype) {
			default:
			case AP_NIL:
				break;

			case AP_COMMENT:
				if (comments == TRUE) {
					/* -- don't skip these -- */
					ap_val2str (tmpbuf,
						    cur_ptr -> ap_obvalue,
						    AP_COMMENT);

					c_ptr = multcat (s_ptr,
							 (s_ptr[0]?" ":""),
							 "(",
							 tmpbuf,
							 ")",
							 NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					s_ptr = c_ptr;
					after_comment = TRUE;
				}
				continue;

			case AP_GENERIC_WORD:
			case AP_MAILBOX:
				/* -- yuk -- */
				ap_val2str (tmpbuf,
					cur_ptr -> ap_obvalue,
					AP_MAILBOX);
				c_ptr = multcat (s_ptr, 
						 (((s_ptr[0]&&last==AP_MAILBOX) || after_comment == TRUE)?" ":""),
						 tmpbuf, NULLCP);
				free (s_ptr);
				if (c_ptr == NULLCP)
					return ((char *)NOTOK);
				s_ptr = c_ptr;
				last = AP_MAILBOX;
				after_comment = FALSE;
				continue;
			}

			break;
		}
	}



	if (domain != NULLAP) {
		PP_DBG (("Lib/addr/ap_p2s:  domain is '%s'",
				domain -> ap_obvalue));

		for (cur_ptr = domain; cur_ptr != NULLAP;
		     cur_ptr = cur_ptr->ap_next) {
			
			if (cur_ptr == group || cur_ptr == name ||
			    cur_ptr == local || cur_ptr == route)
				break;
			switch (cur_ptr->ap_obtype) {
			    default:
			    case AP_NIL:
				break;

			    case AP_COMMENT:
				if (comments == TRUE) {
					ap_val2str(tmpbuf,
						   cur_ptr->ap_obvalue,
						   AP_COMMENT);
					c_ptr = multcat (s_ptr,
							 (s_ptr[0]?" ":""),
							 "(",
							 tmpbuf,
							 ")",
							 NULLCP);
					free (s_ptr);
					if (c_ptr == NULLCP)
						return ((char *)NOTOK);
					s_ptr = c_ptr;
					after_comment = TRUE;
				}
				continue;
				
			    case AP_DOMAIN:
				ap_val2str (tmpbuf, domain -> ap_obvalue, domain -> ap_obtype);
				flip_ptr = NULLCP;
				dref_ptr = tmpbuf;

				/* -- only flip valid domains -- */
				if ((ap_outtype & AP_PARSE_BIG) == AP_PARSE_BIG &&
				    tb_getdomain (tmpbuf, 
						  NULLCP, buf, 
#ifdef UKORDER
						  CH_UK_PREF,
#else
						  CH_USA_PREF,
#endif
						  &subdom) == OK) {
					/* -- usa ordering preferred -- */
					flip_ptr = ap_dmflip (buf);
					dref_ptr = flip_ptr;
				}
				if (subdom != NULLCP) {	
					free(subdom);subdom=NULLCP;
				}

				if ((ap_outtype & AP_PARSE_822) == AP_PARSE_822 ||
				    route_ptr[0] == '\0')
					/* -- easy -- */
					c_ptr = multcat (s_ptr,
							 "@",
							 dref_ptr,
							 route_ptr,
							 NULLCP);
				else
					c_ptr = multcat (s_ptr,
							 "%",
							 dref_ptr,
							 route_ptr,
							 NULLCP);

				if (c_ptr == NULLCP)
					return (c_ptr);
				free (s_ptr);
				s_ptr = c_ptr;
				if (flip_ptr != NULLCP)
					free (flip_ptr);	
				after_comment = FALSE;
				continue;
			}
			break;
		}
	} else if (route_ptr[0] != '\0') {
		c_ptr = multcat (s_ptr,
				 route_ptr,
				 NULLCP);

		if (c_ptr == NULLCP)
			return (c_ptr);
		free (s_ptr);
		s_ptr = c_ptr;
		after_comment = FALSE;
	}


	free (route_ptr);

	if (in_person) {
		c_ptr = multcat (s_ptr, ">", NULLCP);
		free (s_ptr);
		if (c_ptr == NULLCP)
			return ((char *)NOTOK);
		s_ptr = c_ptr;
		after_comment = FALSE;
		if (cur_ptr != NULL && cur_ptr->ap_obtype == AP_PERSON_END)
			cur_ptr = cur_ptr->ap_next;
	}

	if (cur_ptr != NULL 
	    && cur_ptr->ap_obtype == AP_GROUP_END) {
		c_ptr = multcat (s_ptr, ";", NULLCP);
		free (s_ptr);
		if (c_ptr == NULLCP)
			return ((char *)NOTOK);
		s_ptr = c_ptr;
		in_group = FALSE;
		cur_ptr = cur_ptr -> ap_next;
	}

	if (comments == TRUE &&
	    cur_ptr != NULL && cur_ptr->ap_obtype == AP_COMMENT) {
		while (cur_ptr != NULL 
		       && cur_ptr->ap_obtype == AP_COMMENT) {
			/* trailing comments */
			ap_val2str(tmpbuf,
				   cur_ptr->ap_obvalue,
				   AP_COMMENT);
			c_ptr = multcat (s_ptr, 
					 (s_ptr[0]?" ":""),
					 "(",
					 tmpbuf,
					 ")",
					 NULLCP);
			free (s_ptr);
			if (c_ptr == NULLCP)
				return ((char *) NOTOK);
			s_ptr = c_ptr;
			after_comment = TRUE;
			cur_ptr = cur_ptr->ap_next;
		}
	}
				
	return (s_ptr);
}


static void ap_compress_local (group, name, local, domain, route, comments)
AP_ptr                  group,          /* -- start of group name -- */
			name,           /* -- start of person name -- */
			local,          /* -- start of local-part -- */
			domain,         /* -- basic domain ref -- */
			route;          /* -- start of 733 forw routing -- */
int			comments;
{
	/* compress local into one AP_ptr */
	AP_ptr	ix, trc;
	register char	*s_ptr,
			*c_ptr;
	char		tmpbuf [BUFSIZ];

	if (local == NULLAP)
		return;
	s_ptr = strdup("");
	for (ix = local; ix != NULLAP; ix = ix -> ap_next) {
		if (ix == group || ix == name ||
		    ix == domain || ix == route)
			break;
		switch (ix -> ap_obtype) {
		    default:
		    case AP_NIL:
			break;

		    case AP_COMMENT:
			if (comments == TRUE) {
				ap_val2str (tmpbuf,
					    ix -> ap_obvalue,
					    AP_COMMENT);
				c_ptr = multcat (s_ptr,
						 (s_ptr[0] ? " " : ""),
						 "(",
						 tmpbuf,
						 ")",
						 NULLCP);
				free(s_ptr);
				if (c_ptr == NULLCP)
					return;
				s_ptr = c_ptr;
			}
			continue;
			
		    case AP_GENERIC_WORD:
		    case AP_MAILBOX:
			/* -- yuk -- */
			ap_val2str (tmpbuf,
				    ix -> ap_obvalue,
				    AP_MAILBOX);
			c_ptr = multcat (s_ptr, 
					 ((s_ptr[0] && 
					   s_ptr[(strlen(s_ptr) -1)] != '"') 
					  ?" ":""),
					 tmpbuf, NULLCP);
			free (s_ptr);
			if (c_ptr == NULLCP)
				return;
			s_ptr = c_ptr;
			continue;
		}
		break;
	}

	if (ix == local -> ap_next) {
		/* no need to compress */
		free(s_ptr);
		return;
	}
	
	for (trc = local; trc -> ap_next && trc -> ap_next != ix; ) {
		switch (trc -> ap_next -> ap_obtype) {
		    case AP_GENERIC_WORD:
		    case AP_MAILBOX:
			ap_delete (trc);
			break;
		    default:
			trc = trc -> ap_next;
			break;
		}
	}

	switch (local -> ap_obtype) {
		case AP_GENERIC_WORD:
		case AP_MAILBOX:
			free(local -> ap_obvalue);
			local -> ap_obvalue = s_ptr;
			break;
		default:
			trc = ap_alloc ();
			trc -> ap_obvalue = s_ptr;
			trc -> ap_obtype = AP_MAILBOX;
			ap_insert (local, AP_PTR_MORE, trc);
			break;
	}
}
