#define	MSRC_Revision		1
#define	MSRC_CMDLINE		0

/*

	zlc.c


*/


/*
	#includes
*/

#include	<stdio.h>
#include	"zl.h"
#include	"zlc.h"


/*
	entry points
*/

ZL_RC zlCopyOutEntryByUnStoring(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	FILE *fpOut
) {
	long lIndex, lLen ;
	int c ;
	ZL_RC zrc ;

	lLen= pzle->hi.dd.sCmprSize ;
	zrc= ZLRC_SUCCESS ;
	for (lIndex= 0L; lIndex< lLen; lIndex++) {
		c= getc(pzlf->fp) ;
		if (c == EOF) {
#if	MSRC_CMDLINE
			perror("co read") ;
#endif
			zrc= ZLRC_EOF ;
			break ;
		}
		if (putc(c, fpOut) == EOF) {
			zrc= ZLRC_IOERROR ;
			break ;
		}
	}
	return(zrc) ;
}

/* end of zlc.c */
