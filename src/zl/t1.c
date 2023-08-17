#define	MSRC_Revision		25
#define	MSRC_Version		0

/*

	t1.c			 	T1 C Source: Main Module
					Copyright (C) 1996, Point Of Sale Ltd.

*/

/*
	#includes
*/

#include	<stdio.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	"t1.h"


/*
	local #defines
*/

#define	ZL_RC			int
#define	ZLRC_SUCCESS		0
#define	ZLRC_OVERFLOW		1
#define	ZLRC_CANTHANDLE		2
#define	ZLRC_BADFILE		11
#define	ZLRC_NOFILE		12
#define	ZLRC_IOERROR		24
#define	ZLRC_EOF		36
#define	ZLRC_NOTIMPLEMENTED	98
#define	ZLRC_FAILURE		99

#define	ZLUINT4			unsigned long
#define	ZLUINT2			unsigned short

#define	ZLMAX_FILENAMELEN	255

#define	PKW_DMETH		int
#define	PKWDMETH_STORE		0
#define	PKWDMETH_DEFLATE	8

#define	PKWLFHGNLBITS_HASDD	8		/* data descriptor present */

#define	PKWLFHC_SIG		0x04034b50L
#define	PKWCDHC_SIG		0x02014b50L
#define	PKWCDEND_SIG		0x06054b50L

#define	T1VER_MINOR(x) 		((x) % 100)
#define	T1VER_MAJOR(x) 		((x) / 100)
#define	T1MAX_EMSGSIZE 		255

#define	T1MAX_ENTRYEXTRAFIELD	255
#define	T1MAX_ENTRYFILENAME 	255
#define	T1MAX_ENTRYFILECOMMENT	255


/*
	local structs, types
*/


/*
	PKWARE structures
*/

struct pkw0_dd {
	ZLUINT4 crc32 ;		/* crc-32			*/
	ZLUINT4 sCmprSize ;	/* compressed size	  	*/
	ZLUINT4 sUncmprSize ;	/* uncompressed size		*/
} ;

struct pkw0_hinfo {
	ZLUINT2 vReq ;		/* version needed to extract	*/
	ZLUINT2 fGnlBits ;	/* general purpose bit flag     */
	ZLUINT2 mCmprMethod ;	/* compression method		*/
	ZLUINT2 tModTime ;	/* last mod file time           */
	ZLUINT2 dModDate ;	/* last mod file date		*/
	struct pkw0_dd dd ;	/* initial data descriptor	*/
	ZLUINT2 lFileName ;	/* filename length		*/
	ZLUINT2 lExtraField ;	/* extra field length		*/
} ;

struct pkw_ddesc {
	struct pkw0_dd dd ;	/* data descriptor		*/
} ;

struct pkw_sighdr {
	ZLUINT4 sig ;		/* signature			*/
} ;

struct pkw_lfhdr {		/* signature  (0x04034b50)	*/
	/* prefixed by: "struct pkw_sighdr" */
	struct pkw0_hinfo hi ;	/* standard header info		*/
	/* followed by:
		filename (variable size)
		extra field (variable size)
	*/
} ;

struct pkw_cdhdr {   		/* signature  (0x02014b50)	*/
	/* prefixed by: "struct pkw_sighdr" */
	ZLUINT2 vMade ;		/* version made by		*/
	struct pkw0_hinfo hi ;	/* standard header info		*/
	ZLUINT2 lFileComment ;	/* file comment length		*/
	ZLUINT2 nDiskStart ;	/* disk number start		*/
	ZLUINT2 a2IntAttr ;	/* internal file attributes	*/
	ZLUINT4 a4ExtAttr ;	/* external file attributes	*/
	ZLUINT4 roLclHdrOffset ;/* relative offset of lcl hdr	*/
	/* followed by:
		filename (variable size)
		extra field (variable size)
		file comment (variable size)
	*/
} ;

struct pkw_cdend {		/* signature  (0x06054b50)	*/
	/* prefixed by: "struct pkw_sighdr" */
	ZLUINT2 nDisk ;		/* number of this disk		*/
	ZLUINT2 nDiskStart ;	/* # of disk w/start of cd	*/
	ZLUINT2 nDiskCDEnt ;	/* # of entries in cd this disk	*/
	ZLUINT2 nTotCDEnt ;	/* # of entries in cd total	*/
	ZLUINT4 sizeCD ;	/* size of cd			*/
	ZLUINT4 ofsCD ;		/* offset to cd wrt start disk	*/
	ZLUINT2 lFileComment ;	/* zipfile comment length	*/
	/* followed by:
		zipfile comment (variable size)
	*/
} ;


/*
	application structures
*/

struct zl_file {
	FILE *fp ;
	char szFileName[ZLMAX_FILENAMELEN+1] ;
	ZLUINT4 fStateFlags ;
} ;

struct zl_sig {
	struct pkw_sighdr hsig ;
} ;

struct zl_entry {
	char *pszFileName ;
	char *pszExtraField ;
	char *pszFileComment ;
	int lMaxFileName ;
	int lMaxExtraField ;
	int lMaxFileComment ;
	long lhdrLoc ;			/* location of local header */
	struct pkw0_hinfo hi ;
} ;

typedef ZL_RC (*zlDecoderFn_t)(struct zl_file *, struct zl_entry *) ;

struct zl_decoder {
	PKW_DMETH dm ;
	zlDecoderFn_t pFn ;
} ;


/*
	internal function decls
*/

static void zlClose(struct zl_file *pzlf) ;
static ZL_RC zlOpen(struct zl_file *pzlf, const char *pszcZipFileName) ;
static void zlSetEntry(
	struct zl_entry *pzle,
	char *pszEntryFileName,
	char *pszEntryExtraField,
	char *pszEntryFileComment,
	int lMaxFileName,
	int lMaxExtraField,
	int lMaxFileComment
) ;
static void readit(const char *pszcZipFileName) ;
static ZL_RC readRaw(struct zl_file *pzlf, void *pva, int iLen) ;
static int processEntry(struct zl_file *pzlf, struct zl_entry *pzle) ;
static int processCDir(struct zl_file *pzlf, struct zl_entry *pzle) ;
static ZL_RC zlSkipToStart(struct zl_file *pzlf) ;
static ZL_RC zlGetSignature(struct zl_file *pzlf, struct zl_sig *pzls) ;
static ZL_RC zlGetEntry(struct zl_file *pzlf, struct zl_entry *pzle) ;
static ZL_RC zlGetCDir(struct zl_file *pzlf, struct zl_entry *pzle) ;
static int copyOut(FILE *fp, long lLen, char *pszName, FILE *fpOut) ;
static void init(int argc, char *argv[]) ;
static void signon() ;
static void syntax() ;
static void badargs(char *pszMessage, char *pszArg) ;
static void zrcassert_ok(ZL_RC zrc, const char *pszcWhat) ;
static ZL_RC zlUnStoreFn(struct zl_file *pzlf, struct zl_entry *pzle) ;
static ZL_RC zlUnDeflateFn(struct zl_file *pzlf, struct zl_entry *pzle) ;


/*
	internal data
*/


static struct zl_decoder zlda[]= {
	{	PKWDMETH_STORE,		zlUnStoreFn	},
	{	PKWDMETH_DEFLATE,	zlUnDeflateFn	},
} ;
#define	N_ZLDA	(sizeof(zlda) / sizeof(zlda[0]))


/*
	public data
*/

struct t1_parms parms ;


/*
	entry points
*/

void main(int argc, char *argv[])
{
	init(argc, argv) ;
	readit("t1.zip") ;
	exit(T1E_SUCCESS) ;
	/*NOTREACHED*/
}


/*
	internal functions
*/

static ZL_RC zlUnDeflateFn(struct zl_file *pzlf, struct zl_entry *pzle)
{
	return(ZLRC_NOTIMPLEMENTED) ;
}

static ZL_RC zlUnStoreFn(struct zl_file *pzlf, struct zl_entry *pzle)
{
	return(ZLRC_NOTIMPLEMENTED) ;
}

static void zlClose(struct zl_file *pzlf)
{
	if (pzlf->fp) {
		fclose(pzlf->fp) ;
		pzlf->fp= (FILE *) 0 ;
	}
}

static ZL_RC zlOpen(struct zl_file *pzlf, const char *pszcZipFileName)
{
	FILE *fp ;

	memset(pzlf, '\0', sizeof(struct zl_file)) ;
	
	if (strlen(pszcZipFileName)> ZLMAX_FILENAMELEN) {
		return(ZLRC_OVERFLOW) ;
	}

	fp= fopen(pszcZipFileName, "rb") ;
	if (!fp) {
		perror("fopen") ;
		return(ZLRC_NOFILE) ;
	}

	pzlf->fp= fp ;
	strcpy(pzlf->szFileName, pszcZipFileName) ;

	return(ZLRC_SUCCESS) ;
}

static void zlSetEntry(
	struct zl_entry *pzle,
	char *pszEntryFileName,
	char *pszEntryExtraField,
	char *pszEntryFileComment,
	int lMaxFileName,
	int lMaxExtraField,
	int lMaxFileComment
) {
	memset(pzle, '\0', sizeof(struct zl_entry)) ;
	pzle->pszFileName= pszEntryFileName ;
	pzle->pszExtraField= pszEntryExtraField ;
	pzle->pszFileComment= pszEntryFileComment ;
	pzle->lMaxFileName= lMaxFileName ;
	pzle->lMaxExtraField= lMaxExtraField ;
	pzle->lMaxFileComment= lMaxFileComment ;
}

static void readit(const char *pszcZipFileName)
{
	int i ;
	ZL_RC zrc ;
	struct zl_file zlf ;
	struct zl_sig zls ;
	struct zl_entry zle ;
	char szEntryFileName[T1MAX_ENTRYFILENAME+1] ;
	char szEntryExtraField[T1MAX_ENTRYEXTRAFIELD+1] ;
	char szEntryFileComment[T1MAX_ENTRYFILECOMMENT+1] ;

	zrc= zlOpen(&zlf, pszcZipFileName) ;
	zrcassert_ok(zrc, "open") ;

	zlSetEntry(
		&zle,
		szEntryFileName,
		szEntryExtraField,
		szEntryFileComment,
		T1MAX_ENTRYFILENAME,
		T1MAX_ENTRYEXTRAFIELD,
		T1MAX_ENTRYFILECOMMENT
	) ;

	zrc= zlSkipToStart(&zlf) ;
	if (zrc == ZLRC_SUCCESS) {
		zrc= zlGetSignature(&zlf, &zls) ;
	}

	for (i= 0; zrc == ZLRC_SUCCESS; i++) {
		if (zls.hsig.sig != PKWLFHC_SIG) break ;
		zrc= zlGetEntry(&zlf, &zle) ;
		if (zrc == ZLRC_SUCCESS) {
			if (processEntry(&zlf, &zle)) break ;
			zrc= zlGetSignature(&zlf, &zls) ;
		}
	}
	printf("lfd done: i=%d zrc=%d\n", i, zrc) ;

	for (i= 0; zrc == ZLRC_SUCCESS; i++) {
		if (zls.hsig.sig != PKWCDHC_SIG) break ;
		zrc= zlGetCDir(&zlf, &zle) ;
		if (zrc == ZLRC_SUCCESS) {
			if (processCDir(&zlf, &zle)) break ;
			zrc= zlGetSignature(&zlf, &zls) ;
		}
	}
	printf("cdir done: i=%d zrc=%d\n", i, zrc) ;

	zlClose(&zlf) ;
}

static ZL_RC readRaw(struct zl_file *pzlf, void *pva, int iLen)
{
	int l ;

	l= fread(pva, 1, iLen, pzlf->fp) ;
	if (l< 0) {
		perror("read") ;
		return(ZLRC_IOERROR) ;
	}

	if (l< iLen) {
		fprintf(stderr, "premature eof\n") ;
		return(ZLRC_EOF) ;
	}

	return(ZLRC_SUCCESS) ;
}

static int processCDir(struct zl_file *pzlf, struct zl_entry *pzle)
{
	printf(
		"cdir: %-20s %2d %-10ld / %-10ld  => %ld\n",
		pzle->pszFileName,
		pzle->hi.mCmprMethod,
		pzle->hi.dd.sCmprSize,
		pzle->hi.dd.sUncmprSize,
		pzle->lhdrLoc
	) ;

	return(0) ;
}

static int processEntry(struct zl_file *pzlf, struct zl_entry *pzle)
{
	printf(
		"entry: %-20s %2d %-10ld / %-10ld\n",
		pzle->pszFileName,
		pzle->hi.mCmprMethod,
		pzle->hi.dd.sCmprSize,
		pzle->hi.dd.sUncmprSize
	) ;

	if (copyOut(pzlf->fp, pzle->hi.dd.sCmprSize, "data", NULL)) {
		return(1) ;
	}

	return(0) ;
}

static ZL_RC zlSkipToStart(struct zl_file *pzlf)
{
	ZL_RC zrc ;
	int c ;

	while(/*CONSTCOND*/1) {
		c= getc(pzlf->fp) ;
		if (c == EOF) return(ZLRC_EOF) ;
		if (c != 0) {
			if (c>= ' ') ungetc(c, pzlf->fp) ;
			break ;
		}
	}
	return(ZLRC_SUCCESS) ;
}

static ZL_RC zlGetSignature(struct zl_file *pzlf, struct zl_sig *pzls)
{
	ZL_RC zrc ;

	zrc= readRaw(pzlf, &pzls->hsig, sizeof(pzls->hsig)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	return(ZLRC_SUCCESS) ;
}

static ZL_RC zlGetCDir(struct zl_file *pzlf, struct zl_entry *pzle)
{
	ZL_RC zrc ;
	struct pkw_cdhdr cdh ;

	zrc= readRaw(pzlf, &cdh, sizeof(cdh)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	memcpy(&pzle->hi, &cdh.hi, sizeof(pzle->hi)) ;

	if (cdh.hi.fGnlBits & PKWLFHGNLBITS_HASDD) {
		fprintf(stderr, "can't handle this zipfile\n") ;
		return(ZLRC_CANTHANDLE) ;
	}

	if (cdh.hi.lFileName> pzle->lMaxFileName) {
		fprintf(stderr, "file name too long\n") ;
		return(ZLRC_OVERFLOW) ;
	}

	if (cdh.hi.lExtraField> pzle->lMaxExtraField) {
		fprintf(stderr, "extra field too long\n") ;
		return(ZLRC_OVERFLOW) ;
	}
	
	zrc= readRaw(pzlf, pzle->pszFileName, cdh.hi.lFileName) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzle->pszFileName[cdh.hi.lFileName]= '\0' ;

	zrc= readRaw(pzlf, pzle->pszExtraField, cdh.hi.lExtraField) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;

	zrc= readRaw(pzlf, pzle->pszFileComment, cdh.lFileComment) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;

	pzle->lhdrLoc= cdh.roLclHdrOffset ;

	return(ZLRC_SUCCESS) ;
}

static ZL_RC zlGetEntry(struct zl_file *pzlf, struct zl_entry *pzle)
{
	ZL_RC zrc ;
	struct pkw_lfhdr lfh ;

	zrc= readRaw(pzlf, &lfh, sizeof(lfh)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	memcpy(&pzle->hi, &lfh.hi, sizeof(pzle->hi)) ;

	if (lfh.hi.fGnlBits & PKWLFHGNLBITS_HASDD) {
		fprintf(stderr, "can't handle this zipfile\n") ;
		return(ZLRC_CANTHANDLE) ;
	}

	if (lfh.hi.lFileName> pzle->lMaxFileName) {
		fprintf(stderr, "file name too long\n") ;
		return(ZLRC_OVERFLOW) ;
	}

	if (lfh.hi.lExtraField> pzle->lMaxExtraField) {
		fprintf(stderr, "extra field too long\n") ;
		return(ZLRC_OVERFLOW) ;
	}
	
	zrc= readRaw(pzlf, pzle->pszFileName, lfh.hi.lFileName) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzle->pszFileName[lfh.hi.lFileName]= '\0' ;

	zrc= readRaw(pzlf, pzle->pszExtraField, lfh.hi.lExtraField) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;

	return(ZLRC_SUCCESS) ;
}

static int copyOut(FILE *fp, long lLen, char *pszName, FILE *fpOut)
{
	long lIndex ;
	int c ;

	if (fpOut) fprintf(fpOut, "%s: ", pszName) ;
	for (lIndex= 0L; lIndex< lLen; lIndex++) {
		c= getc(fp) ;
		if (c == EOF) {
			perror("co read") ;
			return(1) ;
		}
		if (fpOut) {
			if ((c< 32) || (c> 126)) {
				fprintf(fpOut, "\\%03o", c) ;
			}
			else {
				putc(c, fpOut) ;
			}
		}
	}
	if (fpOut) putc('\n', fpOut) ;
	return(0) ;
}

static void init(int argc, char *argv[])
{
	int iOpt ;

	memset(&parms, '\0', sizeof(struct t1_parms)) ;

	if (!parms.bQuiet) {
		signon() ;
	}

	if (parms.bHelp) {
		syntax() ;
		exit(T1E_HELP) ;
		/*NOTREACHED*/
	}

}

static void signon()
{
	printf(
		"t1 v%d.%d.%d\n",
		MSRC_Version,
		T1VER_MAJOR(MSRC_Revision),
		T1VER_MINOR(MSRC_Revision)
	) ;
	printf("Copyright (C) 1996, Point Of Sale Ltd.\n") ;
	printf("\n") ;
}

static void syntax()
{
	static char *pText[]= {
		"syntax:",
		"\tt1 [options]",
		"",
		"options:",
		"\t-h\tprints this help text",
		"\t-q\toperates quietly",
		"\t-z\tincrease general debug level"
	} ;
#define	NTEXT	(sizeof(pText) / sizeof(pText[0]))
	int i ;

	for (i= 0; i< NTEXT; i++) {
		printf("%s\n", pText[i]) ;
	}
}

static void badargs(char *pszMessage, char *pszArg)
{
	char szEMessage[T1MAX_EMSGSIZE+1] ;

	if (pszMessage) {
		strcpy(szEMessage, pszMessage) ;
		if (pszArg) {
			strcat(szEMessage, ": ") ;
			strcat(szEMessage, pszArg) ;
		}
		clerror(szEMessage) ;
	}
	clerror("(use 'h' option for help)\n") ;
	exit(T1E_BADARGS) ;
	/*NOTREACHED*/
}


static void zrcassert_ok(ZL_RC zrc, const char *pszcWhat)
{
	if (zrc == ZLRC_SUCCESS) return ;
	fprintf(stderr, "fatal error: %s: zrc=%d\n", pszcWhat, zrc) ;
	exit(1) ;
	/*NOTREACHED*/
}

/* end of t1.c */
