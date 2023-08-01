/* ch_bind.h: channel binding types */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/ch_bind.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: ch_bind.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_CH_BIND
#define _H_CH_BIND

/* SEK - someone must document this !! */

typedef	struct	list {
	struct	list	*li_next;
	char		*li_name;
} LIST;
	
typedef	struct {
	int	m_int;
	LIST	*m_list;
} MAT;

typedef	struct	types {
	struct	types	*ty_next;
	int		ty_number;
	char		*ty_name;
} TYPES;

typedef	struct	flat {
	struct	flat	*flt_next;
	char		*flt_name;
	char		*flt_type;
} FLAT;

typedef	struct	fillist {
	struct	fillist	*fil_next;
	char		*fil_name;
	char		*fil_from;
	char		*fil_to;
} FILTER;
#endif
