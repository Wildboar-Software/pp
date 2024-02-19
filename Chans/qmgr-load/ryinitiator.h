/* ryinitiator.h - include file for the generic interactive initiator */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/ryinitiator.h,v 6.0 1991/12/18 20:11:26 jpo Rel $
 *
 * $Log: ryinitiator.h,v $
 * Revision 6.0  1991/12/18  20:11:26  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_QMGRLOAD_RYINITIATOR
#define _H_QMGRLOAD_RYINITIATOR

#include <isode/rosy.h>


static struct client_dispatch {
	char		*ds_name;
	int		ds_operation;

	IFP		ds_argument;
#ifdef	PEPSY_VERSION
	modtyp		*ds_fr_mod;
	int		ds_fr_index;
#else
	IFP		ds_free;
#endif
	IFP		ds_result;
	IFP		ds_error;

	char		*ds_help;
} Client_dispatch;


void			acs_adios(), acs_advise();
void			ros_adios(), ros_advise();
int			ryinitiator();

#endif
