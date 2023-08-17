#define	MSRC_Revision		21
#define	MSRC_Version		0
#define	MSRC_DEBUG		0
#define	MSRC_CMDLINE		0

#define	STATIC

/*

	zl.c			 	ZL C Source
					Copyright (C) 1996, Point Of Sale Ltd.

*/

/*
	#includes
*/

#include	<stdio.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<memory.h>
#include	<string.h>

#include	"zl.h"
#include	"zlc.h"
#include	"zli.h"


/*
	internal data
*/

STATIC struct zl_dcMethod azldm[]= {
	{ PKWDMETH_STORE, zlCopyOutEntryByUnStoring },
 	{ PKWDMETH_DEFLATE, zlCopyOutEntryByInflating },
} ;
#define	N_AZLDM	(sizeof(azldm) / sizeof(azldm[0]))


/*
	internal function decls
*/

STATIC void closeZip(struct zl_file *pzlf) ;
STATIC ZL_RC openZip(struct zl_file *pzlf, const char *pszcZipFileName) ;
STATIC void setEntry(
	struct zl_entry *pzle,
	char *pszEntryFileName,
	char *pszEntryExtraField,
	char *pszEntryFileComment,
	ZL_U4 lMaxFileName,
	ZL_U4 lMaxExtraField,
	ZL_U4 lMaxFileComment
) ;
STATIC ZL_RC readRaw(struct zl_file *pzlf, void *pva, ZL_U4 lLen) ;
STATIC ZL_BOOL processEntry(
	struct zl_file *,
	struct zl_entry *,
	const struct zl_parms *
) ;
STATIC ZL_BOOL processCDir(
	struct zl_file *,
	struct zl_entry *,
	const struct zl_parms *
) ;
STATIC ZL_BOOL processCDirEnd(
	struct zl_file *,
	struct zl_end *,
	const struct zl_parms *
) ;
STATIC ZL_RC skipToStart(struct zl_file *) ;
STATIC ZL_RC getSignature(struct zl_file *, struct zl_sig *) ;
STATIC ZL_RC getLclHdrEntry(struct zl_file *, struct zl_entry *) ;
STATIC ZL_RC getCDirEntry(struct zl_file *, struct zl_entry *) ;
STATIC ZL_RC getCDirEnd(struct zl_file *, struct zl_end *) ;
STATIC ZL_RC copyOutEntry(
	struct zl_file *,
	struct zl_entry *,
	const char *pszEntryFileName
) ;
STATIC ZL_RC seekToFileLoc(
	struct zl_file *,
	ZL_U4 lFilePosition
) ;


/*
	entry points
*/

ZL_RC zlReadZip(
	const char *pszcZipFileName,
	const struct zl_parms *pzlp
) {
	int i ;
	ZL_RC zrc ;
	struct zl_file zlf ;
	struct zl_sig zls ;
	struct zl_entry zle ;
	struct zl_end zlee ;
	char szEntryFileName[ZLMAX_ENTRYFILENAME+1] ;
	char szEntryExtraField[ZLMAX_ENTRYEXTRAFIELD+1] ;
	char szEntryFileComment[ZLMAX_ENTRYFILECOMMENT+1] ;

	zrc= openZip(&zlf, pszcZipFileName) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;

	setEntry(
		&zle,
		szEntryFileName,
		szEntryExtraField,
		szEntryFileComment,
		ZLMAX_ENTRYFILENAME,
		ZLMAX_ENTRYEXTRAFIELD,
		ZLMAX_ENTRYFILECOMMENT
	) ;

	zrc= skipToStart(&zlf) ;
	if (zrc == ZLRC_SUCCESS) {
		zrc= getSignature(&zlf, &zls) ;
	}

	for (i= 0; zrc == ZLRC_SUCCESS; i++) {
		if (zls.hsig.sig != PKWLFHC_SIG) break ;
		zrc= getLclHdrEntry(&zlf, &zle) ;
		if (zrc == ZLRC_SUCCESS) {
			if (!processEntry(&zlf, &zle, pzlp)) break ;
			zrc= getSignature(&zlf, &zls) ;
		}
	}
#if	MSRC_DEBUG && MSRC_CMDLINE
	printf("lfd done: i=%d zrc=%d\n", i, zrc) ;
#endif

	for (i= 0; zrc == ZLRC_SUCCESS; i++) {
		if (zls.hsig.sig != PKWCDHC_SIG) break ;
		zrc= getCDirEntry(&zlf, &zle) ;
		if (zrc == ZLRC_SUCCESS) {
			if (!processCDir(&zlf, &zle, pzlp)) break ;
			zrc= getSignature(&zlf, &zls) ;
		}
	}
#if	MSRC_DEBUG && MSRC_CMDLINE
	printf("cdir done: i=%d zrc=%d\n", i, zrc) ;
#endif

	if (zrc == ZLRC_SUCCESS) {
		if (zls.hsig.sig != PKWCDEND_SIG) {
			zrc= ZLRC_BADFILE ;
		}
	}

	if (zrc == ZLRC_SUCCESS) {
		zrc= getCDirEnd(&zlf, &zlee) ;
		if (zrc == ZLRC_SUCCESS) {
			processCDirEnd(&zlf, &zlee, pzlp) ;
		}
	}

	closeZip(&zlf) ;
	return(zrc) ;
}

ZL_RC zlExtractEntry(
	const char *pszcZipFileName,
	const char *pszEntryFileName,
	ZL_U4 ulEntryLclHdrLoc
) {
	ZL_RC zrc ;
	struct zl_file zlf ;
	struct zl_sig zls ;
	struct zl_entry zle ;
	char szEntryFileName[ZLMAX_ENTRYFILENAME+1] ;
	char szEntryExtraField[ZLMAX_ENTRYEXTRAFIELD+1] ;
	char szEntryFileComment[ZLMAX_ENTRYFILECOMMENT+1] ;

	zrc= openZip(&zlf, pszcZipFileName) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;

	setEntry(
		&zle,
		szEntryFileName,
		szEntryExtraField,
		szEntryFileComment,
		ZLMAX_ENTRYFILENAME,
		ZLMAX_ENTRYEXTRAFIELD,
		ZLMAX_ENTRYFILECOMMENT
	) ;

	zrc= seekToFileLoc(&zlf, ulEntryLclHdrLoc) ;
	if (zrc == ZLRC_SUCCESS) {
		zrc= getSignature(&zlf, &zls) ;
		if (zls.hsig.sig != PKWLFHC_SIG) {
			zrc= ZLRC_BADFILE ;
		}
	}

	if (zrc == ZLRC_SUCCESS) {
		zrc= getLclHdrEntry(&zlf, &zle) ;
		if (zrc == ZLRC_SUCCESS) {
			zrc= copyOutEntry(&zlf, &zle, pszEntryFileName) ;
		}
	}

	closeZip(&zlf) ;
	return(zrc) ;
}

/*
	internal functions
*/

STATIC ZL_RC openZip(struct zl_file *pzlf, const char *pszcZipFileName)
{
	FILE *fp ;

	memset(pzlf, '\0', sizeof(struct zl_file)) ;
	
	if (strlen(pszcZipFileName)> ZLMAX_FILENAMELEN) {
		return(ZLRC_OVERFLOW) ;
	}

	fp= fopen(pszcZipFileName, "rb") ;
	if (!fp) {
#if	MSRC_CMDLINE
		perror("fopen") ;
#endif
		return(ZLRC_NOFILE) ;
	}

	pzlf->fp= fp ;
	strcpy(pzlf->szFileName, pszcZipFileName) ;

	return(ZLRC_SUCCESS) ;
}

STATIC void closeZip(struct zl_file *pzlf)
{
	if (pzlf->fp) {
		fclose(pzlf->fp) ;
		pzlf->fp= (FILE *) 0 ;
	}
}

STATIC void setEntry(
	struct zl_entry *pzle,
	char *pszEntryFileName,
	char *pszEntryExtraField,
	char *pszEntryFileComment,
	ZL_U4 lMaxFileName,
	ZL_U4 lMaxExtraField,
	ZL_U4 lMaxFileComment
) {
	memset(pzle, '\0', sizeof(struct zl_entry)) ;
	pzle->pszFileName= pszEntryFileName ;
	pzle->pszExtraField= pszEntryExtraField ;
	pzle->pszFileComment= pszEntryFileComment ;
	pzle->lMaxFileName= lMaxFileName ;
	pzle->lMaxExtraField= lMaxExtraField ;
	pzle->lMaxFileComment= lMaxFileComment ;
}

STATIC ZL_RC copyOutEntry(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	const char *pszEntryFileName
) {
	FILE *fpOut ;
	ZL_RC zrc ;
	int i ;

	for (i= 0; i< N_AZLDM; i++) {
		if (pzle->hi.mCmprMethod == azldm[i].mCmprMethod) break ;
	}
	if (i>= N_AZLDM) return(ZLRC_NOMETHOD) ;

	fpOut= fopen(pszEntryFileName, "wb") ;
	if (!fpOut) return(ZLRC_CANTWRITE) ;
	zrc= azldm[i].fnUnCompress(pzlf, pzle, fpOut) ;
	fclose(fpOut) ;
	
	return(zrc) ;
}

#define	CALLUSERENTRYFN(x)	       				\
	if (pzlp->x) {		       	       			\
		if (!pzlp->x(pzlf, pzle, pzlp->pUser)) {	\
			return(ZLBOOL_FALSE) ; 			\
		}			       			\
	}

STATIC ZL_BOOL processEntry(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	const struct zl_parms *pzlp
) {
	ZL_U4 ulFilePosition ;

#if	MSRC_DEBUG && MSRC_CMDLINE
	printf(
		"entry: %-20s %2d %-10ld / %-10ld\n",
		pzle->pszFileName,
		pzle->hi.mCmprMethod,
		pzle->hi.dd.sCmprSize,
		pzle->hi.dd.sUncmprSize
	) ;
#endif
	ulFilePosition= (ZL_U4) ftell(pzlf->fp) ;
	CALLUSERENTRYFN(fnProcessEntry) ;
	ulFilePosition+= (ZL_U4) pzle->hi.dd.sCmprSize ;
	return(seekToFileLoc(pzlf, ulFilePosition) == ZLRC_SUCCESS) ;
}

STATIC ZL_BOOL processCDir(
	struct zl_file *pzlf,
	struct zl_entry *pzle,
	const struct zl_parms *pzlp
) {
	ZL_U4 ulFilePosition ;

#if	MSRC_DEBUG && MSRC_CMDLINE
	printf(
		"cdir: %-20s %2d %-10ld / %-10ld  => %ld\n",
		pzle->pszFileName,
		pzle->hi.mCmprMethod,
		pzle->hi.dd.sCmprSize,
		pzle->hi.dd.sUncmprSize,
		pzle->lhdrLoc
	) ;
#endif
	ulFilePosition= (ZL_U4) ftell(pzlf->fp) ;
	CALLUSERENTRYFN(fnProcessCDir) ;
	return(seekToFileLoc(pzlf, ulFilePosition) == ZLRC_SUCCESS) ;
}

STATIC ZL_BOOL processCDirEnd(
	struct zl_file *pzlf,
	struct zl_end *pzlee,
	const struct zl_parms *pzlp
) {
	ZL_U4 ulFilePosition ;
	ZL_U4 lLen ;
	ZL_BOOL bDoDots ;

#if	MSRC_DEBUG && MSRC_CMDLINE
	printf("cdirend: clen=%ld\n", pzlee->lLenComment) ;
#endif
	ulFilePosition= (ZL_U4) ftell(pzlf->fp) ;
	if (pzlp->pszZipFileComment) {
		bDoDots= ZLBOOL_FALSE ;
		lLen= pzlee->lLenComment ;
		if (lLen> pzlp->lMaxLenZipFileComment) {
			lLen= pzlp->lMaxLenZipFileComment - 3 ;
			bDoDots= ZLBOOL_TRUE ;
		}
		readRaw(pzlf, pzlp->pszZipFileComment, lLen) ;
		if (bDoDots) {
			while(lLen< pzlp->lMaxLenZipFileComment) {
				pzlp->pszZipFileComment[lLen++]= '.' ;
			}
		}
		pzlp->pszZipFileComment[lLen++]= '\0' ;
	}
	ulFilePosition+= pzlee->lLenComment ;
	return(seekToFileLoc(pzlf, ulFilePosition) == ZLRC_SUCCESS) ;
}

STATIC ZL_RC readRaw(struct zl_file *pzlf, void *pva, ZL_U4 lLen)
{
	ZL_U4 l ;

	l= (ZL_U4) fread(pva, 1, (size_t) lLen, pzlf->fp) ;
	if (l< lLen) {
#if	MSRC_CMDLINE
		fprintf(stderr, "premature eof\n") ;
#endif
		return(ZLRC_EOF) ;
	}
	return(ZLRC_SUCCESS) ;
}

/*
	It was noted that PK(UN)ZIP can read zipfiles that start with
	a string of binary zeroes and a linefeed.  This routine which
	is called before trying to read from a ZIPfile was created to
	accomodate that case.
*/
STATIC ZL_RC skipToStart(struct zl_file *pzlf)
{
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

STATIC ZL_RC getSignature(struct zl_file *pzlf, struct zl_sig *pzls)
{
	ZL_RC zrc ;

	zrc= readRaw(pzlf, &pzls->hsig, sizeof(pzls->hsig)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	return(ZLRC_SUCCESS) ;
}

STATIC ZL_RC getLclHdrEntry(struct zl_file *pzlf, struct zl_entry *pzle)
{
	ZL_RC zrc ;
	struct pkw_lfhdr lfh ;

	zrc= readRaw(pzlf, &lfh, sizeof(lfh)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	memcpy(&pzle->hi, &lfh.hi, sizeof(pzle->hi)) ;

	if (lfh.hi.fGnlBits & PKWLFHGNLBITS_HASDD) {
#if	MSRC_CMDLINE
		fprintf(stderr, "can't handle this zipfile\n") ;
#endif
		return(ZLRC_CANTHANDLE) ;
	}

	if (lfh.hi.lFileName>= pzle->lMaxFileName) {
#if	MSRC_CMDLINE
		fprintf(stderr, "file name too long\n") ;
#endif
		return(ZLRC_OVERFLOW) ;
	}

	if (lfh.hi.lExtraField>= pzle->lMaxExtraField) {
#if	MSRC_CMDLINE
		fprintf(stderr, "extra field too long\n") ;
#endif
		return(ZLRC_OVERFLOW) ;
	}
	
	zrc= readRaw(pzlf, pzle->pszFileName, lfh.hi.lFileName) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzle->pszFileName[lfh.hi.lFileName]= '\0' ;

	zrc= readRaw(pzlf, pzle->pszExtraField, lfh.hi.lExtraField) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;

	return(ZLRC_SUCCESS) ;
}

STATIC ZL_RC getCDirEntry(struct zl_file *pzlf, struct zl_entry *pzle)
{
	ZL_RC zrc ;
	struct pkw_cdhdr cdh ;

	zrc= readRaw(pzlf, &cdh, sizeof(cdh)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	memcpy(&pzle->hi, &cdh.hi, sizeof(pzle->hi)) ;

	if (cdh.hi.fGnlBits & PKWLFHGNLBITS_HASDD) {
#if	MSRC_CMDLINE
		fprintf(stderr, "can't handle this zipfile\n") ;
#endif
		return(ZLRC_CANTHANDLE) ;
	}

	if (cdh.hi.lFileName> pzle->lMaxFileName) {
#if	MSRC_CMDLINE
		fprintf(stderr, "file name too long\n") ;
#endif
		return(ZLRC_OVERFLOW) ;
	}

	if (cdh.hi.lExtraField> pzle->lMaxExtraField) {
#if	MSRC_CMDLINE
		fprintf(stderr, "extra field too long\n") ;
#endif
		return(ZLRC_OVERFLOW) ;
	}
	
	if (cdh.lFileComment> pzle->lMaxFileComment) {
#if	MSRC_CMDLINE
		fprintf(stderr, "file comment too long\n") ;
#endif
		return(ZLRC_OVERFLOW) ;
	}
	
	zrc= readRaw(pzlf, pzle->pszFileName, cdh.hi.lFileName) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzle->pszFileName[cdh.hi.lFileName]= '\0' ;

	zrc= readRaw(pzlf, pzle->pszExtraField, cdh.hi.lExtraField) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzle->pszExtraField[cdh.hi.lExtraField]= '\0' ;

	zrc= readRaw(pzlf, pzle->pszFileComment, cdh.lFileComment) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzle->pszFileComment[cdh.lFileComment]= '\0' ;

	pzle->lhdrLoc= cdh.roLclHdrOffset ;

	return(ZLRC_SUCCESS) ;
}

STATIC ZL_RC getCDirEnd(struct zl_file *pzlf, struct zl_end *pzlee)
{
	ZL_RC zrc ;
	struct pkw_cdend cde ;

	zrc= readRaw(pzlf, &cde, sizeof(cde)) ;
	if (zrc != ZLRC_SUCCESS) return(zrc) ;
	pzlee->lLenComment= cde.lFileComment ;
	return(ZLRC_SUCCESS) ;
} ;


STATIC ZL_RC seekToFileLoc(
	struct zl_file *pzlf,
	ZL_U4 lFilePosition
) {
	if (fseek(pzlf->fp, lFilePosition, SEEK_SET) != 0) {
		return(ZLRC_IOERROR) ;
	}
	return(ZLRC_SUCCESS) ;
}

/* end of zl.c */
