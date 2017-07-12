// yui: kernel 1.20 のソースをそのまま利用


/*---------------------------------------------------------------------------

  unzip.h (new)

  This header file contains the public macros and typedefs required by
  both the UnZip sources and by any application using the UnZip API.  If
  UNZIP_INTERNAL is defined, it includes unzpriv.h (containing includes,
  prototypes and extern variables used by the actual UnZip sources).

  ---------------------------------------------------------------------------*/


#ifndef __unzip_h   /* prevent multiple inclusions */
#define __unzip_h


/*---------------------------------------------------------------------------
    Predefined, machine-specific macros.
  ---------------------------------------------------------------------------*/

/* use prototypes and ANSI libraries if __STDC__, or Microsoft or Borland C, or
 * Silicon Graphics, or Convex?, or IBM C Set/2, or GNU gcc/emx, or Watcom C,
 * or Macintosh, or Windows NT, or Sequent, or Atari or IBM RS/6000.
 */

#define PROTO
#define MODERN

/* used to remove arguments in function prototypes for non-ANSI C */
#ifdef PROTO
#  define OF(a) a
#else
#  define OF(a) ()
#endif

/* enable the "const" keyword only if MODERN and if not otherwise instructed */
#ifdef MODERN
#  if (!defined(ZCONST) && (defined(USE_CONST) || !defined(NO_CONST)))
#    define ZCONST const
#  endif
#endif

#ifndef ZCONST
#  define ZCONST
#endif


/*---------------------------------------------------------------------------
    OS-dependent includes
  ---------------------------------------------------------------------------*/

//#    include <stddef.h>
#include <stdlib.h>  /* standard library prototypes, malloc(), etc. */
//typedef size_t extent;
typedef void zvoid;

/*---------------------------------------------------------------------------
    Grab system-dependent definition of EXPENTRY for prototypes below.
  ---------------------------------------------------------------------------*/

#define UZ_EXP

/*---------------------------------------------------------------------------
    Public typedefs.
  ---------------------------------------------------------------------------*/

typedef unsigned char   uch;    /* code assumes unsigned bytes; these type-  */
typedef unsigned short  ush;    /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long   ulg;    /*  predefined on some systems) & match zip  */

/* InputFn is not yet used and is likely to change: */

typedef int   (UZ_EXP MsgFn)     (zvoid *pG, uch *buf, ulg size, int flag);
typedef int   (UZ_EXP InputFn)   (zvoid *pG, uch *buf, int *size, int flag);
typedef void  (UZ_EXP PauseFn)   (zvoid *pG, ZCONST char *prompt, int flag);
typedef int   (UZ_EXP PasswdFn)  (zvoid *pG, int *rcnt, char *pwbuf,
                                     int size, ZCONST char *zfn,
                                     ZCONST char *efn);

/*---------------------------------------------------------------------------
    Remaining private stuff for UnZip compilation.
  ---------------------------------------------------------------------------*/

#endif /* !__unzip_h */

/*---------------------------------------------------------------------------

  unzpriv.h

  This header file contains private (internal) macros, typedefs, prototypes
  and global-variable declarations used by all of the UnZip source files.
  In a prior life it was part of the main unzip.h header, but now it is only
  included by that header if UNZIP_INTERNAL is defined.

  ---------------------------------------------------------------------------*/


#ifndef __unzpriv_h   /* prevent multiple inclusions */
#define __unzpriv_h


#include <stdio.h>
#include <ctype.h>       /* skip for VMS, to use tolower() function? */
#include <errno.h>       /* used in mapname() */
#include <string.h>    /* strcpy, strcmp, memcpy, strchr/strrchr, etc. */


/*************/
/*  Defines  */
/*************/

#define UNZIP_VERSION     20   /* compatible with PKUNZIP 2.0 */
#define VMS_UNZIP_VERSION 42   /* if OS-needed-to-extract is VMS:  can do */

#define far
#define near


/* Logic for case of small memory, length of EOL > 1:  if OUTBUFSIZ == 2048,
 * OUTBUFSIZ>>1 == 1024 and OUTBUFSIZ>>7 == 16; therefore rawbuf is 1008 bytes
 * and transbuf 1040 bytes.  Have room for 32 extra EOL chars; 1008/32 == 31.5
 * chars/line, smaller than estimated 35-70 characters per line for C source
 * and normal text.  Hence difference is sufficient for most "average" files.
 * (Argument scales for larger OUTBUFSIZ.)
 */
#ifdef SMALL_MEM          /* i.e., 16-bit OSes:  MS-DOS, OS/2 1.x, etc. */
#  define LoadFarString(x)       fLoadFarString(__G__ (x))
#  define LoadFarStringSmall(x)  fLoadFarStringSmall(__G__ (x))
#  define LoadFarStringSmall2(x) fLoadFarStringSmall2(__G__ (x))
#  if (defined(_MSC_VER) && (_MSC_VER >= 600))
#    define zfstrcpy(dest, src)  _fstrcpy((dest), (src))
#  endif
#  ifndef Far
#    define Far far  /* __far only works for MSC 6.00, not 6.0a or Borland */
#  endif
#  define OUTBUFSIZ INBUFSIZ
#  if (lenEOL == 1)
#    define RAWBUFSIZ (OUTBUFSIZ>>1)
#  else
#    define RAWBUFSIZ ((OUTBUFSIZ>>1) - (OUTBUFSIZ>>7))
#  endif
#  define TRANSBUFSIZ (OUTBUFSIZ-RAWBUFSIZ)
   typedef short  shrint;            /* short/int or "shrink int" (unshrink) */
#else
#  define zfstrcpy(dest, src)       strcpy((dest), (src))
#  ifdef QDOS
#    define LoadFarString(x)        Qstrfix(x)   /* fix up _ for '.' */
#    define LoadFarStringSmall(x)   Qstrfix(x)
#    define LoadFarStringSmall2(x)  Qstrfix(x)
#  else
#    define LoadFarString(x)        x
#    define LoadFarStringSmall(x)   x
#    define LoadFarStringSmall2(x)  x
#  endif
#  ifdef MED_MEM
#    define OUTBUFSIZ 0xFF80         /* can't malloc arrays of 0xFFE8 or more */
#    define TRANSBUFSIZ 0xFF80
     typedef short  shrint;
#  else
#    define OUTBUFSIZ (lenEOL*WSIZE) /* more efficient text conversion */
#    define TRANSBUFSIZ (lenEOL*OUTBUFSIZ)
#    ifdef AMIGA
       typedef short shrint;
#    else
       typedef int  shrint;          /* for efficiency/speed, we hope... */
#    endif
#  endif /* ?MED_MEM */
#  define RAWBUFSIZ OUTBUFSIZ
#endif /* ?SMALL_MEM */




/*---------------------------------------------------------------------------
    True sizes of the various headers, as defined by PKWARE--so it is not
    likely that these will ever change.  But if they do, make sure both these
    defines AND the typedefs below get updated accordingly.
  ---------------------------------------------------------------------------*/
#define LREC_SIZE   26   /* lengths of local file headers, central */
#define CREC_SIZE   42   /*  directory headers, and the end-of-    */
#define ECREC_SIZE  18   /*  central-dir record, respectively      */

#define memzero(dest,len)      memset(dest,0,len)



/*---------------------------------------------------------------------------
    Zipfile work area declarations.
  ---------------------------------------------------------------------------*/

union work {
	uch *Slide;        /* explode(), inflate(), unreduce() */
};

#define slide G.area.Slide
#define redirSlide G.area.Slide

/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */

struct huft {
    uch e;                /* number of extra bits or operation */
    uch b;                /* number of bits in this code or subcode */
    union {
        ush n;            /* literal, length base, or distance base */
        //struct huft *t;   /* pointer to next level of table */
		ush t;			// ポインタをケチる
    } v;
};



/*************/
/*  Globals  */
/*************/

#ifndef __globals_h
#define __globals_h

/*************/
/*  Globals  */
/*************/

struct Globals {
    union work area;          /* see unzpriv.h for definition of work */
	zlibIO *zin;
	zlibIO *zout;
    unsigned hufts;           /* track memory usage */
    struct huft *fixed_tl;    /* inflate static */
    struct huft *fixed_td;    /* inflate static */
    int fixed_bl, fixed_bd;   /* inflate static */
    unsigned wp;              /* inflate static: current position in slide */
    ulg bb;                   /* inflate static: bit buffer */
    unsigned bk;              /* inflate static: bits in bit buffer */
	struct huft *tbp00;

	// 自力 malloc 関係のワーク
	unsigned long *membase;
	unsigned long *memptr;
	unsigned long *memptr0;
};  /* end of struct Globals */


/***************************************************************************/


//#define CRC_32_TAB  G.crc_32_tab


struct Globals *globalsCtor   OF((void));

#ifdef REENTRANT
#  define G                   (*pG)
#  define __G                 pG
#  define __G__               pG,
#  define __GPRO              struct Globals *pG
#  define __GPRO__            struct Globals *pG,
#  define __GDEF              struct Globals *pG;
#  ifdef  USETHREADID
#    define GETGLOBALS()      struct Globals *pG = getGlobalPointer();
#    define DESTROYGLOBALS()  {free_G_buffers(pG); deregisterGlobalPointer(pG);}
#  else
#    define GETGLOBALS()      struct Globals *pG = GG;
#    define DESTROYGLOBALS()  {free_G_buffers(pG); free(pG);}
#  endif /* ?USETHREADID */
#  define CONSTRUCTGLOBALS()  struct Globals *pG = globalsCtor()
#else /* !REENTRANT */
   extern struct Globals      G;
#  define __G
#  define __G__
#  define __GPRO              void
#  define __GPRO__
#  define __GDEF
#  define GETGLOBALS()
#  define CONSTRUCTGLOBALS()  globalsCtor()
#  define DESTROYGLOBALS()
#endif /* ?REENTRANT */

#endif /* __globals_h */


/*************************/
/*  Function Prototypes  */
/*************************/

/*---------------------------------------------------------------------------
    Decompression functions:
  ---------------------------------------------------------------------------*/

int    huft_free                 OF((struct huft *t));          /* inflate.c */
int    huft_build                OF((__GPRO__ unsigned *b, unsigned n,
                                     unsigned s, const ush *d, const ush *e,
                                     struct huft **t, int *m));

#endif /* !__unzpriv_h */



