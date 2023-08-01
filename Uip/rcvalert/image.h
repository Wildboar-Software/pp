/* image.h: image definition */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/image.h,v 6.0 1991/12/18 20:40:03 jpo Rel $
 *
 * $Log: image.h,v $
 * Revision 6.0  1991/12/18  20:40:03  jpo
 * Release 6.0
 *
 *
 */

struct image {
	int 	width, height;
	struct qbuf *data;
	char	*ufnname;
	char 	*phone;
	char 	*description;
	char	*info;
	char 	*address;
	PE	pe;
};

extern struct image *fetch_image ();
