#define	MSRC_Revision		5
#define	MSRC_DEBUG		0

/*
	
*/

#include	"zl.h"


/*
	local #defines
*/

#define	T2MAX_FILENAMELEN	32


/*
	local structs
*/

struct t2_entry {
	unsigned long ulLclHdrPos ;
	char szEntryName[T2MAX_FILENAMELEN+1] ;
} ;


/*
	local function decls
*/

struct t2_parms {
	char szZipFileName[T2MAX_FILENAMELEN+1] ;
	int iNumEntries ;
	int iEntryIdx ;
	struct t2_entry *pt2e ;
} ;

static ZL_BOOL myProcessEntry(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) ;

static ZL_BOOL myProcessCDir(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) ;

static void fillInEntry(
	struct t2_entry *pt2e,
	const struct zl_entry *pzle
) ;

static void extractEntries(struct t2_parms *pt2p) ;


/*
	internal data
*/

static struct t2_parms t2p ;


/*
	main
*/

void main()
{
	struct zl_parms zlp ;
	ZL_RC zlrc ;

	memset(&t2p, '\0', sizeof(t2p)) ;
	strcpy(t2p.szZipFileName, "t2.zip") ;

	memset(&zlp, '\0', sizeof(zlp)) ;
	zlp.fnProcessEntry= myProcessEntry ;
	zlp.fnProcessCDir= myProcessCDir ;
	zlp.pUser= &t2p ;
	
	zlrc= zlReadZip(t2p.szZipFileName, &zlp) ;
#if MSRC_DEBUG
	printf("R(%d); zlReadZip\n", zlrc) ;
#endif

	if (zlrc == ZLRC_SUCCESS) {
		extractEntries(&t2p) ;
	}

	if (t2p.pt2e) free(t2p.pt2e) ;
	exit(0) ;
}


/*
	internal functions
*/

static void extractEntries(struct t2_parms *pt2p)
{
	int i, l ;
	ZL_RC zrc ;

	printf("There are %d entries\n", pt2p->iNumEntries) ;
	l= (pt2p->iNumEntries) ;
	for (i= 0; i< l; i++) {
		printf(
			"Extracting: loc=%06ld entry='%s' --> %s ",
			pt2p->pt2e[i].ulLclHdrPos,
			pt2p->pt2e[i].szEntryName,
			pt2p->pt2e[i].szEntryName
		) ;
		zrc= zlExtractEntry(
			pt2p->szZipFileName,
			pt2p->pt2e[i].szEntryName,
			pt2p->pt2e[i].ulLclHdrPos
		) ;
		printf(" R(%d);\n", zrc) ;
	}
}

static ZL_BOOL myProcessEntry(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) {
	struct t2_parms *pt2p= (struct t2_parms *) pUser ;
#if	MSRC_DEBUG
	printf("in myProcessEntry\n") ;
#endif
	pt2p->iNumEntries++ ;
	return(ZLBOOL_TRUE) ;
}

static ZL_BOOL myProcessCDir(
	const struct zl_file *pzlf,
	const struct zl_entry *pzle,
	void *pUser
) {
	struct t2_parms *pt2p= (struct t2_parms *) pUser ;
	struct t2_entry *pt2e= pt2p->pt2e ;

#if	MSRC_DEBUG
	printf("in myProcessCDir\n") ;
#endif

	if (!pt2e) {
		pt2e= (struct t2_entry *) malloc(
			pt2p->iNumEntries * sizeof(struct t2_entry)
		) ;
		if (!pt2e) return(ZLBOOL_FALSE) ;
		pt2p->pt2e= pt2e ;
		pt2p->iEntryIdx= 0 ;
	}

	fillInEntry(pt2e + pt2p->iEntryIdx++, pzle) ;
	return(ZLBOOL_TRUE) ;
}

static void fillInEntry(
	struct t2_entry *pt2e,
	const struct zl_entry *pzle
) {
	int iLenFileName, iLenEntryName ;

	pt2e->ulLclHdrPos= (unsigned long) pzle->lhdrLoc ;
	iLenFileName= strlen(pzle->pszFileName) ;
	if (iLenFileName<= T2MAX_FILENAMELEN) {
		memcpy(pt2e->szEntryName, pzle->pszFileName, iLenFileName) ;
		pt2e->szEntryName[iLenFileName]= '\0' ;
	}
	else {
		memset(pt2e->szEntryName, "...", 3) ;
		iLenEntryName= T2MAX_FILENAMELEN - 3 ;
		memset(
			pt2e->szEntryName + 3,
			pzle->pszFileName + iLenFileName - iLenEntryName,
			iLenEntryName
		) ;
		pt2e->szEntryName[3+iLenEntryName]= '\0' ;
	}
}	

/* end of t2.c */
