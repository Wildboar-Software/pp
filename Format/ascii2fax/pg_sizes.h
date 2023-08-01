/* pg_sizes.h: various macros/defines to do with fax page sizes */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/pg_sizes.h,v 6.0 1991/12/18 20:15:19 jpo Rel $
 *
 * $Log: pg_sizes.h,v $
 * Revision 6.0  1991/12/18  20:15:19  jpo
 * Release 6.0
 *
 *
 */

#define X_ALLOWED_NOT_PRINT	56
/* pixels standard allows not to print in both x directions */
#define	Y_ALLOWED_NOT_PRINT_TOP	4
/* mm standard allows not to print at top of page */
#define Y_ALLOWED_NOT_PRINT_BOTTOM   7 /* really 6.77 */
/* mm standard allows not to print at bottom of page */

/* a4 */
#ifdef A4
# define FAX_WIDTH	210 /* mm */
# define FAX_HEIGHT	297 /* mm */
#else
# define FAX_WIDTH	216 /* mm */
# define FAX_HEIGHT	273 /* mm */
#endif


/* res */
# define LINES_PER_MM	3.85 /* 7.7 */

# define LEFT_PAD       30 	/* mm */ /* 29.5 */
# define TOP_PAD	6	/* mm */
# define BOTTOM_PAD	2	/* mm */

# define FAX_WIDTH_LINES	1728 /* standard ? */

# define FAX_HEIGHT_LINES	((int)(FAX_HEIGHT * LINES_PER_MM))
# define TEXT_HEIGHT_LINES	((int)((FAX_HEIGHT - BOTTOM_PAD - Y_ALLOWED_NOT_PRINT_BOTTOM) * LINES_PER_MM))
# define LEFT_PAD_LINES	((int)(LEFT_PAD * LINES_PER_MM) + X_ALLOWED_NOT_PRINT)
# define TOP_PAD_LINES	((int)((TOP_PAD + Y_ALLOWED_NOT_PRINT_TOP) * LINES_PER_MM))
