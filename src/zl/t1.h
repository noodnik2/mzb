/*	Revision:		1
*/

/*

	t1.h			T1 C Header: Main Module
					Copyright (C) 1996, Point Of Sale Ltd.

*/

#ifndef	T1_H_INC
#define	T1_H_INC

/*
	sub-#includes
*/
#include	<stdio.h>

/*
	#defines
*/

#define	T1_E			int
#define	T1E_SUCCESS		0
#define	T1E_HELP     		1
#define	T1E_BADARGS  		12
#define	T1E_FAILURE  		99

#define	T1_BOOL		int
#define	T1BOOL_FALSE		0
#define	T1BOOL_TRUE		(!T1BOOL_FALSE)


/*
	structures, typedefs
*/

struct t1_parms {
	T1_BOOL bHelp ;
	T1_BOOL bQuiet ;
	int iDebugLevel ;
} ;


/*
	function decls
*/

void main(int argc, char **argv) ;


/* #ifndef T1_H_INC */
#endif

/* end of t1.h */
