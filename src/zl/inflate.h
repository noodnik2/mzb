/*

	inflate.h

	hacked version of inflate.h by Marty Ross
	see "inflate0.h" for original version.
*/

#ifndef INFLATE_H_INC   /* prevent multiple inclusions */
#define INFLATE_H_INC


/*
	sub-#includes
*/

#include	<malloc.h>
#include	<memory.h>


/************/
/*  Macros  */
/************/

#ifndef MAX
#  define MAX(a,b)   ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)   ((a) < (b) ? (a) : (b))
#endif

#define OF(a) a
#define Trace(x)

#define	memzero(dest,len)	memset(dest, '\0', len)

#define slide  area.Slide

#define MAX_BITS      13                 /* used in old unshrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */
#ifndef WSIZE
#  define WSIZE     0x8000  /* window size--must be a power of two, and */
#endif                      /*  at least 32K for zip's deflate method */

/**************/
/*  Typedefs  */
/**************/

typedef char              boolean;
typedef unsigned char     uch;  /* code assumes unsigned bytes; these type-  */
typedef unsigned short    ush;  /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long     ulg;  /*  predefined on some systems) & match zip  */


/*---------------------------------------------------------------------------
    Zipfile work area declarations.
  ---------------------------------------------------------------------------*/

#ifdef MALLOC_WORK

   union work {
     struct {
       short *Prefix_of;            /* (8193 * sizeof(short)) */
       uch *Suffix_of;
       uch *Stack;
     } shrink;                      /* unshrink() */
     uch *Slide;                    /* explode(), inflate(), unreduce() */
   };
#  define prefix_of  area.shrink.Prefix_of
#  define suffix_of  area.shrink.Suffix_of
#  define stack      area.shrink.Stack

#else /* !MALLOC_WORK */

#  ifdef NEW_UNSHRINK   /* weird segmentation violations if union NODE array */
     union work {
       uch Stack[8192];             /* unshrink() */
       uch Slide[WSIZE];            /* explode(), inflate(), unreduce() */
     };
#    define stack  area.Stack
#  else
     union work {
       struct {
         short Prefix_of[HSIZE];    /* (8192 * sizeof(short)) */
         uch Suffix_of[HSIZE];
         uch Stack[HSIZE];
       } shrink;
       uch Slide[WSIZE];            /* explode(), inflate(), unreduce() */
     };
#    define prefix_of  area.shrink.Prefix_of
#    define suffix_of  area.shrink.Suffix_of
#    define stack      area.shrink.Stack
#  endif /* ?NEW_UNSHRINK */

#endif /* ?MALLOC_WORK */


/*
	API (& "other") functions...
*/

int huft_build(
	unsigned *b,
	unsigned n,
	unsigned s,
	ush *d,
	ush *e,
	struct huft **t,
	int *m
) ;
int huft_free(struct huft *t) ;
int inflate_codes(struct huft *tl, struct huft *td, int bl, int bd) ;
int inflate_stored() ;
int inflate_fixed() ;
int inflate_dynamic() ;
int inflate_block(int *e) ;
int inflate() ;
int inflate_free() ;


/*
	external data
*/
       
extern int qflag ;
extern union work area ;

#endif /* !INFLATE_H_INC */

/* end of hacked inflate.h */
