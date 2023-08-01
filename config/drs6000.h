#ifndef _H_CONFIG
#define _H_CONFIG

#ifndef SYS5
#define         SYS5            /* System 5 */
#ifndef SVR4
#define         SVR4            /*    Release 4 */
#endif
#endif

#define         GDBM

/* define the following if you require BIND nameserver support for SMTP */
/* #define NAMESERVER   */

#define PP_DEBUG_ALL    2
#define PP_DEBUG_SOME   1
#define PP_DEBUG_NONE   0

#define PP_DEBUG PP_DEBUG_ALL

/*
 * define the following if  you require user tools to use uk domain ordering 
 * by default
 * #define UKORDER
 */
#define UNICORN

#endif
