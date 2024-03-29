/* ryinitiator.h - include file for the generic interactive initiator */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Tools/ckmail/RCS/ryinitiator.h,v 6.0 1991/12/18 20:29:15 jpo Rel $
 *
 * $Log: ryinitiator.h,v $
 * Revision 6.0  1991/12/18  20:29:15  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_QMGRLOAD_RYINITIATOR
#define _H_QMGRLOAD_RYINITIATOR

#include <isode/rosy.h>


struct client_dispatch {
	char		*ds_name;
	int		ds_operation;

	IFP		ds_argument;
#ifdef PEPSY_VERSION
	modtyp		*ds_fr_mod;
	int		ds_fr_index;
#else
	IFP		ds_free;
#endif
	IFP		ds_result;
	IFP		ds_error;

	char		*ds_help;
};


void advise (char *what, char *fmt, ...);
void adios (char *what, char* fmt, ...);
void			acs_adios(), acs_advise();
void			ros_adios(), ros_advise();
int			ryinitiator();

#endif
