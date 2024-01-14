/* ryresponder.h - include file for the generic idempotent responder */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/ryresponder.h,v 6.0 1991/12/18 20:27:38 jpo Rel $
 *
 * $Log: ryresponder.h,v $
 * Revision 6.0  1991/12/18  20:27:38  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_QMGRLOAD_RYRESPONDER
#define _H_QMGRLOAD_RYRESPONDER


#include <isode/logger.h>


struct server_dispatch {
	char	*ds_name;
	int	ds_operation;
	IFP	ds_vector;
};


extern int	debug;

void adios (char *what, char* fmt, ...);
void advise (int code, char *what, char *fmt, ...);
void ryr_advise (char *what, char* fmt, ...);
void		acs_advise();
void		ros_adios(), ros_advise();
int		ryresponder();

#endif
