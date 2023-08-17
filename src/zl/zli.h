/*	Revision:		2
*/

/*

	zli.h

*/

#ifndef ZLI_H_INC
#define	ZLI_H_INC


/*
	sub-#includes
*/
#include	"zl.h"


/*
	API entry points
*/

ZL_RC zlCopyOutEntryByInflating(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	FILE *fpOut
) ;


/*
	private entry points
*/
void zliFlush(int n, void *pData) ;
int zliGetChar() ;


/* #ifndef ZLI_H_INC */
#endif

/* end of zli.h */
