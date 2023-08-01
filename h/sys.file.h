/* sys.file.h - system independent sys/file.h */

/* 
 * $Header: /xtel/pp/pp-beta/h/RCS/sys.file.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 *
 * $Log: sys.file.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */

#ifndef _PP_SYS_FILE_H
#define _PP_SYS_FILE_H

#if ISODE > 68

#include <isode/sys.file.h>

#else 

#include <isode/general.h>
#include "util.h"

/* Beware the ordering is important to aviod symbol clashes */

#ifndef SVR4_UCB
#include <sys/ioctl.h>
#endif

#ifdef  BSD42
#include <sys/file.h>
#else    
#ifdef  SYS5
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#endif

#ifdef SYS5
#include <termio.h>
#endif

#endif 
#endif
