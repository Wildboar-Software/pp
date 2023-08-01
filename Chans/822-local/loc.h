/* loc.h: 822-local definitions */

/*
 * @(#) $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/loc.h,v 6.0 1991/12/18 20:05:47 jpo Rel $
 *
 * $Log: loc.h,v $
 * Revision 6.0  1991/12/18  20:05:47  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_822LOCAL_LOC
#define _H_822LOCAL_LOC


typedef enum { strvar = 1, field, constant } stype;


typedef struct Symbol {
	char		*name;
	stype		type;
	char		*str;
	struct Symbol	*next;
} Symbol;


Symbol			*install(), *lookup();



typedef union {
	char		*str;
	Symbol		*sym;
} Datum;

extern Datum		pop();



typedef void		(*Inst)();
#define STOP		((Inst)0)


extern Inst		prog[];
extern Inst		*progp;
extern Inst		*code();

extern void		ifcode(), eq(), ne(), or(), and(), stringpush(),
			tofile(), topipe(), ignore(), exitproc(), varpush(),
			setdeliver(), eval(), defexitproc(), not(), req(),
			prexpr(), assign(), tounixfile();
extern void		adios(), advise();


#endif
