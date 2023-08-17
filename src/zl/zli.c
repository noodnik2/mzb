#define	MSRC_Revision		6
#define	MSRC_CMDLINE		0

/*

	zli.c


*/


/*
	#includes
*/

#include	<stdio.h>
#include	"zli.h"
#include	"inflate.h"


/*
	semi-public data (for "inflate.c")
*/

union work area ;
int qflag= !MSRC_CMDLINE ;


/*
	internal data
*/

static FILE *gfpIn ;
static FILE *gfpOut ;
static long glIndex, glLen ;
static ZL_BOOL gbWriteError ;


/*
	entry points
*/

ZL_RC zlCopyOutEntryByInflating(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	FILE *fpOut
) {
	ZL_RC zlrc ;

	glLen= pzle->hi.dd.sCmprSize ;
	glIndex= 0L ;
	gfpIn= pzlf->fp ;
	gfpOut= fpOut ;
	gbWriteError= ZLBOOL_FALSE ;

	zlrc= ZLRC_SUCCESS ;

	if (inflate()) zlrc= ZLRC_BADFILE ;
	inflate_free() ;

	if (zlrc == ZLRC_SUCCESS) {
		if (gbWriteError) {
			zlrc= ZLRC_IOERROR ;
		}
	}

	return(zlrc) ;
}


/*
	semi-public functions, for "inflate.c"
*/

int zliGetChar()
{
	glIndex++ ;
	if (glIndex> glLen) return(EOF) ;
	return(getc(gfpIn)) ;
}

void zliFlush(int n, void *pData)
{
	if (gbWriteError) return ;
	if (fwrite(pData, 1, n, gfpOut)< (size_t) n) {
		gbWriteError= ZLBOOL_TRUE ;
	}
}

/* end of zli.c */
