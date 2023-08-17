/*	Revision:		2
*/

#ifndef PKW_H_INC
#define	PKW_H_INC

/*

	pkw.h - PKW C Header: PKWARE values/strucs
    Copyright (C) 1996, Point Of Sale Ltd.

*/

/*
	#defines
*/


#define	PKWUINT4		unsigned long
#define	PKWUINT2		unsigned short

#define	PKW_DMETH		PKWUINT2
#define	PKWDMETH_STORE		0
#define	PKWDMETH_DEFLATE	8

#define	PKWLFHGNLBITS_HASDD	8		/* data descriptor present */

#define	PKWLFHC_SIG		0x04034b50L
#define	PKWCDHC_SIG		0x02014b50L
#define	PKWCDEND_SIG		0x06054b50L


/*
	PKWARE structures
*/

struct pkw0_dd {
	PKWUINT4 crc32 ;	/* crc-32			*/
	PKWUINT4 sCmprSize ;	/* compressed size	  	*/
	PKWUINT4 sUncmprSize ;	/* uncompressed size		*/
} ;

struct pkw0_hinfo {
	PKWUINT2 vReq ;		/* version needed to extract	*/
	PKWUINT2 fGnlBits ;	/* general purpose bit flag     */
	PKWUINT2 mCmprMethod ;	/* compression method		*/
	PKWUINT2 tModTime ;	/* last mod file time           */
	PKWUINT2 dModDate ;	/* last mod file date		*/
	struct pkw0_dd dd ;	/* initial data descriptor	*/
	PKWUINT2 lFileName ;	/* filename length		*/
	PKWUINT2 lExtraField ;	/* extra field length		*/
} ;

struct pkw_ddesc {
	struct pkw0_dd dd ;	/* data descriptor		*/
} ;

struct pkw_sighdr {
	PKWUINT4 sig ;		/* signature			*/
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
	PKWUINT2 vMade ;	/* version made by		*/
	struct pkw0_hinfo hi ;	/* standard header info		*/
	PKWUINT2 lFileComment ;	/* file comment length		*/
	PKWUINT2 nDiskStart ;	/* disk number start		*/
	PKWUINT2 a2IntAttr ;	/* internal file attributes	*/
	PKWUINT4 a4ExtAttr ;	/* external file attributes	*/
	PKWUINT4 roLclHdrOffset ;/* relative offset of lcl hdr	*/
	/* followed by:
		filename (variable size)
		extra field (variable size)
		file comment (variable size)
	*/
} ;

struct pkw_cdend {		/* signature  (0x06054b50)	*/
	/* prefixed by: "struct pkw_sighdr" */
	PKWUINT2 nDisk ;	/* number of this disk		*/
	PKWUINT2 nDiskStart ;	/* # of disk w/start of cd	*/
	PKWUINT2 nDiskCDEnt ;	/* # of entries in cd this disk	*/
	PKWUINT2 nTotCDEnt ;	/* # of entries in cd total	*/
	PKWUINT4 sizeCD ;	/* size of cd			*/
	PKWUINT4 ofsCD ;	/* offset to cd wrt start disk	*/
	PKWUINT2 lFileComment ;	/* zipfile comment length	*/
	/* followed by:
		zipfile comment (variable size)
	*/
} ;


/* #ifndef PKW_H_INC */
#endif

/* end of pkw.h */
