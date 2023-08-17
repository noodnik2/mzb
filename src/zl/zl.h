/*	Revision:		12
*/

#ifndef ZL_H_INC
#define	ZL_H_INC

/*

	zl.h			 	ZL C Header
					Copyright (C) 1996, Point Of Sale Ltd.

*/

/*
	sub-#includes
*/

#include	<stdio.h>
#include	"pkw.h"


/*
	#defines
*/

#define	ZL_RC			int
#define	ZLRC_SUCCESS		0
#define	ZLRC_OVERFLOW		1
#define	ZLRC_CANTHANDLE		2
#define	ZLRC_BADFILE		11
#define	ZLRC_NOFILE		12
#define	ZLRC_NOMETHOD		13
#define	ZLRC_EOF		36
#define	ZLRC_IOERROR		37
#define	ZLRC_CANTWRITE		38
#define	ZLRC_NOTIMPLEMENTED	98
#define	ZLRC_FAILURE		99

#define	ZL_BOOL			int
#define	ZLBOOL_FALSE		0
#define	ZLBOOL_TRUE		(!ZLBOOL_FALSE)

#define	ZLMAX_FILENAMELEN	255
#define	ZLMAX_EMSGSIZE 		255
#define	ZLMAX_ENTRYEXTRAFIELD	255
#define	ZLMAX_ENTRYFILENAME 	255
#define	ZLMAX_ENTRYFILECOMMENT	255

#define	ZL_U4			unsigned long


/*
	structs
*/

struct zl_file {
	FILE *fp ;
	char szFileName[ZLMAX_FILENAMELEN+1] ;
	PKWUINT4 fStateFlags ;
} ;

struct zl_sig {
	struct pkw_sighdr hsig ;
} ;

struct zl_entry {
	char *pszFileName ;
	char *pszExtraField ;
	char *pszFileComment ;
	ZL_U4 lMaxFileName ;
	ZL_U4 lMaxExtraField ;
	ZL_U4 lMaxFileComment ;
	ZL_U4 lhdrLoc ;		/* location of local header */
	struct pkw0_hinfo hi ;
} ;

struct zl_end {
	ZL_U4 lLenComment ;
} ;

typedef ZL_BOOL (*zlFnProcessEntry_t)(
	const struct zl_file *,
	const struct zl_entry *,
	void *pData
) ;

struct zl_parms {
	ZL_U4 lMaxLenZipFileComment ;		/* size of user buffer	*/
	char *pszZipFileComment ;		/* => user buffer	*/
	void *pUser ;				/* => user data		*/
	zlFnProcessEntry_t fnProcessEntry ;	/* => user fn for entry	*/
	zlFnProcessEntry_t fnProcessCDir ;	/* => user fn for CDir	*/
} ;	

typedef ZL_RC (*zlFnUnCompress_t)(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	FILE *fpOut
) ;

struct zl_dcMethod {
	PKW_DMETH mCmprMethod ;
	zlFnUnCompress_t fnUnCompress ;
} ;


/*
	API functions
*/

ZL_RC zlReadZip(const char *pszcZipFileName, const struct zl_parms *pzlp) ;

ZL_RC zlExtractEntry(
	const char *pszcZipFileName,
	const char *pszEntryFileName,
	ZL_U4 ulEntryLclHdrLoc
) ;


/* #ifndef ZL_H_INC */
#endif

/* end of zl.h */
