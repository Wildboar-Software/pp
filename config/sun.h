/* config.h: compile time configuration parameters */

/*
 * @(#) $Header$
 *
 * $Log$
 *
 */



#ifndef _H_CONFIG
#define _H_CONFIG

#define         NDBM


/* define the following if you require BIND nameserver support for SMTP */
/* #define NAMESERVER	*/

#define	PP_DEBUG_ALL	2
#define PP_DEBUG_SOME	1
#define PP_DEBUG_NONE	0

#define PP_DEBUG PP_DEBUG_ALL

/*
 * define the following if  you require user tools to use uk domain ordering 
 * by default
 * #define UKORDER
 */

/* Add after this anything specific to your host. */

#endif
