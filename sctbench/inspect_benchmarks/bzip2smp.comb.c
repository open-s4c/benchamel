//args: --no-ht -1 -p{NUMTHREADS} bzip_input {OUTFILE}

/*
 *  ./bzip2smp  -v -p4  < inputfile > output 
 */


/*
  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 3
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/

/*
Copyright (c) 2005 Konstantin Isakov. Based on the original libbzip2 sources,
part of bzip2, version 1.0.2, Copyright (c) 1996-2002 Julian R Seward.
See LICENSE file for details.
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>



/* The level is 0 initially */
extern int verbosityLevel;

/* Messages with level > verbosityLevel are not displayed */
extern void note( int level, const char * format, ... );

//////////////////////////// end of note.h //////////////////////////////

#ifdef __linux__
#define HT_DETECTION
#endif

extern int isHtPresent( void );


///////////////////////////////////////////////////////////////////////////


#ifndef _BZLIB_H
#define _BZLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define BZ_RUN               0
#define BZ_FLUSH             1
#define BZ_FINISH            2
/* This flag requests to stop just before the block sort stage */
#define BZ_STOP_BEFORE_BLOCKSORT            0x10

#define BZ_OK                0
#define BZ_RUN_OK            1
#define BZ_FLUSH_OK          2
#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
/* This indicates that the process was stopped just before the blocksort
stage */
#define BZ_STOPPED_BEFORE_BLOCKSORT        5

#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
#define BZ_CONFIG_ERROR      (-9)

typedef 
   struct {
      char *next_in;
      unsigned int avail_in;
      unsigned int total_in_lo32;
      unsigned int total_in_hi32;

      char *next_out;
      unsigned int avail_out;
      unsigned int total_out_lo32;
      unsigned int total_out_hi32;

      void *state;

      void *(*bzalloc)(void *,int,int);
      void (*bzfree)(void *,void *);
      void *opaque;
   } 
   bz_stream;

typedef 
   struct {
      unsigned long state_in_ch;
      long state_in_len;
      unsigned long combinedCRC;
      long blockNo;
   } 
   bz_stream_state_bs;

   typedef 
   struct {
     unsigned long bsBuff;
     long bsLive;
   } 
   bz_stream_state_out;


#ifndef BZ_IMPORT
#define BZ_EXPORT
#endif

/* Need a definitition for FILE */
#include <stdio.h>

#ifdef _WIN32
#   include <windows.h>
#   ifdef small
      /* windows.h define small to char */
#      undef small
#   endif
#   ifdef BZ_EXPORT
#   define BZ_API(func) WINAPI func
#   define BZ_EXTERN extern
#   else
   /* import windows dll dynamically */
#   define BZ_API(func) (WINAPI * func)
#   define BZ_EXTERN
#   endif
#else
#   define BZ_API(func) func
#   define BZ_EXTERN extern
#endif


/*-- Core (low-level) library functions --*/

BZ_EXTERN int BZ_API(BZ2_bzCompressInit) ( 
      bz_stream* strm, 
      int        blockSize100k, 
      int        verbosity, 
      int        workFactor,
      int        delayedAllocation
   );

BZ_EXTERN int BZ_API(BZ2_bzCompress) ( 
      bz_stream* strm, 
      int action 
   );

/* Saves RL state, block number+1, and updated combined crc */
BZ_EXTERN void BZ_API(BZ2_bzCompressSaveStateBeforeBlocksort) ( 
      bz_stream* strm, 
      bz_stream_state_bs * state
   );

BZ_EXTERN void BZ_API(BZ2_bzCompressRestoreState) ( 
      bz_stream* strm, 
      const bz_stream_state_bs * state
   );

/* Saves the buffer of the bitstream outout */
BZ_EXTERN void BZ_API(BZ2_bzCompressSaveOutputState) ( 
      bz_stream* strm, 
      bz_stream_state_out * state
   );

BZ_EXTERN void BZ_API(BZ2_bzCompressRestoreOutputState) ( 
      bz_stream* strm, 
      const bz_stream_state_out * state
   );

BZ_EXTERN void BZ_API(BZ2_bzCompressDoBlocksort) ( 
      bz_stream* strm );

/* Returns number of bytes used in the outBuf */
BZ_EXTERN size_t BZ_API(BZ2_bzCompressStoreBlocksort) ( 
      bz_stream* strm, void * outBuf,
      int lastBlock );


BZ_EXTERN int BZ_API(BZ2_bzCompressEnd) ( 
      bz_stream* strm 
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompressInit) ( 
      bz_stream *strm, 
      int       verbosity, 
      int       small
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompress) ( 
      bz_stream* strm 
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompressEnd) ( 
      bz_stream *strm 
   );



/*-- High(er) level library functions --*/

#ifndef BZ_NO_STDIO
#define BZ_MAX_UNUSED 5000

typedef void BZFILE;

BZ_EXTERN BZFILE* BZ_API(BZ2_bzReadOpen) ( 
      int*  bzerror,   
      FILE* f, 
      int   verbosity, 
      int   small,
      void* unused,    
      int   nUnused 
   );

BZ_EXTERN void BZ_API(BZ2_bzReadClose) ( 
      int*    bzerror, 
      BZFILE* b 
   );

BZ_EXTERN void BZ_API(BZ2_bzReadGetUnused) ( 
      int*    bzerror, 
      BZFILE* b, 
      void**  unused,  
      int*    nUnused 
   );

BZ_EXTERN int BZ_API(BZ2_bzRead) ( 
      int*    bzerror, 
      BZFILE* b, 
      void*   buf, 
      int     len 
   );

BZ_EXTERN BZFILE* BZ_API(BZ2_bzWriteOpen) ( 
      int*  bzerror,      
      FILE* f, 
      int   blockSize100k, 
      int   verbosity, 
      int   workFactor 
   );

BZ_EXTERN void BZ_API(BZ2_bzWrite) ( 
      int*    bzerror, 
      BZFILE* b, 
      void*   buf, 
      int     len 
   );

BZ_EXTERN void BZ_API(BZ2_bzWriteClose) ( 
      int*          bzerror, 
      BZFILE*       b, 
      int           abandon, 
      unsigned int* nbytes_in, 
      unsigned int* nbytes_out 
   );

BZ_EXTERN void BZ_API(BZ2_bzWriteClose64) ( 
      int*          bzerror, 
      BZFILE*       b, 
      int           abandon, 
      unsigned int* nbytes_in_lo32, 
      unsigned int* nbytes_in_hi32, 
      unsigned int* nbytes_out_lo32, 
      unsigned int* nbytes_out_hi32
   );
#endif


/*-- Utility functions --*/

BZ_EXTERN int BZ_API(BZ2_bzBuffToBuffCompress) ( 
      char*         dest, 
      unsigned int* destLen,
      char*         source, 
      unsigned int  sourceLen,
      int           blockSize100k, 
      int           verbosity, 
      int           workFactor 
   );

BZ_EXTERN int BZ_API(BZ2_bzBuffToBuffDecompress) ( 
      char*         dest, 
      unsigned int* destLen,
      char*         source, 
      unsigned int  sourceLen,
      int           small, 
      int           verbosity 
   );


/*--
   Code contributed by Yoshioka Tsuneo
   (QWF00133@niftyserve.or.jp/tsuneo-y@is.aist-nara.ac.jp),
   to support better zlib compatibility.
   This code is not _officially_ part of libbzip2 (yet);
   I haven't tested it, documented it, or considered the
   threading-safeness of it.
   If this code breaks, please contact both Yoshioka and me.
--*/

BZ_EXTERN const char * BZ_API(BZ2_bzlibVersion) (
      void
   );

#ifndef BZ_NO_STDIO
BZ_EXTERN BZFILE * BZ_API(BZ2_bzopen) (
      const char *path,
      const char *mode
   );

BZ_EXTERN BZFILE * BZ_API(BZ2_bzdopen) (
      int        fd,
      const char *mode
   );
         
BZ_EXTERN int BZ_API(BZ2_bzread) (
      BZFILE* b, 
      void* buf, 
      int len 
   );

BZ_EXTERN int BZ_API(BZ2_bzwrite) (
      BZFILE* b, 
      void*   buf, 
      int     len 
   );

BZ_EXTERN int BZ_API(BZ2_bzflush) (
      BZFILE* b
   );

BZ_EXTERN void BZ_API(BZ2_bzclose) (
      BZFILE* b
   );

BZ_EXTERN const char * BZ_API(BZ2_bzerror) (
      BZFILE *b, 
      int    *errnum
   );
#endif

#ifdef __cplusplus
}
#endif

#endif

/*-------------------------------------------------------------*/
/*--- end                                           bzlib.h ---*/
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*--- Private header file for the library.                  ---*/
/*---                                       bzlib_private.h ---*/
/*-------------------------------------------------------------*/

#ifndef _BZLIB_PRIVATE_H
#define _BZLIB_PRIVATE_H


#ifndef BZ_NO_STDIO
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#endif


/*-- General stuff. --*/

#define BZ_VERSION  "1.0.2, 30-Dec-2001"

typedef char            Char;
typedef unsigned char   Bool;
typedef unsigned char   UChar;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef short           Int16;
typedef unsigned short  UInt16;

#define True  ((Bool)1)
#define False ((Bool)0)

#ifndef __GNUC__
#define __inline__  /* */
#endif 

#ifndef BZ_NO_STDIO
extern void BZ2_bz__AssertH__fail ( int errcode );
#define AssertH(cond,errcode) \
   { if (!(cond)) BZ2_bz__AssertH__fail ( errcode ); }
#if BZ_DEBUG
#define AssertD(cond,msg) \
   { if (!(cond)) {       \
      fprintf ( stderr,   \
        "\n\nlibbzip2(debug build): internal error\n\t%s\n", msg );\
      exit(1); \
   }}
#else
#define AssertD(cond,msg) /* */
#endif
#define VPrintf0(zf) \
   fprintf(stderr,zf)
#define VPrintf1(zf,za1) \
   fprintf(stderr,zf,za1)
#define VPrintf2(zf,za1,za2) \
   fprintf(stderr,zf,za1,za2)
#define VPrintf3(zf,za1,za2,za3) \
   fprintf(stderr,zf,za1,za2,za3)
#define VPrintf4(zf,za1,za2,za3,za4) \
   fprintf(stderr,zf,za1,za2,za3,za4)
#define VPrintf5(zf,za1,za2,za3,za4,za5) \
   fprintf(stderr,zf,za1,za2,za3,za4,za5)
#else
extern void bz_internal_error ( int errcode );
#define AssertH(cond,errcode) \
   { if (!(cond)) bz_internal_error ( errcode ); }
#define AssertD(cond,msg) /* */
#define VPrintf0(zf) /* */
#define VPrintf1(zf,za1) /* */
#define VPrintf2(zf,za1,za2) /* */
#define VPrintf3(zf,za1,za2,za3) /* */
#define VPrintf4(zf,za1,za2,za3,za4) /* */
#define VPrintf5(zf,za1,za2,za3,za4,za5) /* */
#endif


#define BZALLOC(nnn) (strm->bzalloc)(strm->opaque,(nnn),1)
#define BZFREE(ppp)  (strm->bzfree)(strm->opaque,(ppp))


/*-- Header bytes. --*/

#define BZ_HDR_B 0x42   /* 'B' */
#define BZ_HDR_Z 0x5a   /* 'Z' */
#define BZ_HDR_h 0x68   /* 'h' */
#define BZ_HDR_0 0x30   /* '0' */
  
/*-- Constants for the back end. --*/

#define BZ_MAX_ALPHA_SIZE 258
#define BZ_MAX_CODE_LEN    23

#define BZ_RUNA 0
#define BZ_RUNB 1

#define BZ_N_GROUPS 6
#define BZ_G_SIZE   50
#define BZ_N_ITERS  4

#define BZ_MAX_SELECTORS (2 + (900000 / BZ_G_SIZE))



/*-- Stuff for randomising repetitive blocks. --*/

extern Int32 BZ2_rNums[512];

#define BZ_RAND_DECLS                          \
   Int32 rNToGo;                               \
   Int32 rTPos                                 \

#define BZ_RAND_INIT_MASK                      \
   s->rNToGo = 0;                              \
   s->rTPos  = 0                               \

#define BZ_RAND_MASK ((s->rNToGo == 1) ? 1 : 0)

#define BZ_RAND_UPD_MASK                       \
   if (s->rNToGo == 0) {                       \
      s->rNToGo = BZ2_rNums[s->rTPos];         \
      s->rTPos++;                              \
      if (s->rTPos == 512) s->rTPos = 0;       \
   }                                           \
   s->rNToGo--;



/*-- Stuff for doing CRCs. --*/

extern UInt32 BZ2_crc32Table[256];

#define BZ_INITIALISE_CRC(crcVar)              \
{                                              \
   crcVar = 0xffffffffL;                       \
}

#define BZ_FINALISE_CRC(crcVar)                \
{                                              \
   crcVar = ~(crcVar);                         \
}

#define BZ_UPDATE_CRC(crcVar,cha)              \
{                                              \
   crcVar = (crcVar << 8) ^                    \
            BZ2_crc32Table[(crcVar >> 24) ^    \
                           ((UChar)cha)];      \
}



/*-- States and modes for compression. --*/

#define BZ_M_IDLE      1
#define BZ_M_RUNNING   2
#define BZ_M_FLUSHING  3
#define BZ_M_FINISHING 4

#define BZ_S_OUTPUT    1
#define BZ_S_INPUT     2

#define BZ_N_RADIX 2
#define BZ_N_QSORT 12
#define BZ_N_SHELL 18
#define BZ_N_OVERSHOOT (BZ_N_RADIX + BZ_N_QSORT + BZ_N_SHELL + 2)




/*-- Structure holding all the compression-side stuff. --*/

typedef
   struct {
      /* pointer back to the struct bz_stream */
      bz_stream* strm;

      /* mode this stream is in, and whether inputting */
      /* or outputting data */
      Int32    mode;
      Int32    state;

      /* remembers avail_in when flush/finish requested */
      UInt32   avail_in_expect;

      /* for doing the block sorting */
      UInt32*  arr1;
      UInt32*  arr2;
      UInt32*  ftab;
      Int32    origPtr;

      /* aliases for arr1 and arr2 */
      UInt32*  ptr;
      UChar*   block;
      UInt16*  mtfv;
      UChar*   zbits;

      /* for deciding when to use the fallback sorting algorithm */
      Int32    workFactor;

      /* run-length-encoding of the input */
      UInt32   state_in_ch;
      Int32    state_in_len;
      BZ_RAND_DECLS;

      /* input and output limits and current posns */
      Int32    nblock;
      Int32    nblockMAX;
      Int32    numZ;
      Int32    state_out_pos;

      /* map of bytes used in block */
      Int32    nInUse;
      Bool     inUse[256];
      UChar    unseqToSeq[256];

      /* the buffer for bit stream creation */
      UInt32   bsBuff;
      Int32    bsLive;

      /* block and combined CRCs */
      UInt32   blockCRC;
      UInt32   combinedCRC;

      /* misc administratium */
      Int32    verbosity;
      Int32    blockNo;
      Int32    blockSize100k;

      /* stuff for coding the MTF values */
      Int32    nMTF;
      Int32    mtfFreq    [BZ_MAX_ALPHA_SIZE];
      UChar    selector   [BZ_MAX_SELECTORS];
      UChar    selectorMtf[BZ_MAX_SELECTORS];

      UChar    len     [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    code    [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    rfreq   [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      /* second dimension: only 3 needed; 4 makes index calculations faster */
      UInt32   len_pack[BZ_MAX_ALPHA_SIZE][4];

      /* We supply our own arr2. This flag indicates that it should not
      be freed */

      int dontFreeArr2;

      /* some computed MTF values; it was needed to save it in order to split
      the sendMTFValues() function to a calculation part and saving part.*/
      Int32 nGroups, nSelectors, alphaSize;

   }
   EState;



/*-- externs for compression. --*/

extern void 
BZ2_blockSort ( EState* );

extern void 
BZ2_compressBlock ( EState*, Bool );

extern void 
BZ2_compressBlock_compute ( EState* );

extern void 
BZ2_compressBlock_save ( EState*, Bool );

extern void 
BZ2_bsInitWrite ( EState* );

extern void 
BZ2_hbAssignCodes ( Int32*, UChar*, Int32, Int32, Int32 );

extern void 
BZ2_hbMakeCodeLengths ( UChar*, Int32*, Int32, Int32 );



/*-- states for decompression. --*/

#define BZ_X_IDLE        1
#define BZ_X_OUTPUT      2

#define BZ_X_MAGIC_1     10
#define BZ_X_MAGIC_2     11
#define BZ_X_MAGIC_3     12
#define BZ_X_MAGIC_4     13
#define BZ_X_BLKHDR_1    14
#define BZ_X_BLKHDR_2    15
#define BZ_X_BLKHDR_3    16
#define BZ_X_BLKHDR_4    17
#define BZ_X_BLKHDR_5    18
#define BZ_X_BLKHDR_6    19
#define BZ_X_BCRC_1      20
#define BZ_X_BCRC_2      21
#define BZ_X_BCRC_3      22
#define BZ_X_BCRC_4      23
#define BZ_X_RANDBIT     24
#define BZ_X_ORIGPTR_1   25
#define BZ_X_ORIGPTR_2   26
#define BZ_X_ORIGPTR_3   27
#define BZ_X_MAPPING_1   28
#define BZ_X_MAPPING_2   29
#define BZ_X_SELECTOR_1  30
#define BZ_X_SELECTOR_2  31
#define BZ_X_SELECTOR_3  32
#define BZ_X_CODING_1    33
#define BZ_X_CODING_2    34
#define BZ_X_CODING_3    35
#define BZ_X_MTF_1       36
#define BZ_X_MTF_2       37
#define BZ_X_MTF_3       38
#define BZ_X_MTF_4       39
#define BZ_X_MTF_5       40
#define BZ_X_MTF_6       41
#define BZ_X_ENDHDR_2    42
#define BZ_X_ENDHDR_3    43
#define BZ_X_ENDHDR_4    44
#define BZ_X_ENDHDR_5    45
#define BZ_X_ENDHDR_6    46
#define BZ_X_CCRC_1      47
#define BZ_X_CCRC_2      48
#define BZ_X_CCRC_3      49
#define BZ_X_CCRC_4      50



/*-- Constants for the fast MTF decoder. --*/

#define MTFA_SIZE 4096
#define MTFL_SIZE 16



/*-- Structure holding all the decompression-side stuff. --*/

typedef
   struct {
      /* pointer back to the struct bz_stream */
      bz_stream* strm;

      /* state indicator for this stream */
      Int32    state;

      /* for doing the final run-length decoding */
      UChar    state_out_ch;
      Int32    state_out_len;
      Bool     blockRandomised;
      BZ_RAND_DECLS;

      /* the buffer for bit stream reading */
      UInt32   bsBuff;
      Int32    bsLive;

      /* misc administratium */
      Int32    blockSize100k;
      Bool     smallDecompress;
      Int32    currBlockNo;
      Int32    verbosity;

      /* for undoing the Burrows-Wheeler transform */
      Int32    origPtr;
      UInt32   tPos;
      Int32    k0;
      Int32    unzftab[256];
      Int32    nblock_used;
      Int32    cftab[257];
      Int32    cftabCopy[257];

      /* for undoing the Burrows-Wheeler transform (FAST) */
      UInt32   *tt;

      /* for undoing the Burrows-Wheeler transform (SMALL) */
      UInt16   *ll16;
      UChar    *ll4;

      /* stored and calculated CRCs */
      UInt32   storedBlockCRC;
      UInt32   storedCombinedCRC;
      UInt32   calculatedBlockCRC;
      UInt32   calculatedCombinedCRC;

      /* map of bytes used in block */
      Int32    nInUse;
      Bool     inUse[256];
      Bool     inUse16[16];
      UChar    seqToUnseq[256];

      /* for decoding the MTF values */
      UChar    mtfa   [MTFA_SIZE];
      Int32    mtfbase[256 / MTFL_SIZE];
      UChar    selector   [BZ_MAX_SELECTORS];
      UChar    selectorMtf[BZ_MAX_SELECTORS];
      UChar    len  [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];

      Int32    limit  [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    base   [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    perm   [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    minLens[BZ_N_GROUPS];

      /* save area for scalars in the main decompress code */
      Int32    save_i;
      Int32    save_j;
      Int32    save_t;
      Int32    save_alphaSize;
      Int32    save_nGroups;
      Int32    save_nSelectors;
      Int32    save_EOB;
      Int32    save_groupNo;
      Int32    save_groupPos;
      Int32    save_nextSym;
      Int32    save_nblockMAX;
      Int32    save_nblock;
      Int32    save_es;
      Int32    save_N;
      Int32    save_curr;
      Int32    save_zt;
      Int32    save_zn; 
      Int32    save_zvec;
      Int32    save_zj;
      Int32    save_gSel;
      Int32    save_gMinlen;
      Int32*   save_gLimit;
      Int32*   save_gBase;
      Int32*   save_gPerm;

   }
   DState;



/*-- Macros for decompression. --*/

#define BZ_GET_FAST(cccc)                     \
    s->tPos = s->tt[s->tPos];                 \
    cccc = (UChar)(s->tPos & 0xff);           \
    s->tPos >>= 8;

#define BZ_GET_FAST_C(cccc)                   \
    c_tPos = c_tt[c_tPos];                    \
    cccc = (UChar)(c_tPos & 0xff);            \
    c_tPos >>= 8;

#define SET_LL4(i,n)                                          \
   { if (((i) & 0x1) == 0)                                    \
        s->ll4[(i) >> 1] = (s->ll4[(i) >> 1] & 0xf0) | (n); else    \
        s->ll4[(i) >> 1] = (s->ll4[(i) >> 1] & 0x0f) | ((n) << 4);  \
   }

#define GET_LL4(i)                             \
   ((((UInt32)(s->ll4[(i) >> 1])) >> (((i) << 2) & 0x4)) & 0xF)

#define SET_LL(i,n)                          \
   { s->ll16[i] = (UInt16)(n & 0x0000ffff);  \
     SET_LL4(i, n >> 16);                    \
   }

#define GET_LL(i) \
   (((UInt32)s->ll16[i]) | (GET_LL4(i) << 16))

#define BZ_GET_SMALL(cccc)                            \
      cccc = BZ2_indexIntoF ( s->tPos, s->cftab );    \
      s->tPos = GET_LL(s->tPos);


/*-- externs for decompression. --*/

extern Int32 BZ2_indexIntoF ( Int32, Int32* );

extern Int32 
BZ2_decompress ( DState* );

extern void 
BZ2_hbCreateDecodeTables ( Int32*, Int32*, Int32*, UChar*,
                           Int32,  Int32, Int32 );


#endif


/*-- BZ_NO_STDIO seems to make NULL disappear on some platforms. --*/

#ifdef BZ_NO_STDIO
#ifndef NULL
#define NULL 0
#endif
#endif


/*-------------------------------------------------------------*/
/*--- end                                   bzlib_private.h ---*/
/*-------------------------------------------------------------*/



/*---------------------------------------------*/
Int32 BZ2_rNums[512] = { 
   619, 720, 127, 481, 931, 816, 813, 233, 566, 247, 
   985, 724, 205, 454, 863, 491, 741, 242, 949, 214, 
   733, 859, 335, 708, 621, 574, 73, 654, 730, 472, 
   419, 436, 278, 496, 867, 210, 399, 680, 480, 51, 
   878, 465, 811, 169, 869, 675, 611, 697, 867, 561, 
   862, 687, 507, 283, 482, 129, 807, 591, 733, 623, 
   150, 238, 59, 379, 684, 877, 625, 169, 643, 105, 
   170, 607, 520, 932, 727, 476, 693, 425, 174, 647, 
   73, 122, 335, 530, 442, 853, 695, 249, 445, 515, 
   909, 545, 703, 919, 874, 474, 882, 500, 594, 612, 
   641, 801, 220, 162, 819, 984, 589, 513, 495, 799, 
   161, 604, 958, 533, 221, 400, 386, 867, 600, 782, 
   382, 596, 414, 171, 516, 375, 682, 485, 911, 276, 
   98, 553, 163, 354, 666, 933, 424, 341, 533, 870, 
   227, 730, 475, 186, 263, 647, 537, 686, 600, 224, 
   469, 68, 770, 919, 190, 373, 294, 822, 808, 206, 
   184, 943, 795, 384, 383, 461, 404, 758, 839, 887, 
   715, 67, 618, 276, 204, 918, 873, 777, 604, 560, 
   951, 160, 578, 722, 79, 804, 96, 409, 713, 940, 
   652, 934, 970, 447, 318, 353, 859, 672, 112, 785, 
   645, 863, 803, 350, 139, 93, 354, 99, 820, 908, 
   609, 772, 154, 274, 580, 184, 79, 626, 630, 742, 
   653, 282, 762, 623, 680, 81, 927, 626, 789, 125, 
   411, 521, 938, 300, 821, 78, 343, 175, 128, 250, 
   170, 774, 972, 275, 999, 639, 495, 78, 352, 126, 
   857, 956, 358, 619, 580, 124, 737, 594, 701, 612, 
   669, 112, 134, 694, 363, 992, 809, 743, 168, 974, 
   944, 375, 748, 52, 600, 747, 642, 182, 862, 81, 
   344, 805, 988, 739, 511, 655, 814, 334, 249, 515, 
   897, 955, 664, 981, 649, 113, 974, 459, 893, 228, 
   433, 837, 553, 268, 926, 240, 102, 654, 459, 51, 
   686, 754, 806, 760, 493, 403, 415, 394, 687, 700, 
   946, 670, 656, 610, 738, 392, 760, 799, 887, 653, 
   978, 321, 576, 617, 626, 502, 894, 679, 243, 440, 
   680, 879, 194, 572, 640, 724, 926, 56, 204, 700, 
   707, 151, 457, 449, 797, 195, 791, 558, 945, 679, 
   297, 59, 87, 824, 713, 663, 412, 693, 342, 606, 
   134, 108, 571, 364, 631, 212, 174, 643, 304, 329, 
   343, 97, 430, 751, 497, 314, 983, 374, 822, 928, 
   140, 206, 73, 263, 980, 736, 876, 478, 430, 305, 
   170, 514, 364, 692, 829, 82, 855, 953, 676, 246, 
   369, 970, 294, 750, 807, 827, 150, 790, 288, 923, 
   804, 378, 215, 828, 592, 281, 565, 555, 710, 82, 
   896, 831, 547, 261, 524, 462, 293, 465, 502, 56, 
   661, 821, 976, 991, 658, 869, 905, 758, 745, 193, 
   768, 550, 608, 933, 378, 286, 215, 979, 792, 961, 
   61, 688, 793, 644, 986, 403, 106, 366, 905, 644, 
   372, 567, 466, 434, 645, 210, 389, 550, 919, 135, 
   780, 773, 635, 389, 707, 100, 626, 958, 165, 504, 
   920, 176, 193, 713, 857, 265, 203, 50, 668, 108, 
   645, 990, 626, 197, 510, 357, 358, 850, 858, 364, 
   936, 638
};


/*-------------------------------------------------------------*/
/*--- end                                       randtable.c ---*/
/*-------------------------------------------------------------*/





/*-------------------------------------------------------------*/
/*--- Table for doing CRCs                                  ---*/
/*---                                            crctable.c ---*/
/*-------------------------------------------------------------*/

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/


//#include "bzlib_private.h"

/*--
  I think this is an implementation of the AUTODIN-II,
  Ethernet & FDDI 32-bit CRC standard.  Vaguely derived
  from code by Rob Warnock, in Section 51 of the
  comp.compression FAQ.
--*/

UInt32 BZ2_crc32Table[256] = {

   /*-- Ugly, innit? --*/

   0x00000000L, 0x04c11db7L, 0x09823b6eL, 0x0d4326d9L,
   0x130476dcL, 0x17c56b6bL, 0x1a864db2L, 0x1e475005L,
   0x2608edb8L, 0x22c9f00fL, 0x2f8ad6d6L, 0x2b4bcb61L,
   0x350c9b64L, 0x31cd86d3L, 0x3c8ea00aL, 0x384fbdbdL,
   0x4c11db70L, 0x48d0c6c7L, 0x4593e01eL, 0x4152fda9L,
   0x5f15adacL, 0x5bd4b01bL, 0x569796c2L, 0x52568b75L,
   0x6a1936c8L, 0x6ed82b7fL, 0x639b0da6L, 0x675a1011L,
   0x791d4014L, 0x7ddc5da3L, 0x709f7b7aL, 0x745e66cdL,
   0x9823b6e0L, 0x9ce2ab57L, 0x91a18d8eL, 0x95609039L,
   0x8b27c03cL, 0x8fe6dd8bL, 0x82a5fb52L, 0x8664e6e5L,
   0xbe2b5b58L, 0xbaea46efL, 0xb7a96036L, 0xb3687d81L,
   0xad2f2d84L, 0xa9ee3033L, 0xa4ad16eaL, 0xa06c0b5dL,
   0xd4326d90L, 0xd0f37027L, 0xddb056feL, 0xd9714b49L,
   0xc7361b4cL, 0xc3f706fbL, 0xceb42022L, 0xca753d95L,
   0xf23a8028L, 0xf6fb9d9fL, 0xfbb8bb46L, 0xff79a6f1L,
   0xe13ef6f4L, 0xe5ffeb43L, 0xe8bccd9aL, 0xec7dd02dL,
   0x34867077L, 0x30476dc0L, 0x3d044b19L, 0x39c556aeL,
   0x278206abL, 0x23431b1cL, 0x2e003dc5L, 0x2ac12072L,
   0x128e9dcfL, 0x164f8078L, 0x1b0ca6a1L, 0x1fcdbb16L,
   0x018aeb13L, 0x054bf6a4L, 0x0808d07dL, 0x0cc9cdcaL,
   0x7897ab07L, 0x7c56b6b0L, 0x71159069L, 0x75d48ddeL,
   0x6b93dddbL, 0x6f52c06cL, 0x6211e6b5L, 0x66d0fb02L,
   0x5e9f46bfL, 0x5a5e5b08L, 0x571d7dd1L, 0x53dc6066L,
   0x4d9b3063L, 0x495a2dd4L, 0x44190b0dL, 0x40d816baL,
   0xaca5c697L, 0xa864db20L, 0xa527fdf9L, 0xa1e6e04eL,
   0xbfa1b04bL, 0xbb60adfcL, 0xb6238b25L, 0xb2e29692L,
   0x8aad2b2fL, 0x8e6c3698L, 0x832f1041L, 0x87ee0df6L,
   0x99a95df3L, 0x9d684044L, 0x902b669dL, 0x94ea7b2aL,
   0xe0b41de7L, 0xe4750050L, 0xe9362689L, 0xedf73b3eL,
   0xf3b06b3bL, 0xf771768cL, 0xfa325055L, 0xfef34de2L,
   0xc6bcf05fL, 0xc27dede8L, 0xcf3ecb31L, 0xcbffd686L,
   0xd5b88683L, 0xd1799b34L, 0xdc3abdedL, 0xd8fba05aL,
   0x690ce0eeL, 0x6dcdfd59L, 0x608edb80L, 0x644fc637L,
   0x7a089632L, 0x7ec98b85L, 0x738aad5cL, 0x774bb0ebL,
   0x4f040d56L, 0x4bc510e1L, 0x46863638L, 0x42472b8fL,
   0x5c007b8aL, 0x58c1663dL, 0x558240e4L, 0x51435d53L,
   0x251d3b9eL, 0x21dc2629L, 0x2c9f00f0L, 0x285e1d47L,
   0x36194d42L, 0x32d850f5L, 0x3f9b762cL, 0x3b5a6b9bL,
   0x0315d626L, 0x07d4cb91L, 0x0a97ed48L, 0x0e56f0ffL,
   0x1011a0faL, 0x14d0bd4dL, 0x19939b94L, 0x1d528623L,
   0xf12f560eL, 0xf5ee4bb9L, 0xf8ad6d60L, 0xfc6c70d7L,
   0xe22b20d2L, 0xe6ea3d65L, 0xeba91bbcL, 0xef68060bL,
   0xd727bbb6L, 0xd3e6a601L, 0xdea580d8L, 0xda649d6fL,
   0xc423cd6aL, 0xc0e2d0ddL, 0xcda1f604L, 0xc960ebb3L,
   0xbd3e8d7eL, 0xb9ff90c9L, 0xb4bcb610L, 0xb07daba7L,
   0xae3afba2L, 0xaafbe615L, 0xa7b8c0ccL, 0xa379dd7bL,
   0x9b3660c6L, 0x9ff77d71L, 0x92b45ba8L, 0x9675461fL,
   0x8832161aL, 0x8cf30badL, 0x81b02d74L, 0x857130c3L,
   0x5d8a9099L, 0x594b8d2eL, 0x5408abf7L, 0x50c9b640L,
   0x4e8ee645L, 0x4a4ffbf2L, 0x470cdd2bL, 0x43cdc09cL,
   0x7b827d21L, 0x7f436096L, 0x7200464fL, 0x76c15bf8L,
   0x68860bfdL, 0x6c47164aL, 0x61043093L, 0x65c52d24L,
   0x119b4be9L, 0x155a565eL, 0x18197087L, 0x1cd86d30L,
   0x029f3d35L, 0x065e2082L, 0x0b1d065bL, 0x0fdc1becL,
   0x3793a651L, 0x3352bbe6L, 0x3e119d3fL, 0x3ad08088L,
   0x2497d08dL, 0x2056cd3aL, 0x2d15ebe3L, 0x29d4f654L,
   0xc5a92679L, 0xc1683bceL, 0xcc2b1d17L, 0xc8ea00a0L,
   0xd6ad50a5L, 0xd26c4d12L, 0xdf2f6bcbL, 0xdbee767cL,
   0xe3a1cbc1L, 0xe760d676L, 0xea23f0afL, 0xeee2ed18L,
   0xf0a5bd1dL, 0xf464a0aaL, 0xf9278673L, 0xfde69bc4L,
   0x89b8fd09L, 0x8d79e0beL, 0x803ac667L, 0x84fbdbd0L,
   0x9abc8bd5L, 0x9e7d9662L, 0x933eb0bbL, 0x97ffad0cL,
   0xafb010b1L, 0xab710d06L, 0xa6322bdfL, 0xa2f33668L,
   0xbcb4666dL, 0xb8757bdaL, 0xb5365d03L, 0xb1f740b4L
};


/*-------------------------------------------------------------*/
/*--- end                                        crctable.c ---*/
/*-------------------------------------------------------------*/





/*-------------------------------------------------------------*/
/*--- Compression machinery (not incl block sorting)        ---*/
/*---                                            compress.c ---*/
/*-------------------------------------------------------------*/

/* Modified by Konstantin Isakov for the bzip2smp program */

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/

/*--
   CHANGES
   ~~~~~~~
   0.9.0 -- original version.

   0.9.0a/b -- no changes in this file.

   0.9.0c
      * changed setting of nGroups in sendMTFValues() so as to 
        do a bit better on small files
--*/


/*---------------------------------------------------*/
/*--- Bit stream I/O                              ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
void BZ2_bsInitWrite ( EState* s )
{
   s->bsLive = 0;
   s->bsBuff = 0;
}


/*---------------------------------------------------*/
static
void bsFinishWrite ( EState* s )
{
   while (s->bsLive > 0) {
      s->zbits[s->numZ] = (UChar)(s->bsBuff >> 24);
      s->numZ++;
      s->bsBuff <<= 8;
      s->bsLive -= 8;
   }
}


/*---------------------------------------------------*/
#define bsNEEDW(nz)                           \
{                                             \
   while (s->bsLive >= 8) {                   \
      s->zbits[s->numZ]                       \
         = (UChar)(s->bsBuff >> 24);          \
      s->numZ++;                              \
      s->bsBuff <<= 8;                        \
      s->bsLive -= 8;                         \
   }                                          \
}


/*---------------------------------------------------*/
static
__inline__
void bsW ( EState* s, Int32 n, UInt32 v )
{
   bsNEEDW ( n );
   s->bsBuff |= (v << (32 - s->bsLive - n));
   s->bsLive += n;
}


/*---------------------------------------------------*/
static
void bsPutUInt32 ( EState* s, UInt32 u )
{
   bsW ( s, 8, (u >> 24) & 0xffL );
   bsW ( s, 8, (u >> 16) & 0xffL );
   bsW ( s, 8, (u >>  8) & 0xffL );
   bsW ( s, 8,  u        & 0xffL );
}


/*---------------------------------------------------*/
static
void bsPutUChar ( EState* s, UChar c )
{
   bsW( s, 8, (UInt32)c );
}


/*---------------------------------------------------*/
/*--- The back end proper                         ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
static
void makeMaps_e ( EState* s )
{
   Int32 i;
   s->nInUse = 0;
   for (i = 0; i < 256; i++)
      if (s->inUse[i]) {
         s->unseqToSeq[i] = s->nInUse;
         s->nInUse++;
      }
}


/*---------------------------------------------------*/
static
void generateMTFValues ( EState* s )
{
   UChar   yy[256];
   Int32   i, j;
   Int32   zPend;
   Int32   wr;
   Int32   EOB;

   /* 
      After sorting (eg, here),
         s->arr1 [ 0 .. s->nblock-1 ] holds sorted order,
         and
         ((UChar*)s->arr2) [ 0 .. s->nblock-1 ] 
         holds the original block data.

      The first thing to do is generate the MTF values,
      and put them in
         ((UInt16*)s->arr1) [ 0 .. s->nblock-1 ].
      Because there are strictly fewer or equal MTF values
      than block values, ptr values in this area are overwritten
      with MTF values only when they are no longer needed.

      The final compressed bitstream is generated into the
      area starting at
         (UChar*) (&((UChar*)s->arr2)[s->nblock])

      These storage aliases are set up in bzCompressInit(),
      except for the last one, which is arranged in 
      compressBlock().
   */
   UInt32* ptr   = s->ptr;
   UChar* block  = s->block;
   UInt16* mtfv  = s->mtfv;

   makeMaps_e ( s );
   EOB = s->nInUse+1;

   for (i = 0; i <= EOB; i++) s->mtfFreq[i] = 0;

   wr = 0;
   zPend = 0;
   for (i = 0; i < s->nInUse; i++) yy[i] = (UChar) i;

   for (i = 0; i < s->nblock; i++) {
      UChar ll_i;
      AssertD ( wr <= i, "generateMTFValues(1)" );
      j = ptr[i]-1; if (j < 0) j += s->nblock;
      ll_i = s->unseqToSeq[block[j]];
      AssertD ( ll_i < s->nInUse, "generateMTFValues(2a)" );

      if (yy[0] == ll_i) { 
         zPend++;
      } else {

         if (zPend > 0) {
            zPend--;
            while (True) {
               if (zPend & 1) {
                  mtfv[wr] = BZ_RUNB; wr++; 
                  s->mtfFreq[BZ_RUNB]++; 
               } else {
                  mtfv[wr] = BZ_RUNA; wr++; 
                  s->mtfFreq[BZ_RUNA]++; 
               }
               if (zPend < 2) break;
               zPend = (zPend - 2) / 2;
            };
            zPend = 0;
         }
         {
            register UChar  rtmp;
            register UChar* ryy_j;
            register UChar  rll_i;
            rtmp  = yy[1];
            yy[1] = yy[0];
            ryy_j = &(yy[1]);
            rll_i = ll_i;
            while ( rll_i != rtmp ) {
               register UChar rtmp2;
               ryy_j++;
               rtmp2  = rtmp;
               rtmp   = *ryy_j;
               *ryy_j = rtmp2;
            };
            yy[0] = rtmp;
            j = ryy_j - &(yy[0]);
            mtfv[wr] = j+1; wr++; s->mtfFreq[j+1]++;
         }

      }
   }

   if (zPend > 0) {
      zPend--;
      while (True) {
         if (zPend & 1) {
            mtfv[wr] = BZ_RUNB; wr++; 
            s->mtfFreq[BZ_RUNB]++; 
         } else {
            mtfv[wr] = BZ_RUNA; wr++; 
            s->mtfFreq[BZ_RUNA]++; 
         }
         if (zPend < 2) break;
         zPend = (zPend - 2) / 2;
      };
      zPend = 0;
   }

   mtfv[wr] = EOB; wr++; s->mtfFreq[EOB]++;

   s->nMTF = wr;
}


/*---------------------------------------------------*/
#define BZ_LESSER_ICOST  0
#define BZ_GREATER_ICOST 15

static
void sendMTFValues_compute( EState* s )
{
   Int32 v, t, i, j, gs, ge, totc, bt, bc, iter;
   Int32 nSelectors, alphaSize, minLen, maxLen;
   Int32 nGroups;

   /*--
   UChar  len [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
   is a global since the decoder also needs it.

   Int32  code[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
   Int32  rfreq[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
   are also globals only used in this proc.
   Made global to keep stack frame size small.
   --*/


   UInt16 cost[BZ_N_GROUPS];
   Int32  fave[BZ_N_GROUPS];

   UInt16* mtfv = s->mtfv;

   if (s->verbosity >= 3)
      VPrintf3( "      %d in block, %d after MTF & 1-2 coding, "
                "%d+2 syms in use\n", 
                s->nblock, s->nMTF, s->nInUse );

   alphaSize = s->nInUse+2;
   for (t = 0; t < BZ_N_GROUPS; t++)
      for (v = 0; v < alphaSize; v++)
         s->len[t][v] = BZ_GREATER_ICOST;

   /*--- Decide how many coding tables to use ---*/
   AssertH ( s->nMTF > 0, 3001 );
   if (s->nMTF < 200)  nGroups = 2; else
   if (s->nMTF < 600)  nGroups = 3; else
   if (s->nMTF < 1200) nGroups = 4; else
   if (s->nMTF < 2400) nGroups = 5; else
                       nGroups = 6;

   /*--- Generate an initial set of coding tables ---*/
   { 
      Int32 nPart, remF, tFreq, aFreq;

      nPart = nGroups;
      remF  = s->nMTF;
      gs = 0;
      while (nPart > 0) {
         tFreq = remF / nPart;
         ge = gs-1;
         aFreq = 0;
         while (aFreq < tFreq && ge < alphaSize-1) {
            ge++;
            aFreq += s->mtfFreq[ge];
         }

         if (ge > gs 
             && nPart != nGroups && nPart != 1 
             && ((nGroups-nPart) % 2 == 1)) {
            aFreq -= s->mtfFreq[ge];
            ge--;
         }

         if (s->verbosity >= 3)
            VPrintf5( "      initial group %d, [%d .. %d], "
                      "has %d syms (%4.1f%%)\n",
                      nPart, gs, ge, aFreq, 
                      (100.0 * (float)aFreq) / (float)(s->nMTF) );
 
         for (v = 0; v < alphaSize; v++)
            if (v >= gs && v <= ge) 
               s->len[nPart-1][v] = BZ_LESSER_ICOST; else
               s->len[nPart-1][v] = BZ_GREATER_ICOST;
 
         nPart--;
         gs = ge+1;
         remF -= aFreq;
      }
   }

   /*--- 
      Iterate up to BZ_N_ITERS times to improve the tables.
   ---*/
   for (iter = 0; iter < BZ_N_ITERS; iter++) {

      for (t = 0; t < nGroups; t++) fave[t] = 0;

      for (t = 0; t < nGroups; t++)
         for (v = 0; v < alphaSize; v++)
            s->rfreq[t][v] = 0;

      /*---
        Set up an auxiliary length table which is used to fast-track
	the common case (nGroups == 6). 
      ---*/
      if (nGroups == 6) {
         for (v = 0; v < alphaSize; v++) {
            s->len_pack[v][0] = (s->len[1][v] << 16) | s->len[0][v];
            s->len_pack[v][1] = (s->len[3][v] << 16) | s->len[2][v];
            s->len_pack[v][2] = (s->len[5][v] << 16) | s->len[4][v];
	 }
      }

      nSelectors = 0;
      totc = 0;
      gs = 0;
      while (True) {

         /*--- Set group start & end marks. --*/
         if (gs >= s->nMTF) break;
         ge = gs + BZ_G_SIZE - 1; 
         if (ge >= s->nMTF) ge = s->nMTF-1;

         /*-- 
            Calculate the cost of this group as coded
            by each of the coding tables.
         --*/
         for (t = 0; t < nGroups; t++) cost[t] = 0;

         if (nGroups == 6 && 50 == ge-gs+1) {
            /*--- fast track the common case ---*/
            register UInt32 cost01, cost23, cost45;
            register UInt16 icv;
            cost01 = cost23 = cost45 = 0;

#           define BZ_ITER(nn)                \
               icv = mtfv[gs+(nn)];           \
               cost01 += s->len_pack[icv][0]; \
               cost23 += s->len_pack[icv][1]; \
               cost45 += s->len_pack[icv][2]; \

            BZ_ITER(0);  BZ_ITER(1);  BZ_ITER(2);  BZ_ITER(3);  BZ_ITER(4);
            BZ_ITER(5);  BZ_ITER(6);  BZ_ITER(7);  BZ_ITER(8);  BZ_ITER(9);
            BZ_ITER(10); BZ_ITER(11); BZ_ITER(12); BZ_ITER(13); BZ_ITER(14);
            BZ_ITER(15); BZ_ITER(16); BZ_ITER(17); BZ_ITER(18); BZ_ITER(19);
            BZ_ITER(20); BZ_ITER(21); BZ_ITER(22); BZ_ITER(23); BZ_ITER(24);
            BZ_ITER(25); BZ_ITER(26); BZ_ITER(27); BZ_ITER(28); BZ_ITER(29);
            BZ_ITER(30); BZ_ITER(31); BZ_ITER(32); BZ_ITER(33); BZ_ITER(34);
            BZ_ITER(35); BZ_ITER(36); BZ_ITER(37); BZ_ITER(38); BZ_ITER(39);
            BZ_ITER(40); BZ_ITER(41); BZ_ITER(42); BZ_ITER(43); BZ_ITER(44);
            BZ_ITER(45); BZ_ITER(46); BZ_ITER(47); BZ_ITER(48); BZ_ITER(49);

#           undef BZ_ITER

            cost[0] = cost01 & 0xffff; cost[1] = cost01 >> 16;
            cost[2] = cost23 & 0xffff; cost[3] = cost23 >> 16;
            cost[4] = cost45 & 0xffff; cost[5] = cost45 >> 16;

         } else {
	    /*--- slow version which correctly handles all situations ---*/
            for (i = gs; i <= ge; i++) { 
               UInt16 icv = mtfv[i];
               for (t = 0; t < nGroups; t++) cost[t] += s->len[t][icv];
            }
         }
 
         /*-- 
            Find the coding table which is best for this group,
            and record its identity in the selector table.
         --*/
         bc = 999999999; bt = -1;
         for (t = 0; t < nGroups; t++)
            if (cost[t] < bc) { bc = cost[t]; bt = t; };
         totc += bc;
         fave[bt]++;
         s->selector[nSelectors] = bt;
         nSelectors++;

         /*-- 
            Increment the symbol frequencies for the selected table.
          --*/
         if (nGroups == 6 && 50 == ge-gs+1) {
            /*--- fast track the common case ---*/

#           define BZ_ITUR(nn) s->rfreq[bt][ mtfv[gs+(nn)] ]++

            BZ_ITUR(0);  BZ_ITUR(1);  BZ_ITUR(2);  BZ_ITUR(3);  BZ_ITUR(4);
            BZ_ITUR(5);  BZ_ITUR(6);  BZ_ITUR(7);  BZ_ITUR(8);  BZ_ITUR(9);
            BZ_ITUR(10); BZ_ITUR(11); BZ_ITUR(12); BZ_ITUR(13); BZ_ITUR(14);
            BZ_ITUR(15); BZ_ITUR(16); BZ_ITUR(17); BZ_ITUR(18); BZ_ITUR(19);
            BZ_ITUR(20); BZ_ITUR(21); BZ_ITUR(22); BZ_ITUR(23); BZ_ITUR(24);
            BZ_ITUR(25); BZ_ITUR(26); BZ_ITUR(27); BZ_ITUR(28); BZ_ITUR(29);
            BZ_ITUR(30); BZ_ITUR(31); BZ_ITUR(32); BZ_ITUR(33); BZ_ITUR(34);
            BZ_ITUR(35); BZ_ITUR(36); BZ_ITUR(37); BZ_ITUR(38); BZ_ITUR(39);
            BZ_ITUR(40); BZ_ITUR(41); BZ_ITUR(42); BZ_ITUR(43); BZ_ITUR(44);
            BZ_ITUR(45); BZ_ITUR(46); BZ_ITUR(47); BZ_ITUR(48); BZ_ITUR(49);

#           undef BZ_ITUR

         } else {
	    /*--- slow version which correctly handles all situations ---*/
            for (i = gs; i <= ge; i++)
               s->rfreq[bt][ mtfv[i] ]++;
         }

         gs = ge+1;
      }
      if (s->verbosity >= 3) {
         VPrintf2 ( "      pass %d: size is %d, grp uses are ", 
                   iter+1, totc/8 );
         for (t = 0; t < nGroups; t++)
            VPrintf1 ( "%d ", fave[t] );
         VPrintf0 ( "\n" );
      }

      /*--
        Recompute the tables based on the accumulated frequencies.
      --*/
      /* maxLen was changed from 20 to 17 in bzip2-1.0.3.  See 
         comment in huffman.c for details. */
      for (t = 0; t < nGroups; t++)
         BZ2_hbMakeCodeLengths ( &(s->len[t][0]), &(s->rfreq[t][0]), 
                                 alphaSize, 17 /*20*/ );
   }


   AssertH( nGroups < 8, 3002 );
   AssertH( nSelectors < 32768 &&
            nSelectors <= (2 + (900000 / BZ_G_SIZE)),
            3003 );


   /*--- Compute MTF values for the selectors. ---*/
   {
      UChar pos[BZ_N_GROUPS], ll_i, tmp2, tmp;
      for (i = 0; i < nGroups; i++) pos[i] = i;
      for (i = 0; i < nSelectors; i++) {
         ll_i = s->selector[i];
         j = 0;
         tmp = pos[j];
         while ( ll_i != tmp ) {
            j++;
            tmp2 = tmp;
            tmp = pos[j];
            pos[j] = tmp2;
         };
         pos[0] = tmp;
         s->selectorMtf[i] = j;
      }
   };

   /*--- Assign actual codes for the tables. --*/
   for (t = 0; t < nGroups; t++) {
      minLen = 32;
      maxLen = 0;
      for (i = 0; i < alphaSize; i++) {
         if (s->len[t][i] > maxLen) maxLen = s->len[t][i];
         if (s->len[t][i] < minLen) minLen = s->len[t][i];
      }
      AssertH ( !(maxLen > 17 /*20*/ ), 3004 );
      AssertH ( !(minLen < 1),  3005 );
      BZ2_hbAssignCodes ( &(s->code[t][0]), &(s->len[t][0]), 
                          minLen, maxLen, alphaSize );
   }

   s->nGroups = nGroups;
   s->nSelectors = nSelectors;
   s->alphaSize = alphaSize;
}

static void sendMTFValues_save( EState* s )
{
  Int32 nBytes, selCtr;
  Int32 i, j, t, gs, ge;
  Int32 nGroups = s->nGroups;
  Int32 nSelectors = s->nSelectors;
  Int32 alphaSize = s->alphaSize;

  UInt16* mtfv = s->mtfv;

  /*--- Transmit the mapping table. ---*/
  { 
     Bool inUse16[16];
     for (i = 0; i < 16; i++) {
         inUse16[i] = False;
         for (j = 0; j < 16; j++)
            if (s->inUse[i * 16 + j]) inUse16[i] = True;
     }

     nBytes = s->numZ;
     for (i = 0; i < 16; i++)
        if (inUse16[i]) bsW(s,1,1); else bsW(s,1,0);

     for (i = 0; i < 16; i++)
        if (inUse16[i])
           for (j = 0; j < 16; j++) {
              if (s->inUse[i * 16 + j]) bsW(s,1,1); else bsW(s,1,0);
           }

     if (s->verbosity >= 3) 
        VPrintf1( "      bytes: mapping %d, ", s->numZ-nBytes );
  }

  /*--- Now the selectors. ---*/
  nBytes = s->numZ;
  bsW ( s, 3, nGroups );
  bsW ( s, 15, nSelectors );
  for (i = 0; i < nSelectors; i++) { 
     for (j = 0; j < s->selectorMtf[i]; j++) bsW(s,1,1);
     bsW(s,1,0);
  }
  if (s->verbosity >= 3)
     VPrintf1( "selectors %d, ", s->numZ-nBytes );

  /*--- Now the coding tables. ---*/
  nBytes = s->numZ;

  for (t = 0; t < nGroups; t++) {
     Int32 curr = s->len[t][0];
     bsW ( s, 5, curr );
     for (i = 0; i < alphaSize; i++) {
        while (curr < s->len[t][i]) { bsW(s,2,2); curr++; /* 10 */ };
        while (curr > s->len[t][i]) { bsW(s,2,3); curr--; /* 11 */ };
        bsW ( s, 1, 0 );
     }
  }

  if (s->verbosity >= 3)
     VPrintf1 ( "code lengths %d, ", s->numZ-nBytes );

  /*--- And finally, the block data proper ---*/
  nBytes = s->numZ;
  selCtr = 0;
  gs = 0;
  while (True) {
     if (gs >= s->nMTF) break;
     ge = gs + BZ_G_SIZE - 1; 
     if (ge >= s->nMTF) ge = s->nMTF-1;
     AssertH ( s->selector[selCtr] < nGroups, 3006 );

     if (nGroups == 6 && 50 == ge-gs+1) {
           /*--- fast track the common case ---*/
           UInt16 mtfv_i;
           UChar* s_len_sel_selCtr 
              = &(s->len[s->selector[selCtr]][0]);
           Int32* s_code_sel_selCtr
              = &(s->code[s->selector[selCtr]][0]);

#           define BZ_ITAH(nn)                      \
              mtfv_i = mtfv[gs+(nn)];              \
              bsW ( s,                             \
                    s_len_sel_selCtr[mtfv_i],      \
                    s_code_sel_selCtr[mtfv_i] )

           BZ_ITAH(0);  BZ_ITAH(1);  BZ_ITAH(2);  BZ_ITAH(3);  BZ_ITAH(4);
           BZ_ITAH(5);  BZ_ITAH(6);  BZ_ITAH(7);  BZ_ITAH(8);  BZ_ITAH(9);
           BZ_ITAH(10); BZ_ITAH(11); BZ_ITAH(12); BZ_ITAH(13); BZ_ITAH(14);
           BZ_ITAH(15); BZ_ITAH(16); BZ_ITAH(17); BZ_ITAH(18); BZ_ITAH(19);
           BZ_ITAH(20); BZ_ITAH(21); BZ_ITAH(22); BZ_ITAH(23); BZ_ITAH(24);
           BZ_ITAH(25); BZ_ITAH(26); BZ_ITAH(27); BZ_ITAH(28); BZ_ITAH(29);
           BZ_ITAH(30); BZ_ITAH(31); BZ_ITAH(32); BZ_ITAH(33); BZ_ITAH(34);
           BZ_ITAH(35); BZ_ITAH(36); BZ_ITAH(37); BZ_ITAH(38); BZ_ITAH(39);
           BZ_ITAH(40); BZ_ITAH(41); BZ_ITAH(42); BZ_ITAH(43); BZ_ITAH(44);
           BZ_ITAH(45); BZ_ITAH(46); BZ_ITAH(47); BZ_ITAH(48); BZ_ITAH(49);

#           undef BZ_ITAH

     } else {
  /*--- slow version which correctly handles all situations ---*/
        for (i = gs; i <= ge; i++) {
           bsW ( s, 
                 s->len  [s->selector[selCtr]] [mtfv[i]],
                 s->code [s->selector[selCtr]] [mtfv[i]] );
        }
     }


     gs = ge+1;
     selCtr++;
  }
  AssertH( selCtr == nSelectors, 3007 );

  if (s->verbosity >= 3)
     VPrintf1( "codes %d\n", s->numZ-nBytes );
}

/*
static
void sendMTFValues ( EState* s )
{
  sendMTFValues_compute( s );
  sendMTFValues_save( s );
}
*/

/*---------------------------------------------------*/
void BZ2_compressBlock_compute ( EState* s )
{
   if (s->nblock > 0) {

      BZ_FINALISE_CRC ( s->blockCRC );
      s->combinedCRC = (s->combinedCRC << 1) | (s->combinedCRC >> 31);
      s->combinedCRC ^= s->blockCRC;
      if (s->blockNo > 1) s->numZ = 0;

      if (s->verbosity >= 2)
         VPrintf4( "    block %d: crc = 0x%08x, "
                   "combined CRC = 0x%08x, size = %d\n",
                   s->blockNo, s->blockCRC, s->combinedCRC, s->nblock );

      BZ2_blockSort ( s );

      generateMTFValues ( s );
      sendMTFValues_compute ( s );
   }
}

void BZ2_compressBlock_save ( EState* s, Bool is_last_block )
{
  /*s->zbits = (UChar*) (&((UChar*)s->arr2)[s->nblock]); XXX */
  s->zbits = (UChar*)(s->arr2);

  /*-- If this is the first block, create the stream header. --*/
  if (s->blockNo == 1) {
     BZ2_bsInitWrite ( s );
     bsPutUChar ( s, BZ_HDR_B );
     bsPutUChar ( s, BZ_HDR_Z );
     bsPutUChar ( s, BZ_HDR_h );
     bsPutUChar ( s, (UChar)(BZ_HDR_0 + s->blockSize100k) );
  }

  if (s->nblock > 0) {

     bsPutUChar ( s, 0x31 ); bsPutUChar ( s, 0x41 );
     bsPutUChar ( s, 0x59 ); bsPutUChar ( s, 0x26 );
     bsPutUChar ( s, 0x53 ); bsPutUChar ( s, 0x59 );

     /*-- Now the block's CRC, so it is in a known place. --*/
     bsPutUInt32 ( s, s->blockCRC );

     /*-- 
        Now a single bit indicating (non-)randomisation. 
        As of version 0.9.5, we use a better sorting algorithm
        which makes randomisation unnecessary.  So always set
        the randomised bit to 'no'.  Of course, the decoder
        still needs to be able to handle randomised blocks
        so as to maintain backwards compatibility with
        older versions of bzip2.
     --*/
     bsW(s,1,0);

     bsW ( s, 24, s->origPtr );
     sendMTFValues_save ( s );
  }


  /*-- If this is the last block, add the stream trailer. --*/
  if (is_last_block) {

     bsPutUChar ( s, 0x17 ); bsPutUChar ( s, 0x72 );
     bsPutUChar ( s, 0x45 ); bsPutUChar ( s, 0x38 );
     bsPutUChar ( s, 0x50 ); bsPutUChar ( s, 0x90 );
     bsPutUInt32 ( s, s->combinedCRC );
     if (s->verbosity >= 2)
        VPrintf1( "    final combined CRC = 0x%08x\n   ", s->combinedCRC );
     bsFinishWrite ( s );
  }
}

void BZ2_compressBlock ( EState* s, Bool is_last_block )
{
  BZ2_compressBlock_compute( s );
  BZ2_compressBlock_save( s, is_last_block );
}


/*-------------------------------------------------------------*/
/*--- end                                        compress.c ---*/
/*-------------------------------------------------------------*/

#ifdef __linux__

static void noHt( void )
{
  note( 1, "No hyperthreading detected.\n" );
}

static char cpuInfoFile[] = "/proc/cpuinfo";
static char siblingsString[] = "siblings\t: ";
static char cpuCoresString[] = "cpu cores\t: ";

int isHtPresent( void )
{
  char buf[ 4096 ];
  char * str;
  FILE * f;

  f = fopen( cpuInfoFile, "r" );

  if ( f == NULL )
  {
    note( 1, "Failed to open %s, assuming there is no hyperthreading\n",
             cpuInfoFile );

    return 0;
  }

  buf[ fread( buf, 1, sizeof( buf ) - 1, f ) ] = 0;

  fclose( f );

  if ( ( str = strstr( buf, siblingsString ) ) )
  {
    int siblings;
    int cores;

    if ( sscanf( str + strlen( siblingsString ), "%d", & siblings ) != 1 )
    {
      note( 1, "Failed to read the number of siblings in %s, assuming no ht\n",
               cpuInfoFile );
      return 0;
    }

    str = strstr( buf, cpuCoresString );

    if ( str == NULL )
    {
      note( 1, "No cpu cores string found in %s, but it should be there, assuming no ht\n",
               cpuInfoFile );

      return 0;
    }

    if ( sscanf( str + strlen( cpuCoresString ), "%d", & cores ) != 1 )
    {
      note( 1, "Failed to read the number of cores in %s, assuming no ht\n",
               cpuInfoFile );
      return 0;
    }

    switch( siblings / cores )
    {
      case 1:
        noHt();
        return 0;

      case 2:
        note( 1, "Hyperthreading detected.\n" );
        return 1;

      default:
        note( 1, "Strange number of siblings (%d) in %s, but assuming ht is present\n",
                 siblings / cores, cpuInfoFile );

        return 1;
    }
  }
  else
  {
    noHt();
    return 0;
  }
}
#else
int isHtPresent( void )
{
  note( 1, "Assuming that hyperthreading is not present.\n" );
  return 0;
}
#endif



/*-------------------------------------------------------------*/
/*--- Block sorting machinery                               ---*/
/*---                                           blocksort.c ---*/
/*-------------------------------------------------------------*/

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.

  To get some idea how the block sorting algorithms in this file 
  work, read my paper 
     On the Performance of BWT Sorting Algorithms
  in Proceedings of the IEEE Data Compression Conference 2000,
  Snowbird, Utah, USA, 27-30 March 2000.  The main sort in this
  file implements the algorithm called  cache  in the paper.
--*/


//#include "bzlib_private.h"

/*---------------------------------------------*/
/*--- Fallback O(N log(N)^2) sorting        ---*/
/*--- algorithm, for repetitive blocks      ---*/
/*---------------------------------------------*/

/*---------------------------------------------*/
static 
__inline__
void fallbackSimpleSort ( UInt32* fmap, 
                          UInt32* eclass, 
                          Int32   lo, 
                          Int32   hi )
{
   Int32 i, j, tmp;
   UInt32 ec_tmp;

   if (lo == hi) return;

   if (hi - lo > 3) {
      for ( i = hi-4; i >= lo; i-- ) {
         tmp = fmap[i];
         ec_tmp = eclass[tmp];
         for ( j = i+4; j <= hi && ec_tmp > eclass[fmap[j]]; j += 4 )
            fmap[j-4] = fmap[j];
         fmap[j-4] = tmp;
      }
   }

   for ( i = hi-1; i >= lo; i-- ) {
      tmp = fmap[i];
      ec_tmp = eclass[tmp];
      for ( j = i+1; j <= hi && ec_tmp > eclass[fmap[j]]; j++ )
         fmap[j-1] = fmap[j];
      fmap[j-1] = tmp;
   }
}


/*---------------------------------------------*/
#define fswap(zz1, zz2) \
   { Int32 zztmp = zz1; zz1 = zz2; zz2 = zztmp; }

#define fvswap(zzp1, zzp2, zzn)       \
{                                     \
   Int32 yyp1 = (zzp1);               \
   Int32 yyp2 = (zzp2);               \
   Int32 yyn  = (zzn);                \
   while (yyn > 0) {                  \
      fswap(fmap[yyp1], fmap[yyp2]);  \
      yyp1++; yyp2++; yyn--;          \
   }                                  \
}


#define fmin(a,b) ((a) < (b)) ? (a) : (b)

#define fpush(lz,hz) { stackLo[sp] = lz; \
                       stackHi[sp] = hz; \
                       sp++; }

#define fpop(lz,hz) { sp--;              \
                      lz = stackLo[sp];  \
                      hz = stackHi[sp]; }

#define FALLBACK_QSORT_SMALL_THRESH 10
#define FALLBACK_QSORT_STACK_SIZE   100


static
void fallbackQSort3 ( UInt32* fmap, 
                      UInt32* eclass,
                      Int32   loSt, 
                      Int32   hiSt )
{
   Int32 unLo, unHi, ltLo, gtHi, n, m;
   Int32 sp, lo, hi;
   UInt32 med, r, r3;
   Int32 stackLo[FALLBACK_QSORT_STACK_SIZE];
   Int32 stackHi[FALLBACK_QSORT_STACK_SIZE];

   r = 0;

   sp = 0;
   fpush ( loSt, hiSt );

   while (sp > 0) {

      AssertH ( sp < FALLBACK_QSORT_STACK_SIZE, 1004 );

      fpop ( lo, hi );
      if (hi - lo < FALLBACK_QSORT_SMALL_THRESH) {
         fallbackSimpleSort ( fmap, eclass, lo, hi );
         continue;
      }

      /* Random partitioning.  Median of 3 sometimes fails to
         avoid bad cases.  Median of 9 seems to help but 
         looks rather expensive.  This too seems to work but
         is cheaper.  Guidance for the magic constants 
         7621 and 32768 is taken from Sedgewick's algorithms
         book, chapter 35.
      */
      r = ((r * 7621) + 1) % 32768;
      r3 = r % 3;
      if (r3 == 0) med = eclass[fmap[lo]]; else
      if (r3 == 1) med = eclass[fmap[(lo+hi)>>1]]; else
                   med = eclass[fmap[hi]];

      unLo = ltLo = lo;
      unHi = gtHi = hi;

      while (1) {
         while (1) {
            if (unLo > unHi) break;
            n = (Int32)eclass[fmap[unLo]] - (Int32)med;
            if (n == 0) { 
               fswap(fmap[unLo], fmap[ltLo]); 
               ltLo++; unLo++; 
               continue; 
            };
            if (n > 0) break;
            unLo++;
         }
         while (1) {
            if (unLo > unHi) break;
            n = (Int32)eclass[fmap[unHi]] - (Int32)med;
            if (n == 0) { 
               fswap(fmap[unHi], fmap[gtHi]); 
               gtHi--; unHi--; 
               continue; 
            };
            if (n < 0) break;
            unHi--;
         }
         if (unLo > unHi) break;
         fswap(fmap[unLo], fmap[unHi]); unLo++; unHi--;
      }

      AssertD ( unHi == unLo-1, "fallbackQSort3(2)" );

      if (gtHi < ltLo) continue;

      n = fmin(ltLo-lo, unLo-ltLo); fvswap(lo, unLo-n, n);
      m = fmin(hi-gtHi, gtHi-unHi); fvswap(unLo, hi-m+1, m);

      n = lo + unLo - ltLo - 1;
      m = hi - (gtHi - unHi) + 1;

      if (n - lo > hi - m) {
         fpush ( lo, n );
         fpush ( m, hi );
      } else {
         fpush ( m, hi );
         fpush ( lo, n );
      }
   }
}

#undef fmin
#undef fpush
#undef fpop
#undef fswap
#undef fvswap
#undef FALLBACK_QSORT_SMALL_THRESH
#undef FALLBACK_QSORT_STACK_SIZE


/*---------------------------------------------*/
/* Pre:
      nblock > 0
      eclass exists for [0 .. nblock-1]
      ((UChar*)eclass) [0 .. nblock-1] holds block
      ptr exists for [0 .. nblock-1]

   Post:
      ((UChar*)eclass) [0 .. nblock-1] holds block
      All other areas of eclass destroyed
      fmap [0 .. nblock-1] holds sorted order
      bhtab [ 0 .. 2+(nblock/32) ] destroyed
*/

#define       SET_BH(zz)  bhtab[(zz) >> 5] |= (1 << ((zz) & 31))
#define     CLEAR_BH(zz)  bhtab[(zz) >> 5] &= ~(1 << ((zz) & 31))
#define     ISSET_BH(zz)  (bhtab[(zz) >> 5] & (1 << ((zz) & 31)))
#define      WORD_BH(zz)  bhtab[(zz) >> 5]
#define UNALIGNED_BH(zz)  ((zz) & 0x01f)

static
void fallbackSort ( UInt32* fmap, 
                    UInt32* eclass, 
                    UInt32* bhtab,
                    Int32   nblock,
                    Int32   verb )
{
   Int32 ftab[257];
   Int32 ftabCopy[256];
   Int32 H, i, j, k, l, r, cc, cc1;
   Int32 nNotDone;
   Int32 nBhtab;
   UChar* eclass8 = (UChar*)eclass;

   /*--
      Initial 1-char radix sort to generate
      initial fmap and initial BH bits.
   --*/
   if (verb >= 4)
      VPrintf0 ( "        bucket sorting ...\n" );
   for (i = 0; i < 257;    i++) ftab[i] = 0;
   for (i = 0; i < nblock; i++) ftab[eclass8[i]]++;
   for (i = 0; i < 256;    i++) ftabCopy[i] = ftab[i];
   for (i = 1; i < 257;    i++) ftab[i] += ftab[i-1];

   for (i = 0; i < nblock; i++) {
      j = eclass8[i];
      k = ftab[j] - 1;
      ftab[j] = k;
      fmap[k] = i;
   }

   nBhtab = 2 + (nblock / 32);
   for (i = 0; i < nBhtab; i++) bhtab[i] = 0;
   for (i = 0; i < 256; i++) SET_BH(ftab[i]);

   /*--
      Inductively refine the buckets.  Kind-of an
      "exponential radix sort" (!), inspired by the
      Manber-Myers suffix array construction algorithm.
   --*/

   /*-- set sentinel bits for block-end detection --*/
   for (i = 0; i < 32; i++) { 
      SET_BH(nblock + 2*i);
      CLEAR_BH(nblock + 2*i + 1);
   }

   /*-- the log(N) loop --*/
   H = 1;
   while (1) {

      if (verb >= 4) 
         VPrintf1 ( "        depth %6d has ", H );

      j = 0;
      for (i = 0; i < nblock; i++) {
         if (ISSET_BH(i)) j = i;
         k = fmap[i] - H; if (k < 0) k += nblock;
         eclass[k] = j;
      }

      nNotDone = 0;
      r = -1;
      while (1) {

	 /*-- find the next non-singleton bucket --*/
         k = r + 1;
         while (ISSET_BH(k) && UNALIGNED_BH(k)) k++;
         if (ISSET_BH(k)) {
            while (WORD_BH(k) == 0xffffffff) k += 32;
            while (ISSET_BH(k)) k++;
         }
         l = k - 1;
         if (l >= nblock) break;
         while (!ISSET_BH(k) && UNALIGNED_BH(k)) k++;
         if (!ISSET_BH(k)) {
            while (WORD_BH(k) == 0x00000000) k += 32;
            while (!ISSET_BH(k)) k++;
         }
         r = k - 1;
         if (r >= nblock) break;

         /*-- now [l, r] bracket current bucket --*/
         if (r > l) {
            nNotDone += (r - l + 1);
            fallbackQSort3 ( fmap, eclass, l, r );

            /*-- scan bucket and generate header bits-- */
            cc = -1;
            for (i = l; i <= r; i++) {
               cc1 = eclass[fmap[i]];
               if (cc != cc1) { SET_BH(i); cc = cc1; };
            }
         }
      }

      if (verb >= 4) 
         VPrintf1 ( "%6d unresolved strings\n", nNotDone );

      H *= 2;
      if (H > nblock || nNotDone == 0) break;
   }

   /*-- 
      Reconstruct the original block in
      eclass8 [0 .. nblock-1], since the
      previous phase destroyed it.
   --*/
   if (verb >= 4)
      VPrintf0 ( "        reconstructing block ...\n" );
   j = 0;
   for (i = 0; i < nblock; i++) {
      while (ftabCopy[j] == 0) j++;
      ftabCopy[j]--;
      eclass8[fmap[i]] = (UChar)j;
   }
   AssertH ( j < 256, 1005 );
}

#undef       SET_BH
#undef     CLEAR_BH
#undef     ISSET_BH
#undef      WORD_BH
#undef UNALIGNED_BH


/*---------------------------------------------*/
/*--- The main, O(N^2 log(N)) sorting       ---*/
/*--- algorithm.  Faster for "normal"       ---*/
/*--- non-repetitive blocks.                ---*/
/*---------------------------------------------*/

/*---------------------------------------------*/
static
__inline__
Bool mainGtU ( UInt32  i1, 
               UInt32  i2,
               UChar*  block, 
               UInt16* quadrant,
               UInt32  nblock,
               Int32*  budget )
{
   Int32  k;
   UChar  c1, c2;
   UInt16 s1, s2;

   AssertD ( i1 != i2, "mainGtU" );
   /* 1 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 2 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 3 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 4 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 5 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 6 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 7 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 8 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 9 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 10 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 11 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;
   /* 12 */
   c1 = block[i1]; c2 = block[i2];
   if (c1 != c2) return (c1 > c2);
   i1++; i2++;

   k = nblock + 8;

   do {
      /* 1 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 2 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 3 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 4 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 5 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 6 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 7 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;
      /* 8 */
      c1 = block[i1]; c2 = block[i2];
      if (c1 != c2) return (c1 > c2);
      s1 = quadrant[i1]; s2 = quadrant[i2];
      if (s1 != s2) return (s1 > s2);
      i1++; i2++;

      if (i1 >= nblock) i1 -= nblock;
      if (i2 >= nblock) i2 -= nblock;

      k -= 8;
      (*budget)--;
   }
      while (k >= 0);

   return False;
}


/*---------------------------------------------*/
/*--
   Knuth's increments seem to work better
   than Incerpi-Sedgewick here.  Possibly
   because the number of elems to sort is
   usually small, typically <= 20.
--*/
static
Int32 incs[14] = { 1, 4, 13, 40, 121, 364, 1093, 3280,
                   9841, 29524, 88573, 265720,
                   797161, 2391484 };

static
void mainSimpleSort ( UInt32* ptr,
                      UChar*  block,
                      UInt16* quadrant,
                      Int32   nblock,
                      Int32   lo, 
                      Int32   hi, 
                      Int32   d,
                      Int32*  budget )
{
   Int32 i, j, h, bigN, hp;
   UInt32 v;

   bigN = hi - lo + 1;
   if (bigN < 2) return;

   hp = 0;
   while (incs[hp] < bigN) hp++;
   hp--;

   for (; hp >= 0; hp--) {
      h = incs[hp];

      i = lo + h;
      while (True) {

         /*-- copy 1 --*/
         if (i > hi) break;
         v = ptr[i];
         j = i;
         while ( mainGtU ( 
                    ptr[j-h]+d, v+d, block, quadrant, nblock, budget 
                 ) ) {
            ptr[j] = ptr[j-h];
            j = j - h;
            if (j <= (lo + h - 1)) break;
         }
         ptr[j] = v;
         i++;

         /*-- copy 2 --*/
         if (i > hi) break;
         v = ptr[i];
         j = i;
         while ( mainGtU ( 
                    ptr[j-h]+d, v+d, block, quadrant, nblock, budget 
                 ) ) {
            ptr[j] = ptr[j-h];
            j = j - h;
            if (j <= (lo + h - 1)) break;
         }
         ptr[j] = v;
         i++;

         /*-- copy 3 --*/
         if (i > hi) break;
         v = ptr[i];
         j = i;
         while ( mainGtU ( 
                    ptr[j-h]+d, v+d, block, quadrant, nblock, budget 
                 ) ) {
            ptr[j] = ptr[j-h];
            j = j - h;
            if (j <= (lo + h - 1)) break;
         }
         ptr[j] = v;
         i++;

         if (*budget < 0) return;
      }
   }
}


/*---------------------------------------------*/
/*--
   The following is an implementation of
   an elegant 3-way quicksort for strings,
   described in a paper "Fast Algorithms for
   Sorting and Searching Strings", by Robert
   Sedgewick and Jon L. Bentley.
--*/

#define mswap(zz1, zz2) \
   { Int32 zztmp = zz1; zz1 = zz2; zz2 = zztmp; }

#define mvswap(zzp1, zzp2, zzn)       \
{                                     \
   Int32 yyp1 = (zzp1);               \
   Int32 yyp2 = (zzp2);               \
   Int32 yyn  = (zzn);                \
   while (yyn > 0) {                  \
      mswap(ptr[yyp1], ptr[yyp2]);    \
      yyp1++; yyp2++; yyn--;          \
   }                                  \
}

static 
__inline__
UChar mmed3 ( UChar a, UChar b, UChar c )
{
   UChar t;
   if (a > b) { t = a; a = b; b = t; };
   if (b > c) { 
      b = c;
      if (a > b) b = a;
   }
   return b;
}

#define mmin(a,b) ((a) < (b)) ? (a) : (b)

#define mpush(lz,hz,dz) { stackLo[sp] = lz; \
                          stackHi[sp] = hz; \
                          stackD [sp] = dz; \
                          sp++; }

#define mpop(lz,hz,dz) { sp--;             \
                         lz = stackLo[sp]; \
                         hz = stackHi[sp]; \
                         dz = stackD [sp]; }


#define mnextsize(az) (nextHi[az]-nextLo[az])

#define mnextswap(az,bz)                                        \
   { Int32 tz;                                                  \
     tz = nextLo[az]; nextLo[az] = nextLo[bz]; nextLo[bz] = tz; \
     tz = nextHi[az]; nextHi[az] = nextHi[bz]; nextHi[bz] = tz; \
     tz = nextD [az]; nextD [az] = nextD [bz]; nextD [bz] = tz; }


#define MAIN_QSORT_SMALL_THRESH 20
#define MAIN_QSORT_DEPTH_THRESH (BZ_N_RADIX + BZ_N_QSORT)
#define MAIN_QSORT_STACK_SIZE 100

static
void mainQSort3 ( UInt32* ptr,
                  UChar*  block,
                  UInt16* quadrant,
                  Int32   nblock,
                  Int32   loSt, 
                  Int32   hiSt, 
                  Int32   dSt,
                  Int32*  budget )
{
   Int32 unLo, unHi, ltLo, gtHi, n, m, med;
   Int32 sp, lo, hi, d;

   Int32 stackLo[MAIN_QSORT_STACK_SIZE];
   Int32 stackHi[MAIN_QSORT_STACK_SIZE];
   Int32 stackD [MAIN_QSORT_STACK_SIZE];

   Int32 nextLo[3];
   Int32 nextHi[3];
   Int32 nextD [3];

   sp = 0;
   mpush ( loSt, hiSt, dSt );

   while (sp > 0) {

      AssertH ( sp < MAIN_QSORT_STACK_SIZE, 1001 );

      mpop ( lo, hi, d );
      if (hi - lo < MAIN_QSORT_SMALL_THRESH || 
          d > MAIN_QSORT_DEPTH_THRESH) {
         mainSimpleSort ( ptr, block, quadrant, nblock, lo, hi, d, budget );
         if (*budget < 0) return;
         continue;
      }

      med = (Int32) 
            mmed3 ( block[ptr[ lo         ]+d],
                    block[ptr[ hi         ]+d],
                    block[ptr[ (lo+hi)>>1 ]+d] );

      unLo = ltLo = lo;
      unHi = gtHi = hi;

      while (True) {
         while (True) {
            if (unLo > unHi) break;
            n = ((Int32)block[ptr[unLo]+d]) - med;
            if (n == 0) { 
               mswap(ptr[unLo], ptr[ltLo]); 
               ltLo++; unLo++; continue; 
            };
            if (n >  0) break;
            unLo++;
         }
         while (True) {
            if (unLo > unHi) break;
            n = ((Int32)block[ptr[unHi]+d]) - med;
            if (n == 0) { 
               mswap(ptr[unHi], ptr[gtHi]); 
               gtHi--; unHi--; continue; 
            };
            if (n <  0) break;
            unHi--;
         }
         if (unLo > unHi) break;
         mswap(ptr[unLo], ptr[unHi]); unLo++; unHi--;
      }

      AssertD ( unHi == unLo-1, "mainQSort3(2)" );

      if (gtHi < ltLo) {
         mpush(lo, hi, d+1 );
         continue;
      }

      n = mmin(ltLo-lo, unLo-ltLo); mvswap(lo, unLo-n, n);
      m = mmin(hi-gtHi, gtHi-unHi); mvswap(unLo, hi-m+1, m);

      n = lo + unLo - ltLo - 1;
      m = hi - (gtHi - unHi) + 1;

      nextLo[0] = lo;  nextHi[0] = n;   nextD[0] = d;
      nextLo[1] = m;   nextHi[1] = hi;  nextD[1] = d;
      nextLo[2] = n+1; nextHi[2] = m-1; nextD[2] = d+1;

      if (mnextsize(0) < mnextsize(1)) mnextswap(0,1);
      if (mnextsize(1) < mnextsize(2)) mnextswap(1,2);
      if (mnextsize(0) < mnextsize(1)) mnextswap(0,1);

      AssertD (mnextsize(0) >= mnextsize(1), "mainQSort3(8)" );
      AssertD (mnextsize(1) >= mnextsize(2), "mainQSort3(9)" );

      mpush (nextLo[0], nextHi[0], nextD[0]);
      mpush (nextLo[1], nextHi[1], nextD[1]);
      mpush (nextLo[2], nextHi[2], nextD[2]);
   }
}

#undef mswap
#undef mvswap
#undef mpush
#undef mpop
#undef mmin
#undef mnextsize
#undef mnextswap
#undef MAIN_QSORT_SMALL_THRESH
#undef MAIN_QSORT_DEPTH_THRESH
#undef MAIN_QSORT_STACK_SIZE


/*---------------------------------------------*/
/* Pre:
      nblock > N_OVERSHOOT
      block32 exists for [0 .. nblock-1 +N_OVERSHOOT]
      ((UChar*)block32) [0 .. nblock-1] holds block
      ptr exists for [0 .. nblock-1]

   Post:
      ((UChar*)block32) [0 .. nblock-1] holds block
      All other areas of block32 destroyed
      ftab [0 .. 65536 ] destroyed
      ptr [0 .. nblock-1] holds sorted order
      if (*budget < 0), sorting was abandoned
*/

#define BIGFREQ(b) (ftab[((b)+1) << 8] - ftab[(b) << 8])
#define SETMASK (1 << 21)
#define CLEARMASK (~(SETMASK))

static
void mainSort ( UInt32* ptr, 
                UChar*  block,
                UInt16* quadrant, 
                UInt32* ftab,
                Int32   nblock,
                Int32   verb,
                Int32*  budget )
{
   Int32  i, j, k, ss, sb;
   Int32  runningOrder[256];
   Bool   bigDone[256];
   Int32  copyStart[256];
   Int32  copyEnd  [256];
   UChar  c1;
   Int32  numQSorted;
   UInt16 s;
   if (verb >= 4) VPrintf0 ( "        main sort initialise ...\n" );

   /*-- set up the 2-byte frequency table --*/
   for (i = 65536; i >= 0; i--) ftab[i] = 0;

   j = block[0] << 8;
   i = nblock-1;
   for (; i >= 3; i -= 4) {
      quadrant[i] = 0;
      j = (j >> 8) | ( ((UInt16)block[i]) << 8);
      ftab[j]++;
      quadrant[i-1] = 0;
      j = (j >> 8) | ( ((UInt16)block[i-1]) << 8);
      ftab[j]++;
      quadrant[i-2] = 0;
      j = (j >> 8) | ( ((UInt16)block[i-2]) << 8);
      ftab[j]++;
      quadrant[i-3] = 0;
      j = (j >> 8) | ( ((UInt16)block[i-3]) << 8);
      ftab[j]++;
   }
   for (; i >= 0; i--) {
      quadrant[i] = 0;
      j = (j >> 8) | ( ((UInt16)block[i]) << 8);
      ftab[j]++;
   }

   /*-- (emphasises close relationship of block & quadrant) --*/
   for (i = 0; i < BZ_N_OVERSHOOT; i++) {
      block   [nblock+i] = block[i];
      quadrant[nblock+i] = 0;
   }

   if (verb >= 4) VPrintf0 ( "        bucket sorting ...\n" );

   /*-- Complete the initial radix sort --*/
   for (i = 1; i <= 65536; i++) ftab[i] += ftab[i-1];

   s = block[0] << 8;
   i = nblock-1;
   for (; i >= 3; i -= 4) {
      s = (s >> 8) | (block[i] << 8);
      j = ftab[s] -1;
      ftab[s] = j;
      ptr[j] = i;
      s = (s >> 8) | (block[i-1] << 8);
      j = ftab[s] -1;
      ftab[s] = j;
      ptr[j] = i-1;
      s = (s >> 8) | (block[i-2] << 8);
      j = ftab[s] -1;
      ftab[s] = j;
      ptr[j] = i-2;
      s = (s >> 8) | (block[i-3] << 8);
      j = ftab[s] -1;
      ftab[s] = j;
      ptr[j] = i-3;
   }
   for (; i >= 0; i--) {
      s = (s >> 8) | (block[i] << 8);
      j = ftab[s] -1;
      ftab[s] = j;
      ptr[j] = i;
   }

   /*--
      Now ftab contains the first loc of every small bucket.
      Calculate the running order, from smallest to largest
      big bucket.
   --*/
   for (i = 0; i <= 255; i++) {
      bigDone     [i] = False;
      runningOrder[i] = i;
   }

   {
      Int32 vv;
      Int32 h = 1;
      do h = 3 * h + 1; while (h <= 256);
      do {
         h = h / 3;
         for (i = h; i <= 255; i++) {
            vv = runningOrder[i];
            j = i;
            while ( BIGFREQ(runningOrder[j-h]) > BIGFREQ(vv) ) {
               runningOrder[j] = runningOrder[j-h];
               j = j - h;
               if (j <= (h - 1)) goto zero;
            }
            zero:
            runningOrder[j] = vv;
         }
      } while (h != 1);
   }

   /*--
      The main sorting loop.
   --*/

   numQSorted = 0;

   for (i = 0; i <= 255; i++) {

      /*--
         Process big buckets, starting with the least full.
         Basically this is a 3-step process in which we call
         mainQSort3 to sort the small buckets [ss, j], but
         also make a big effort to avoid the calls if we can.
      --*/
      ss = runningOrder[i];

      /*--
         Step 1:
         Complete the big bucket [ss] by quicksorting
         any unsorted small buckets [ss, j], for j != ss.  
         Hopefully previous pointer-scanning phases have already
         completed many of the small buckets [ss, j], so
         we don't have to sort them at all.
      --*/
      for (j = 0; j <= 255; j++) {
         if (j != ss) {
            sb = (ss << 8) + j;
            if ( ! (ftab[sb] & SETMASK) ) {
               Int32 lo = ftab[sb]   & CLEARMASK;
               Int32 hi = (ftab[sb+1] & CLEARMASK) - 1;
               if (hi > lo) {
                  if (verb >= 4)
                     VPrintf4 ( "        qsort [0x%x, 0x%x]   "
                                "done %d   this %d\n",
                                ss, j, numQSorted, hi - lo + 1 );
                  mainQSort3 ( 
                     ptr, block, quadrant, nblock, 
                     lo, hi, BZ_N_RADIX, budget 
                  );   
                  numQSorted += (hi - lo + 1);
                  if (*budget < 0) return;
               }
            }
            ftab[sb] |= SETMASK;
         }
      }

      AssertH ( !bigDone[ss], 1006 );

      /*--
         Step 2:
         Now scan this big bucket [ss] so as to synthesise the
         sorted order for small buckets [t, ss] for all t,
         including, magically, the bucket [ss,ss] too.
         This will avoid doing Real Work in subsequent Step 1's.
      --*/
      {
         for (j = 0; j <= 255; j++) {
            copyStart[j] =  ftab[(j << 8) + ss]     & CLEARMASK;
            copyEnd  [j] = (ftab[(j << 8) + ss + 1] & CLEARMASK) - 1;
         }
         for (j = ftab[ss << 8] & CLEARMASK; j < copyStart[ss]; j++) {
            k = ptr[j]-1; if (k < 0) k += nblock;
            c1 = block[k];
            if (!bigDone[c1])
               ptr[ copyStart[c1]++ ] = k;
         }
         for (j = (ftab[(ss+1) << 8] & CLEARMASK) - 1; j > copyEnd[ss]; j--) {
            k = ptr[j]-1; if (k < 0) k += nblock;
            c1 = block[k];
            if (!bigDone[c1]) 
               ptr[ copyEnd[c1]-- ] = k;
         }
      }

      AssertH ( (copyStart[ss]-1 == copyEnd[ss])
                || 
                /* Extremely rare case missing in bzip2-1.0.0 and 1.0.1.
                   Necessity for this case is demonstrated by compressing 
                   a sequence of approximately 48.5 million of character 
                   251; 1.0.0/1.0.1 will then die here. */
                (copyStart[ss] == 0 && copyEnd[ss] == nblock-1),
                1007 )

      for (j = 0; j <= 255; j++) ftab[(j << 8) + ss] |= SETMASK;

      /*--
         Step 3:
         The [ss] big bucket is now done.  Record this fact,
         and update the quadrant descriptors.  Remember to
         update quadrants in the overshoot area too, if
         necessary.  The "if (i < 255)" test merely skips
         this updating for the last bucket processed, since
         updating for the last bucket is pointless.

         The quadrant array provides a way to incrementally
         cache sort orderings, as they appear, so as to 
         make subsequent comparisons in fullGtU() complete
         faster.  For repetitive blocks this makes a big
         difference (but not big enough to be able to avoid
         the fallback sorting mechanism, exponential radix sort).

         The precise meaning is: at all times:

            for 0 <= i < nblock and 0 <= j <= nblock

            if block[i] != block[j], 

               then the relative values of quadrant[i] and 
                    quadrant[j] are meaningless.

               else {
                  if quadrant[i] < quadrant[j]
                     then the string starting at i lexicographically
                     precedes the string starting at j

                  else if quadrant[i] > quadrant[j]
                     then the string starting at j lexicographically
                     precedes the string starting at i

                  else
                     the relative ordering of the strings starting
                     at i and j has not yet been determined.
               }
      --*/
      bigDone[ss] = True;

      if (i < 255) {
         Int32 bbStart  = ftab[ss << 8] & CLEARMASK;
         Int32 bbSize   = (ftab[(ss+1) << 8] & CLEARMASK) - bbStart;
         Int32 shifts   = 0;

         while ((bbSize >> shifts) > 65534) shifts++;

         for (j = bbSize-1; j >= 0; j--) {
            Int32 a2update     = ptr[bbStart + j];
            UInt16 qVal        = (UInt16)(j >> shifts);
            quadrant[a2update] = qVal;
            if (a2update < BZ_N_OVERSHOOT)
               quadrant[a2update + nblock] = qVal;
         }
         AssertH ( ((bbSize-1) >> shifts) <= 65535, 1002 );
      }

   }

   if (verb >= 4)
      VPrintf3 ( "        %d pointers, %d sorted, %d scanned\n",
                 nblock, numQSorted, nblock - numQSorted );
}

#undef BIGFREQ
#undef SETMASK
#undef CLEARMASK


/*---------------------------------------------*/
/* Pre:
      nblock > 0
      arr2 exists for [0 .. nblock-1 +N_OVERSHOOT]
      ((UChar*)arr2)  [0 .. nblock-1] holds block
      arr1 exists for [0 .. nblock-1]

   Post:
      ((UChar*)arr2) [0 .. nblock-1] holds block
      All other areas of block destroyed
      ftab [ 0 .. 65536 ] destroyed
      arr1 [0 .. nblock-1] holds sorted order
*/
void BZ2_blockSort ( EState* s )
{
   UInt32* ptr    = s->ptr; 
   UChar*  block  = s->block;
   UInt32* ftab   = s->ftab;
   Int32   nblock = s->nblock;
   Int32   verb   = s->verbosity;
   Int32   wfact  = s->workFactor;
   UInt16* quadrant;
   Int32   budget;
   Int32   budgetInit;
   Int32   i;

   if (nblock < 10000) {
      fallbackSort ( s->arr1, s->arr2, ftab, nblock, verb );
   } else {
      /* Calculate the location for quadrant, remembering to get
         the alignment right.  Assumes that &(block[0]) is at least
         2-byte aligned -- this should be ok since block is really
         the first section of arr2.
      */
      i = nblock+BZ_N_OVERSHOOT;
      if (i & 1) i++;
      quadrant = (UInt16*)(&(block[i]));

      /* (wfact-1) / 3 puts the default-factor-30
         transition point at very roughly the same place as 
         with v0.1 and v0.9.0.  
         Not that it particularly matters any more, since the
         resulting compressed stream is now the same regardless
         of whether or not we use the main sort or fallback sort.
      */
      if (wfact < 1  ) wfact = 1;
      if (wfact > 100) wfact = 100;
      budgetInit = nblock * ((wfact-1) / 3);
      budget = budgetInit;

      mainSort ( ptr, block, quadrant, ftab, nblock, verb, &budget );
      if (verb >= 3) 
         VPrintf3 ( "      %d work, %d block, ratio %5.2f\n",
                    budgetInit - budget,
                    nblock, 
                    (float)(budgetInit - budget) /
                    (float)(nblock==0 ? 1 : nblock) ); 
      if (budget < 0) {
         if (verb >= 2) 
            VPrintf0 ( "    too repetitive; using fallback"
                       " sorting algorithm\n" );
         fallbackSort ( s->arr1, s->arr2, ftab, nblock, verb );
      }
   }

   s->origPtr = -1;
   for (i = 0; i < s->nblock; i++)
      if (ptr[i] == 0)
         { s->origPtr = i; break; };

   AssertH( s->origPtr != -1, 1003 );
}


/*-------------------------------------------------------------*/
/*--- end                                       blocksort.c ---*/
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*--- Decompression machinery                               ---*/
/*---                                          decompress.c ---*/
/*-------------------------------------------------------------*/

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/


//#include "bzlib_private.h"


/*---------------------------------------------------*/
static
void makeMaps_d ( DState* s )
{
   Int32 i;
   s->nInUse = 0;
   for (i = 0; i < 256; i++)
      if (s->inUse[i]) {
         s->seqToUnseq[s->nInUse] = i;
         s->nInUse++;
      }
}


/*---------------------------------------------------*/
#define RETURN(rrr)                               \
   { retVal = rrr; goto save_state_and_return; };

#define GET_BITS(lll,vvv,nnn)                     \
   case lll: s->state = lll;                      \
   while (True) {                                 \
      if (s->bsLive >= nnn) {                     \
         UInt32 v;                                \
         v = (s->bsBuff >>                        \
             (s->bsLive-nnn)) & ((1 << nnn)-1);   \
         s->bsLive -= nnn;                        \
         vvv = v;                                 \
         break;                                   \
      }                                           \
      if (s->strm->avail_in == 0) RETURN(BZ_OK);  \
      s->bsBuff                                   \
         = (s->bsBuff << 8) |                     \
           ((UInt32)                              \
              (*((UChar*)(s->strm->next_in))));   \
      s->bsLive += 8;                             \
      s->strm->next_in++;                         \
      s->strm->avail_in--;                        \
      s->strm->total_in_lo32++;                   \
      if (s->strm->total_in_lo32 == 0)            \
         s->strm->total_in_hi32++;                \
   }

#define GET_UCHAR(lll,uuu)                        \
   GET_BITS(lll,uuu,8)

#define GET_BIT(lll,uuu)                          \
   GET_BITS(lll,uuu,1)

/*---------------------------------------------------*/
#define GET_MTF_VAL(label1,label2,lval)           \
{                                                 \
   if (groupPos == 0) {                           \
      groupNo++;                                  \
      if (groupNo >= nSelectors)                  \
         RETURN(BZ_DATA_ERROR);                   \
      groupPos = BZ_G_SIZE;                       \
      gSel = s->selector[groupNo];                \
      gMinlen = s->minLens[gSel];                 \
      gLimit = &(s->limit[gSel][0]);              \
      gPerm = &(s->perm[gSel][0]);                \
      gBase = &(s->base[gSel][0]);                \
   }                                              \
   groupPos--;                                    \
   zn = gMinlen;                                  \
   GET_BITS(label1, zvec, zn);                    \
   while (1) {                                    \
      if (zn > 20 /* the longest code */)         \
         RETURN(BZ_DATA_ERROR);                   \
      if (zvec <= gLimit[zn]) break;              \
      zn++;                                       \
      GET_BIT(label2, zj);                        \
      zvec = (zvec << 1) | zj;                    \
   };                                             \
   if (zvec - gBase[zn] < 0                       \
       || zvec - gBase[zn] >= BZ_MAX_ALPHA_SIZE)  \
      RETURN(BZ_DATA_ERROR);                      \
   lval = gPerm[zvec - gBase[zn]];                \
}


/*---------------------------------------------------*/
Int32 BZ2_decompress ( DState* s )
{
   UChar      uc;
   Int32      retVal;
   Int32      minLen, maxLen;
   bz_stream* strm = s->strm;

   /* stuff that needs to be saved/restored */
   Int32  i;
   Int32  j;
   Int32  t;
   Int32  alphaSize;
   Int32  nGroups;
   Int32  nSelectors;
   Int32  EOB;
   Int32  groupNo;
   Int32  groupPos;
   Int32  nextSym;
   Int32  nblockMAX;
   Int32  nblock;
   Int32  es;
   Int32  N;
   Int32  curr;
   Int32  zt;
   Int32  zn; 
   Int32  zvec;
   Int32  zj;
   Int32  gSel;
   Int32  gMinlen;
   Int32* gLimit;
   Int32* gBase;
   Int32* gPerm;

   if (s->state == BZ_X_MAGIC_1) {
      /*initialise the save area*/
      s->save_i           = 0;
      s->save_j           = 0;
      s->save_t           = 0;
      s->save_alphaSize   = 0;
      s->save_nGroups     = 0;
      s->save_nSelectors  = 0;
      s->save_EOB         = 0;
      s->save_groupNo     = 0;
      s->save_groupPos    = 0;
      s->save_nextSym     = 0;
      s->save_nblockMAX   = 0;
      s->save_nblock      = 0;
      s->save_es          = 0;
      s->save_N           = 0;
      s->save_curr        = 0;
      s->save_zt          = 0;
      s->save_zn          = 0;
      s->save_zvec        = 0;
      s->save_zj          = 0;
      s->save_gSel        = 0;
      s->save_gMinlen     = 0;
      s->save_gLimit      = NULL;
      s->save_gBase       = NULL;
      s->save_gPerm       = NULL;
   }

   /*restore from the save area*/
   i           = s->save_i;
   j           = s->save_j;
   t           = s->save_t;
   alphaSize   = s->save_alphaSize;
   nGroups     = s->save_nGroups;
   nSelectors  = s->save_nSelectors;
   EOB         = s->save_EOB;
   groupNo     = s->save_groupNo;
   groupPos    = s->save_groupPos;
   nextSym     = s->save_nextSym;
   nblockMAX   = s->save_nblockMAX;
   nblock      = s->save_nblock;
   es          = s->save_es;
   N           = s->save_N;
   curr        = s->save_curr;
   zt          = s->save_zt;
   zn          = s->save_zn; 
   zvec        = s->save_zvec;
   zj          = s->save_zj;
   gSel        = s->save_gSel;
   gMinlen     = s->save_gMinlen;
   gLimit      = s->save_gLimit;
   gBase       = s->save_gBase;
   gPerm       = s->save_gPerm;

   retVal = BZ_OK;

   switch (s->state) {

      GET_UCHAR(BZ_X_MAGIC_1, uc);
      if (uc != BZ_HDR_B) RETURN(BZ_DATA_ERROR_MAGIC);

      GET_UCHAR(BZ_X_MAGIC_2, uc);
      if (uc != BZ_HDR_Z) RETURN(BZ_DATA_ERROR_MAGIC);

      GET_UCHAR(BZ_X_MAGIC_3, uc)
      if (uc != BZ_HDR_h) RETURN(BZ_DATA_ERROR_MAGIC);

      GET_BITS(BZ_X_MAGIC_4, s->blockSize100k, 8)
      if (s->blockSize100k < (BZ_HDR_0 + 1) || 
          s->blockSize100k > (BZ_HDR_0 + 9)) RETURN(BZ_DATA_ERROR_MAGIC);
      s->blockSize100k -= BZ_HDR_0;

      if (s->smallDecompress) {
         s->ll16 = BZALLOC( s->blockSize100k * 100000 * sizeof(UInt16) );
         s->ll4  = BZALLOC( 
                      ((1 + s->blockSize100k * 100000) >> 1) * sizeof(UChar) 
                   );
         if (s->ll16 == NULL || s->ll4 == NULL) RETURN(BZ_MEM_ERROR);
      } else {
         s->tt  = BZALLOC( s->blockSize100k * 100000 * sizeof(Int32) );
         if (s->tt == NULL) RETURN(BZ_MEM_ERROR);
      }

      GET_UCHAR(BZ_X_BLKHDR_1, uc);

      if (uc == 0x17) goto endhdr_2;
      if (uc != 0x31) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_2, uc);
      if (uc != 0x41) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_3, uc);
      if (uc != 0x59) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_4, uc);
      if (uc != 0x26) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_5, uc);
      if (uc != 0x53) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_6, uc);
      if (uc != 0x59) RETURN(BZ_DATA_ERROR);

      s->currBlockNo++;
      if (s->verbosity >= 2)
         VPrintf1 ( "\n    [%d: huff+mtf ", s->currBlockNo );
 
      s->storedBlockCRC = 0;
      GET_UCHAR(BZ_X_BCRC_1, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_BCRC_2, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_BCRC_3, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_BCRC_4, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);

      GET_BITS(BZ_X_RANDBIT, s->blockRandomised, 1);

      s->origPtr = 0;
      GET_UCHAR(BZ_X_ORIGPTR_1, uc);
      s->origPtr = (s->origPtr << 8) | ((Int32)uc);
      GET_UCHAR(BZ_X_ORIGPTR_2, uc);
      s->origPtr = (s->origPtr << 8) | ((Int32)uc);
      GET_UCHAR(BZ_X_ORIGPTR_3, uc);
      s->origPtr = (s->origPtr << 8) | ((Int32)uc);

      if (s->origPtr < 0)
         RETURN(BZ_DATA_ERROR);
      if (s->origPtr > 10 + 100000*s->blockSize100k) 
         RETURN(BZ_DATA_ERROR);

      /*--- Receive the mapping table ---*/
      for (i = 0; i < 16; i++) {
         GET_BIT(BZ_X_MAPPING_1, uc);
         if (uc == 1) 
            s->inUse16[i] = True; else 
            s->inUse16[i] = False;
      }

      for (i = 0; i < 256; i++) s->inUse[i] = False;

      for (i = 0; i < 16; i++)
         if (s->inUse16[i])
            for (j = 0; j < 16; j++) {
               GET_BIT(BZ_X_MAPPING_2, uc);
               if (uc == 1) s->inUse[i * 16 + j] = True;
            }
      makeMaps_d ( s );
      if (s->nInUse == 0) RETURN(BZ_DATA_ERROR);
      alphaSize = s->nInUse+2;

      /*--- Now the selectors ---*/
      GET_BITS(BZ_X_SELECTOR_1, nGroups, 3);
      if (nGroups < 2 || nGroups > 6) RETURN(BZ_DATA_ERROR);
      GET_BITS(BZ_X_SELECTOR_2, nSelectors, 15);
      if (nSelectors < 1) RETURN(BZ_DATA_ERROR);
      for (i = 0; i < nSelectors; i++) {
         j = 0;
         while (True) {
            GET_BIT(BZ_X_SELECTOR_3, uc);
            if (uc == 0) break;
            j++;
            if (j >= nGroups) RETURN(BZ_DATA_ERROR);
         }
         s->selectorMtf[i] = j;
      }

      /*--- Undo the MTF values for the selectors. ---*/
      {
         UChar pos[BZ_N_GROUPS], tmp, v;
         for (v = 0; v < nGroups; v++) pos[v] = v;
   
         for (i = 0; i < nSelectors; i++) {
            v = s->selectorMtf[i];
            tmp = pos[v];
            while (v > 0) { pos[v] = pos[v-1]; v--; }
            pos[0] = tmp;
            s->selector[i] = tmp;
         }
      }

      /*--- Now the coding tables ---*/
      for (t = 0; t < nGroups; t++) {
         GET_BITS(BZ_X_CODING_1, curr, 5);
         for (i = 0; i < alphaSize; i++) {
            while (True) {
               if (curr < 1 || curr > 20) RETURN(BZ_DATA_ERROR);
               GET_BIT(BZ_X_CODING_2, uc);
               if (uc == 0) break;
               GET_BIT(BZ_X_CODING_3, uc);
               if (uc == 0) curr++; else curr--;
            }
            s->len[t][i] = curr;
         }
      }

      /*--- Create the Huffman decoding tables ---*/
      for (t = 0; t < nGroups; t++) {
         minLen = 32;
         maxLen = 0;
         for (i = 0; i < alphaSize; i++) {
            if (s->len[t][i] > maxLen) maxLen = s->len[t][i];
            if (s->len[t][i] < minLen) minLen = s->len[t][i];
         }
         BZ2_hbCreateDecodeTables ( 
            &(s->limit[t][0]), 
            &(s->base[t][0]), 
            &(s->perm[t][0]), 
            &(s->len[t][0]),
            minLen, maxLen, alphaSize
         );
         s->minLens[t] = minLen;
      }

      /*--- Now the MTF values ---*/

      EOB      = s->nInUse+1;
      nblockMAX = 100000 * s->blockSize100k;
      groupNo  = -1;
      groupPos = 0;

      for (i = 0; i <= 255; i++) s->unzftab[i] = 0;

      /*-- MTF init --*/
      {
         Int32 ii, jj, kk;
         kk = MTFA_SIZE-1;
         for (ii = 256 / MTFL_SIZE - 1; ii >= 0; ii--) {
            for (jj = MTFL_SIZE-1; jj >= 0; jj--) {
               s->mtfa[kk] = (UChar)(ii * MTFL_SIZE + jj);
               kk--;
            }
            s->mtfbase[ii] = kk + 1;
         }
      }
      /*-- end MTF init --*/

      nblock = 0;
      GET_MTF_VAL(BZ_X_MTF_1, BZ_X_MTF_2, nextSym);

      while (True) {

         if (nextSym == EOB) break;

         if (nextSym == BZ_RUNA || nextSym == BZ_RUNB) {

            es = -1;
            N = 1;
            do {
               if (nextSym == BZ_RUNA) es = es + (0+1) * N; else
               if (nextSym == BZ_RUNB) es = es + (1+1) * N;
               N = N * 2;
               GET_MTF_VAL(BZ_X_MTF_3, BZ_X_MTF_4, nextSym);
            }
               while (nextSym == BZ_RUNA || nextSym == BZ_RUNB);

            es++;
            uc = s->seqToUnseq[ s->mtfa[s->mtfbase[0]] ];
            s->unzftab[uc] += es;

            if (s->smallDecompress)
               while (es > 0) {
                  if (nblock >= nblockMAX) RETURN(BZ_DATA_ERROR);
                  s->ll16[nblock] = (UInt16)uc;
                  nblock++;
                  es--;
               }
            else
               while (es > 0) {
                  if (nblock >= nblockMAX) RETURN(BZ_DATA_ERROR);
                  s->tt[nblock] = (UInt32)uc;
                  nblock++;
                  es--;
               };

            continue;

         } else {

            if (nblock >= nblockMAX) RETURN(BZ_DATA_ERROR);

            /*-- uc = MTF ( nextSym-1 ) --*/
            {
               Int32 ii, jj, kk, pp, lno, off;
               UInt32 nn;
               nn = (UInt32)(nextSym - 1);

               if (nn < MTFL_SIZE) {
                  /* avoid general-case expense */
                  pp = s->mtfbase[0];
                  uc = s->mtfa[pp+nn];
                  while (nn > 3) {
                     Int32 z = pp+nn;
                     s->mtfa[(z)  ] = s->mtfa[(z)-1];
                     s->mtfa[(z)-1] = s->mtfa[(z)-2];
                     s->mtfa[(z)-2] = s->mtfa[(z)-3];
                     s->mtfa[(z)-3] = s->mtfa[(z)-4];
                     nn -= 4;
                  }
                  while (nn > 0) { 
                     s->mtfa[(pp+nn)] = s->mtfa[(pp+nn)-1]; nn--; 
                  };
                  s->mtfa[pp] = uc;
               } else { 
                  /* general case */
                  lno = nn / MTFL_SIZE;
                  off = nn % MTFL_SIZE;
                  pp = s->mtfbase[lno] + off;
                  uc = s->mtfa[pp];
                  while (pp > s->mtfbase[lno]) { 
                     s->mtfa[pp] = s->mtfa[pp-1]; pp--; 
                  };
                  s->mtfbase[lno]++;
                  while (lno > 0) {
                     s->mtfbase[lno]--;
                     s->mtfa[s->mtfbase[lno]] 
                        = s->mtfa[s->mtfbase[lno-1] + MTFL_SIZE - 1];
                     lno--;
                  }
                  s->mtfbase[0]--;
                  s->mtfa[s->mtfbase[0]] = uc;
                  if (s->mtfbase[0] == 0) {
                     kk = MTFA_SIZE-1;
                     for (ii = 256 / MTFL_SIZE-1; ii >= 0; ii--) {
                        for (jj = MTFL_SIZE-1; jj >= 0; jj--) {
                           s->mtfa[kk] = s->mtfa[s->mtfbase[ii] + jj];
                           kk--;
                        }
                        s->mtfbase[ii] = kk + 1;
                     }
                  }
               }
            }
            /*-- end uc = MTF ( nextSym-1 ) --*/

            s->unzftab[s->seqToUnseq[uc]]++;
            if (s->smallDecompress)
               s->ll16[nblock] = (UInt16)(s->seqToUnseq[uc]); else
               s->tt[nblock]   = (UInt32)(s->seqToUnseq[uc]);
            nblock++;

            GET_MTF_VAL(BZ_X_MTF_5, BZ_X_MTF_6, nextSym);
            continue;
         }
      }

      /* Now we know what nblock is, we can do a better sanity
         check on s->origPtr.
      */
      if (s->origPtr < 0 || s->origPtr >= nblock)
         RETURN(BZ_DATA_ERROR);

      /*-- Set up cftab to facilitate generation of T^(-1) --*/
      s->cftab[0] = 0;
      for (i = 1; i <= 256; i++) s->cftab[i] = s->unzftab[i-1];
      for (i = 1; i <= 256; i++) s->cftab[i] += s->cftab[i-1];
      for (i = 0; i <= 256; i++) {
         if (s->cftab[i] < 0 || s->cftab[i] > nblock) {
            /* s->cftab[i] can legitimately be == nblock */
            RETURN(BZ_DATA_ERROR);
         }
      }

      s->state_out_len = 0;
      s->state_out_ch  = 0;
      BZ_INITIALISE_CRC ( s->calculatedBlockCRC );
      s->state = BZ_X_OUTPUT;
      if (s->verbosity >= 2) VPrintf0 ( "rt+rld" );

      if (s->smallDecompress) {

         /*-- Make a copy of cftab, used in generation of T --*/
         for (i = 0; i <= 256; i++) s->cftabCopy[i] = s->cftab[i];

         /*-- compute the T vector --*/
         for (i = 0; i < nblock; i++) {
            uc = (UChar)(s->ll16[i]);
            SET_LL(i, s->cftabCopy[uc]);
            s->cftabCopy[uc]++;
         }

         /*-- Compute T^(-1) by pointer reversal on T --*/
         i = s->origPtr;
         j = GET_LL(i);
         do {
            Int32 tmp = GET_LL(j);
            SET_LL(j, i);
            i = j;
            j = tmp;
         }
            while (i != s->origPtr);

         s->tPos = s->origPtr;
         s->nblock_used = 0;
         if (s->blockRandomised) {
            BZ_RAND_INIT_MASK;
            BZ_GET_SMALL(s->k0); s->nblock_used++;
            BZ_RAND_UPD_MASK; s->k0 ^= BZ_RAND_MASK; 
         } else {
            BZ_GET_SMALL(s->k0); s->nblock_used++;
         }

      } else {

         /*-- compute the T^(-1) vector --*/
         for (i = 0; i < nblock; i++) {
            uc = (UChar)(s->tt[i] & 0xff);
            s->tt[s->cftab[uc]] |= (i << 8);
            s->cftab[uc]++;
         }

         s->tPos = s->tt[s->origPtr] >> 8;
         s->nblock_used = 0;
         if (s->blockRandomised) {
            BZ_RAND_INIT_MASK;
            BZ_GET_FAST(s->k0); s->nblock_used++;
            BZ_RAND_UPD_MASK; s->k0 ^= BZ_RAND_MASK; 
         } else {
            BZ_GET_FAST(s->k0); s->nblock_used++;
         }

      }

      RETURN(BZ_OK);



    endhdr_2:

      GET_UCHAR(BZ_X_ENDHDR_2, uc);
      if (uc != 0x72) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_3, uc);
      if (uc != 0x45) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_4, uc);
      if (uc != 0x38) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_5, uc);
      if (uc != 0x50) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_6, uc);
      if (uc != 0x90) RETURN(BZ_DATA_ERROR);

      s->storedCombinedCRC = 0;
      GET_UCHAR(BZ_X_CCRC_1, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_CCRC_2, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_CCRC_3, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_CCRC_4, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);

      s->state = BZ_X_IDLE;
      RETURN(BZ_STREAM_END);

      default: AssertH ( False, 4001 );
   }

   AssertH ( False, 4002 );

   save_state_and_return:

   s->save_i           = i;
   s->save_j           = j;
   s->save_t           = t;
   s->save_alphaSize   = alphaSize;
   s->save_nGroups     = nGroups;
   s->save_nSelectors  = nSelectors;
   s->save_EOB         = EOB;
   s->save_groupNo     = groupNo;
   s->save_groupPos    = groupPos;
   s->save_nextSym     = nextSym;
   s->save_nblockMAX   = nblockMAX;
   s->save_nblock      = nblock;
   s->save_es          = es;
   s->save_N           = N;
   s->save_curr        = curr;
   s->save_zt          = zt;
   s->save_zn          = zn;
   s->save_zvec        = zvec;
   s->save_zj          = zj;
   s->save_gSel        = gSel;
   s->save_gMinlen     = gMinlen;
   s->save_gLimit      = gLimit;
   s->save_gBase       = gBase;
   s->save_gPerm       = gPerm;

   return retVal;   
}


/*-------------------------------------------------------------*/
/*--- end                                      decompress.c ---*/
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*--- Library top-level functions.                          ---*/
/*---                                               bzlib.c ---*/
/*-------------------------------------------------------------*/

/* Modified by Konstantin Isakov for the bzip2smp program */

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/

/*--
   CHANGES
   ~~~~~~~
   0.9.0 -- original version.

   0.9.0a/b -- no changes in this file.

   0.9.0c
      * made zero-length BZ_FLUSH work correctly in bzCompress().
      * fixed bzWrite/bzRead to ignore zero-length requests.
      * fixed bzread to correctly handle read requests after EOF.
      * wrong parameter order in call to bzDecompressInit in
        bzBuffToBuffDecompress.  Fixed.
--*/

//#include "bzlib_private.h"


/*---------------------------------------------------*/
/*--- Compression stuff                           ---*/
/*---------------------------------------------------*/


/*---------------------------------------------------*/
#ifndef BZ_NO_STDIO
void BZ2_bz__AssertH__fail ( int errcode )
{
   fprintf(stderr, 
      "\n\nbzip2/libbzip2: internal error number %d.\n"
      "This is a bug in bzip2/libbzip2, %s.\n"
      "Please report it to me at: jseward@acm.org.  If this happened\n"
      "when you were using some program which uses libbzip2 as a\n"
      "component, you should also report this bug to the author(s)\n"
      "of that program.  Please make an effort to report this bug;\n"
      "timely and accurate bug reports eventually lead to higher\n"
      "quality software.  Thanks.  Julian Seward, 30 December 2001.\n\n",
      errcode,
      BZ2_bzlibVersion()
   );

   if (errcode == 1007) {
   fprintf(stderr,
      "\n*** A special note about internal error number 1007 ***\n"
      "\n"
      "Experience suggests that a common cause of i.e. 1007\n"
      "is unreliable memory or other hardware.  The 1007 assertion\n"
      "just happens to cross-check the results of huge numbers of\n"
      "memory reads/writes, and so acts (unintendedly) as a stress\n"
      "test of your memory system.\n"
      "\n"
      "I suggest the following: try compressing the file again,\n"
      "possibly monitoring progress in detail with the -vv flag.\n"
      "\n"
      "* If the error cannot be reproduced, and/or happens at different\n"
      "  points in compression, you may have a flaky memory system.\n"
      "  Try a memory-test program.  I have used Memtest86\n"
      "  (www.memtest86.com).  At the time of writing it is free (GPLd).\n"
      "  Memtest86 tests memory much more thorougly than your BIOSs\n"
      "  power-on test, and may find failures that the BIOS doesn't.\n"
      "\n"
      "* If the error can be repeatably reproduced, this is a bug in\n"
      "  bzip2, and I would very much like to hear about it.  Please\n"
      "  let me know, and, ideally, save a copy of the file causing the\n"
      "  problem -- without which I will be unable to investigate it.\n"
      "\n"
   );
   }

   exit(3);
}
#endif


/*---------------------------------------------------*/
static
int bz_config_ok ( void )
{
   if (sizeof(int)   != 4) return 0;
   if (sizeof(short) != 2) return 0;
   if (sizeof(char)  != 1) return 0;
   return 1;
}


/*---------------------------------------------------*/
static
void* default_bzalloc ( void* opaque, Int32 items, Int32 size )
{
   void* v = malloc ( items * size );
   return v;
}

static
void default_bzfree ( void* opaque, void* addr )
{
   if (addr != NULL) free ( addr );
}


/*---------------------------------------------------*/
static
void prepare_new_block ( EState* s )
{
   Int32 i;
   s->nblock = 0;
   s->numZ = 0;
   s->state_out_pos = 0;
   BZ_INITIALISE_CRC ( s->blockCRC );
   for (i = 0; i < 256; i++) s->inUse[i] = False;
   s->blockNo++;
}


/*---------------------------------------------------*/
static
void init_RL ( EState* s )
{
   s->state_in_ch  = 256;
   s->state_in_len = 0;
}


static
Bool isempty_RL ( EState* s )
{
   if (s->state_in_ch < 256 && s->state_in_len > 0)
      return False; else
      return True;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzCompressInit) 
                    ( bz_stream* strm, 
                     int        blockSize100k,
                     int        verbosity,
                     int        workFactor,
                     int        delayedAllocation )
{
   Int32   n;
   EState* s;

   if (!bz_config_ok()) return BZ_CONFIG_ERROR;

   if (strm == NULL || 
       blockSize100k < 1 || blockSize100k > 9 ||
       workFactor < 0 || workFactor > 250)
     return BZ_PARAM_ERROR;

   if (workFactor == 0) workFactor = 30;
   if (strm->bzalloc == NULL) strm->bzalloc = default_bzalloc;
   if (strm->bzfree == NULL) strm->bzfree = default_bzfree;

   s = BZALLOC( sizeof(EState) );
   if (s == NULL) return BZ_MEM_ERROR;
   s->strm = strm;

   s->arr1 = NULL;
   s->arr2 = NULL;
   s->ftab = NULL;

   n       = 100000 * blockSize100k;

   if ( !delayedAllocation )
     s->arr1 = BZALLOC( n                  * sizeof(UInt32) );
   s->arr2 = BZALLOC( (n+BZ_N_OVERSHOOT) * sizeof(UInt32) );
   s->ftab = BZALLOC( 65537              * sizeof(UInt32) );

   s->dontFreeArr2 = 0;

   if ( ( !delayedAllocation && s->arr1 == NULL ) ||
        s->arr2 == NULL || s->ftab == NULL) {
      if (s->arr1 != NULL) BZFREE(s->arr1);
      if (s->arr2 != NULL) BZFREE(s->arr2);
      if (s->ftab != NULL) BZFREE(s->ftab);
      if (s       != NULL) BZFREE(s);
      return BZ_MEM_ERROR;
   }

   s->blockNo           = 0;
   s->state             = BZ_S_INPUT;
   s->mode              = BZ_M_RUNNING;
   s->combinedCRC       = 0;
   s->blockSize100k     = blockSize100k;
   s->nblockMAX         = 100000 * blockSize100k - 19;
   s->verbosity         = verbosity;
   s->workFactor        = workFactor;

   s->block             = (UChar*)s->arr2;
   s->mtfv              = (UInt16*)s->arr1;
   s->zbits             = NULL;
   s->ptr               = (UInt32*)s->arr1;

   strm->state          = s;
   strm->total_in_lo32  = 0;
   strm->total_in_hi32  = 0;
   strm->total_out_lo32 = 0;
   strm->total_out_hi32 = 0;
   init_RL ( s );
   prepare_new_block ( s );
   return BZ_OK;
}


/*---------------------------------------------------*/
static
void add_pair_to_block ( EState* s )
{
   Int32 i;
   UChar ch = (UChar)(s->state_in_ch);
   for (i = 0; i < s->state_in_len; i++) {
      BZ_UPDATE_CRC( s->blockCRC, ch );
   }
   s->inUse[s->state_in_ch] = True;
   switch (s->state_in_len) {
      case 1:
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         break;
      case 2:
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         break;
      case 3:
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         break;
      default:
         s->inUse[s->state_in_len-4] = True;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = ((UChar)(s->state_in_len-4));
         s->nblock++;
         break;
   }
}


/*---------------------------------------------------*/
static
void flush_RL ( EState* s )
{
   if (s->state_in_ch < 256) add_pair_to_block ( s );
   init_RL ( s );
}


/*---------------------------------------------------*/
#define ADD_CHAR_TO_BLOCK(zs,zchh0)               \
{                                                 \
   UInt32 zchh = (UInt32)(zchh0);                 \
   /*-- fast track the common case --*/           \
   if (zchh != zs->state_in_ch &&                 \
       zs->state_in_len == 1) {                   \
      UChar ch = (UChar)(zs->state_in_ch);        \
      BZ_UPDATE_CRC( zs->blockCRC, ch );          \
      zs->inUse[zs->state_in_ch] = True;          \
      zs->block[zs->nblock] = (UChar)ch;          \
      zs->nblock++;                               \
      zs->state_in_ch = zchh;                     \
   }                                              \
   else                                           \
   /*-- general, uncommon cases --*/              \
   if (zchh != zs->state_in_ch ||                 \
      zs->state_in_len == 255) {                  \
      if (zs->state_in_ch < 256)                  \
         add_pair_to_block ( zs );                \
      zs->state_in_ch = zchh;                     \
      zs->state_in_len = 1;                       \
   } else {                                       \
      zs->state_in_len++;                         \
   }                                              \
}


/*---------------------------------------------------*/
static
Bool copy_input_until_stop ( EState* s )
{
   Bool progress_in = False;

   if (s->mode == BZ_M_RUNNING) {

      /*-- fast track the common case --*/
      while (True) {
         /*-- block full? --*/
         if (s->nblock >= s->nblockMAX) break;
         /*-- no input? --*/
         if (s->strm->avail_in == 0) break;
         progress_in = True;
         ADD_CHAR_TO_BLOCK ( s, (UInt32)(*((UChar*)(s->strm->next_in))) ); 
         s->strm->next_in++;
         s->strm->avail_in--;
         s->strm->total_in_lo32++;
         if (s->strm->total_in_lo32 == 0) s->strm->total_in_hi32++;
      }

   } else {

      /*-- general, uncommon case --*/
      while (True) {
         /*-- block full? --*/
         if (s->nblock >= s->nblockMAX) break;
         /*-- no input? --*/
         if (s->strm->avail_in == 0) break;
         /*-- flush/finish end? --*/
         if (s->avail_in_expect == 0) break;
         progress_in = True;
         ADD_CHAR_TO_BLOCK ( s, (UInt32)(*((UChar*)(s->strm->next_in))) ); 
         s->strm->next_in++;
         s->strm->avail_in--;
         s->strm->total_in_lo32++;
         if (s->strm->total_in_lo32 == 0) s->strm->total_in_hi32++;
         s->avail_in_expect--;
      }
   }
   return progress_in;
}


/*---------------------------------------------------*/
static
Bool copy_output_until_stop ( EState* s )
{
   Bool progress_out = False;

   while (True) {

      /*-- no output space? --*/
      if (s->strm->avail_out == 0) break;

      /*-- block done? --*/
      if (s->state_out_pos >= s->numZ) break;

      progress_out = True;
      *(s->strm->next_out) = s->zbits[s->state_out_pos];
      s->state_out_pos++;
      s->strm->avail_out--;
      s->strm->next_out++;
      s->strm->total_out_lo32++;
      if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
   }

   return progress_out;
}

static void check_delayed_alloc( EState * s )
{
  /* If the allocation was delayed, perform it now */
  if ( ! s->arr1 )
  {
    Int32 n = 100000 * s->blockSize100k;
    bz_stream * strm = s->strm; /* BZALLOC wants this */

    s->arr1 = BZALLOC( n * sizeof(UInt32) );
    s->mtfv              = (UInt16*)s->arr1;
    s->ptr               = (UInt32*)s->arr1;
  }
}

/*---------------------------------------------------*/
static
Bool handle_compress ( bz_stream* strm, Bool * stop_before_blocksort )
{
   Bool progress_in  = False;
   Bool progress_out = False;
   EState* s = strm->state;
   
   while (True) {

      if (s->state == BZ_S_OUTPUT) {
         progress_out |= copy_output_until_stop ( s );
         if (s->state_out_pos < s->numZ) break;
         if (s->mode == BZ_M_FINISHING && 
             s->avail_in_expect == 0 &&
             isempty_RL(s)) break;
         prepare_new_block ( s );
         s->state = BZ_S_INPUT;
         if (s->mode == BZ_M_FLUSHING && 
             s->avail_in_expect == 0 &&
             isempty_RL(s)) break;
      }

      if (s->state == BZ_S_INPUT) {
         progress_in |= copy_input_until_stop ( s );
         if (s->mode != BZ_M_RUNNING && s->avail_in_expect == 0) {
            flush_RL ( s );
            if (stop_before_blocksort) {
               *stop_before_blocksort = True;
               /*fprintf( stderr, "stopping before blocksort, %d\n", s->nblock );*/
               break;
            }
            check_delayed_alloc( s );
            BZ2_compressBlock ( s, (Bool)(s->mode == BZ_M_FINISHING) );
            s->state = BZ_S_OUTPUT;
         }
         else
         if (s->nblock >= s->nblockMAX) {

            if (stop_before_blocksort) {
               *stop_before_blocksort = True;
               /*fprintf( stderr, "stopping before blocksort, %d\n", s->nblock );*/
               break;
            }

            check_delayed_alloc( s );
            BZ2_compressBlock ( s, False );
            s->state = BZ_S_OUTPUT;
         }
         else
         if (s->strm->avail_in == 0) {
            break;
         }
      }

   }

   return progress_in || progress_out;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzCompress) ( bz_stream *strm, int action )
{
   Bool progress;
   Bool stopped = False;
   Bool* stopBeforeBlocksort =
     action & BZ_STOP_BEFORE_BLOCKSORT ? &stopped : NULL;
   EState* s;

   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;

   if (s->strm != strm) return BZ_PARAM_ERROR;

   action &= ~BZ_STOP_BEFORE_BLOCKSORT;

   preswitch:
   switch (s->mode) {

      case BZ_M_IDLE:
         return BZ_SEQUENCE_ERROR;

      case BZ_M_RUNNING:
         if (action == BZ_RUN) {
            progress = handle_compress ( strm, stopBeforeBlocksort );

            if ( stopped )
              return BZ_STOPPED_BEFORE_BLOCKSORT;

            return progress ? BZ_RUN_OK : BZ_PARAM_ERROR;
         } 
         else
	 if (action == BZ_FLUSH) {
            s->avail_in_expect = strm->avail_in;
            s->mode = BZ_M_FLUSHING;
            goto preswitch;
         }
         else
         if (action == BZ_FINISH) {
            s->avail_in_expect = strm->avail_in;
            s->mode = BZ_M_FINISHING;
            goto preswitch;
         }
         else 
            return BZ_PARAM_ERROR;

      case BZ_M_FLUSHING:
         if (action != BZ_FLUSH) return BZ_SEQUENCE_ERROR;
         if (s->avail_in_expect != s->strm->avail_in) 
            return BZ_SEQUENCE_ERROR;
         progress = handle_compress ( strm, stopBeforeBlocksort );

         if ( stopped )
           return BZ_STOPPED_BEFORE_BLOCKSORT;

         if (s->avail_in_expect > 0 || !isempty_RL(s) ||
             s->state_out_pos < s->numZ) return BZ_FLUSH_OK;
         s->mode = BZ_M_RUNNING;
         return BZ_RUN_OK;

      case BZ_M_FINISHING:
         if (action != BZ_FINISH) return BZ_SEQUENCE_ERROR;
         if (s->avail_in_expect != s->strm->avail_in) 
            return BZ_SEQUENCE_ERROR;
         progress = handle_compress ( strm, stopBeforeBlocksort );

         if ( stopped )
           return BZ_STOPPED_BEFORE_BLOCKSORT;

         if (!progress) return BZ_SEQUENCE_ERROR;
         if (s->avail_in_expect > 0 || !isempty_RL(s) ||
             s->state_out_pos < s->numZ) return BZ_FINISH_OK;
         s->mode = BZ_M_IDLE;
         return BZ_STREAM_END;
   }
   return BZ_OK; /*--not reached--*/
}

void BZ_API(BZ2_bzCompressSaveStateBeforeBlocksort) ( bz_stream* strm, 
   bz_stream_state_bs * state )
{
   UInt32 blockCRC;
   EState* s = strm->state;

   state->state_in_ch = s->state_in_ch;
   state->state_in_len = s->state_in_len;

   state->blockNo = s->blockNo + 1;

   blockCRC = s->blockCRC;
   BZ_FINALISE_CRC( blockCRC );

   state->combinedCRC = (s->combinedCRC << 1) | (s->combinedCRC >> 31);
   state->combinedCRC ^= blockCRC;
}

void BZ_API(BZ2_bzCompressRestoreState) ( bz_stream* strm, 
                                         const bz_stream_state_bs * state )
{
  EState* s = strm->state;

  s->state_in_ch = state->state_in_ch;
  s->state_in_len = state->state_in_len;

  s->blockNo = state->blockNo;

  s->combinedCRC = state->combinedCRC;
}

void BZ_API(BZ2_bzCompressSaveOutputState) ( bz_stream* strm, 
      bz_stream_state_out * state )
{
  EState* s = strm->state;

  state->bsBuff = s->bsBuff;
  state->bsLive = s->bsLive;
}

void BZ_API(BZ2_bzCompressRestoreOutputState) ( bz_stream* strm, 
      const bz_stream_state_out * state )
{
  EState* s = strm->state;

  s->bsBuff = state->bsBuff;
  s->bsLive = state->bsLive;
}

void BZ_API(BZ2_bzCompressDoBlocksort) ( bz_stream* strm )
{
  EState* s = strm->state;

  check_delayed_alloc( s );
  
  BZ2_compressBlock_compute ( strm->state );

  /* When storing, a user supplied buffer will be used instead. */
  BZFREE( s->arr2 );
}

size_t BZ_API(BZ2_bzCompressStoreBlocksort) ( bz_stream* strm, void * outBuf,
                                            int lastBlock )
{
  EState* s = strm->state;

  /* From now on, the arr2 buffer is only used as a plain output buffer */
  s->arr2 = outBuf;
  s->block = (UChar*)s->arr2;
  s->dontFreeArr2 = 1;

  BZ2_compressBlock_save ( s, lastBlock );

  s->state = BZ_S_OUTPUT;

  return s->numZ;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzCompressEnd)  ( bz_stream *strm )
{
   EState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   if (s->arr1 != NULL) BZFREE(s->arr1);
   if (s->arr2 != NULL && ! s->dontFreeArr2 ) BZFREE(s->arr2);
   if (s->ftab != NULL) BZFREE(s->ftab);
   BZFREE(strm->state);

   strm->state = NULL;   

   return BZ_OK;
}


/*---------------------------------------------------*/
/*--- Decompression stuff                         ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
int BZ_API(BZ2_bzDecompressInit) 
                     ( bz_stream* strm, 
                       int        verbosity,
                       int        small )
{
   DState* s;

   if (!bz_config_ok()) return BZ_CONFIG_ERROR;

   if (strm == NULL) return BZ_PARAM_ERROR;
   if (small != 0 && small != 1) return BZ_PARAM_ERROR;
   if (verbosity < 0 || verbosity > 4) return BZ_PARAM_ERROR;

   if (strm->bzalloc == NULL) strm->bzalloc = default_bzalloc;
   if (strm->bzfree == NULL) strm->bzfree = default_bzfree;

   s = BZALLOC( sizeof(DState) );
   if (s == NULL) return BZ_MEM_ERROR;
   s->strm                  = strm;
   strm->state              = s;
   s->state                 = BZ_X_MAGIC_1;
   s->bsLive                = 0;
   s->bsBuff                = 0;
   s->calculatedCombinedCRC = 0;
   strm->total_in_lo32      = 0;
   strm->total_in_hi32      = 0;
   strm->total_out_lo32     = 0;
   strm->total_out_hi32     = 0;
   s->smallDecompress       = (Bool)small;
   s->ll4                   = NULL;
   s->ll16                  = NULL;
   s->tt                    = NULL;
   s->currBlockNo           = 0;
   s->verbosity             = verbosity;

   return BZ_OK;
}


/*---------------------------------------------------*/
/* Return  True iff data corruption is discovered.
   Returns False if there is no problem.
*/
static
Bool unRLE_obuf_to_output_FAST ( DState* s )
{
   UChar k1;

   if (s->blockRandomised) {

      while (True) {
         /* try to finish existing run */
         while (True) {
            if (s->strm->avail_out == 0) return False;
            if (s->state_out_len == 0) break;
            *( (UChar*)(s->strm->next_out) ) = s->state_out_ch;
            BZ_UPDATE_CRC ( s->calculatedBlockCRC, s->state_out_ch );
            s->state_out_len--;
            s->strm->next_out++;
            s->strm->avail_out--;
            s->strm->total_out_lo32++;
            if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
         }

         /* can a new run be started? */
         if (s->nblock_used == s->save_nblock+1) return False;
               
         /* Only caused by corrupt data stream? */
         if (s->nblock_used > s->save_nblock+1)
            return True;
   
         s->state_out_len = 1;
         s->state_out_ch = s->k0;
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         s->state_out_len = 2;
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         s->state_out_len = 3;
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         s->state_out_len = ((Int32)k1) + 4;
         BZ_GET_FAST(s->k0); BZ_RAND_UPD_MASK; 
         s->k0 ^= BZ_RAND_MASK; s->nblock_used++;
      }

   } else {

      /* restore */
      UInt32        c_calculatedBlockCRC = s->calculatedBlockCRC;
      UChar         c_state_out_ch       = s->state_out_ch;
      Int32         c_state_out_len      = s->state_out_len;
      Int32         c_nblock_used        = s->nblock_used;
      Int32         c_k0                 = s->k0;
      UInt32*       c_tt                 = s->tt;
      UInt32        c_tPos               = s->tPos;
      char*         cs_next_out          = s->strm->next_out;
      unsigned int  cs_avail_out         = s->strm->avail_out;
      /* end restore */

      UInt32       avail_out_INIT = cs_avail_out;
      Int32        s_save_nblockPP = s->save_nblock+1;
      unsigned int total_out_lo32_old;

      while (True) {

         /* try to finish existing run */
         if (c_state_out_len > 0) {
            while (True) {
               if (cs_avail_out == 0) goto return_notr;
               if (c_state_out_len == 1) break;
               *( (UChar*)(cs_next_out) ) = c_state_out_ch;
               BZ_UPDATE_CRC ( c_calculatedBlockCRC, c_state_out_ch );
               c_state_out_len--;
               cs_next_out++;
               cs_avail_out--;
            }
            s_state_out_len_eq_one:
            {
               if (cs_avail_out == 0) { 
                  c_state_out_len = 1; goto return_notr;
               };
               *( (UChar*)(cs_next_out) ) = c_state_out_ch;
               BZ_UPDATE_CRC ( c_calculatedBlockCRC, c_state_out_ch );
               cs_next_out++;
               cs_avail_out--;
            }
         }   
         /* Only caused by corrupt data stream? */
         if (c_nblock_used > s_save_nblockPP)
            return True;

         /* can a new run be started? */
         if (c_nblock_used == s_save_nblockPP) {
            c_state_out_len = 0; goto return_notr;
         };   
         c_state_out_ch = c_k0;
         BZ_GET_FAST_C(k1); c_nblock_used++;
         if (k1 != c_k0) { 
            c_k0 = k1; goto s_state_out_len_eq_one; 
         };
         if (c_nblock_used == s_save_nblockPP) 
            goto s_state_out_len_eq_one;
   
         c_state_out_len = 2;
         BZ_GET_FAST_C(k1); c_nblock_used++;
         if (c_nblock_used == s_save_nblockPP) continue;
         if (k1 != c_k0) { c_k0 = k1; continue; };
   
         c_state_out_len = 3;
         BZ_GET_FAST_C(k1); c_nblock_used++;
         if (c_nblock_used == s_save_nblockPP) continue;
         if (k1 != c_k0) { c_k0 = k1; continue; };
   
         BZ_GET_FAST_C(k1); c_nblock_used++;
         c_state_out_len = ((Int32)k1) + 4;
         BZ_GET_FAST_C(c_k0); c_nblock_used++;
      }

      return_notr:
      total_out_lo32_old = s->strm->total_out_lo32;
      s->strm->total_out_lo32 += (avail_out_INIT - cs_avail_out);
      if (s->strm->total_out_lo32 < total_out_lo32_old)
         s->strm->total_out_hi32++;

      /* save */
      s->calculatedBlockCRC = c_calculatedBlockCRC;
      s->state_out_ch       = c_state_out_ch;
      s->state_out_len      = c_state_out_len;
      s->nblock_used        = c_nblock_used;
      s->k0                 = c_k0;
      s->tt                 = c_tt;
      s->tPos               = c_tPos;
      s->strm->next_out     = cs_next_out;
      s->strm->avail_out    = cs_avail_out;
      /* end save */
   }
   return False;
}



/*---------------------------------------------------*/
/* __inline__  */
Int32 BZ2_indexIntoF ( Int32 indx, Int32 *cftab )
{
   Int32 nb, na, mid;
   nb = 0;
   na = 256;
   do {
      mid = (nb + na) >> 1;
      if (indx >= cftab[mid]) nb = mid; else na = mid;
   }
   while (na - nb != 1);
   return nb;
}


/*---------------------------------------------------*/
/* Return  True iff data corruption is discovered.
   Returns False if there is no problem.
*/
static
Bool unRLE_obuf_to_output_SMALL ( DState* s )
{
   UChar k1;

   if (s->blockRandomised) {

      while (True) {
         /* try to finish existing run */
         while (True) {
            if (s->strm->avail_out == 0) return False;
            if (s->state_out_len == 0) break;
            *( (UChar*)(s->strm->next_out) ) = s->state_out_ch;
            BZ_UPDATE_CRC ( s->calculatedBlockCRC, s->state_out_ch );
            s->state_out_len--;
            s->strm->next_out++;
            s->strm->avail_out--;
            s->strm->total_out_lo32++;
            if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
         }
   
         /* can a new run be started? */
         if (s->nblock_used == s->save_nblock+1) return False;

         /* Only caused by corrupt data stream? */
         if (s->nblock_used > s->save_nblock+1)
            return True;
   
         s->state_out_len = 1;
         s->state_out_ch = s->k0;
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         s->state_out_len = 2;
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         s->state_out_len = 3;
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK; 
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         s->state_out_len = ((Int32)k1) + 4;
         BZ_GET_SMALL(s->k0); BZ_RAND_UPD_MASK; 
         s->k0 ^= BZ_RAND_MASK; s->nblock_used++;
      }

   } else {

      while (True) {
         /* try to finish existing run */
         while (True) {
            if (s->strm->avail_out == 0) return False;
            if (s->state_out_len == 0) break;
            *( (UChar*)(s->strm->next_out) ) = s->state_out_ch;
            BZ_UPDATE_CRC ( s->calculatedBlockCRC, s->state_out_ch );
            s->state_out_len--;
            s->strm->next_out++;
            s->strm->avail_out--;
            s->strm->total_out_lo32++;
            if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
         }
   
         /* can a new run be started? */
         if (s->nblock_used == s->save_nblock+1) return False;

         /* Only caused by corrupt data stream? */
         if (s->nblock_used > s->save_nblock+1)
            return True;
   
         s->state_out_len = 1;
         s->state_out_ch = s->k0;
         BZ_GET_SMALL(k1); s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         s->state_out_len = 2;
         BZ_GET_SMALL(k1); s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         s->state_out_len = 3;
         BZ_GET_SMALL(k1); s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };
   
         BZ_GET_SMALL(k1); s->nblock_used++;
         s->state_out_len = ((Int32)k1) + 4;
         BZ_GET_SMALL(s->k0); s->nblock_used++;
      }

   }
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzDecompress) ( bz_stream *strm )
{
   Bool    corrupt;
   DState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   while (True) {
      if (s->state == BZ_X_IDLE) return BZ_SEQUENCE_ERROR;
      if (s->state == BZ_X_OUTPUT) {
         if (s->smallDecompress)
            corrupt = unRLE_obuf_to_output_SMALL ( s ); else
            corrupt = unRLE_obuf_to_output_FAST  ( s );
         if (corrupt) return BZ_DATA_ERROR;
         if (s->nblock_used == s->save_nblock+1 && s->state_out_len == 0) {
            BZ_FINALISE_CRC ( s->calculatedBlockCRC );
            if (s->verbosity >= 3) 
               VPrintf2 ( " {0x%08x, 0x%08x}", s->storedBlockCRC, 
                          s->calculatedBlockCRC );
            if (s->verbosity >= 2) VPrintf0 ( "]" );
            if (s->calculatedBlockCRC != s->storedBlockCRC)
               return BZ_DATA_ERROR;
            s->calculatedCombinedCRC 
               = (s->calculatedCombinedCRC << 1) | 
                    (s->calculatedCombinedCRC >> 31);
            s->calculatedCombinedCRC ^= s->calculatedBlockCRC;
            s->state = BZ_X_BLKHDR_1;
         } else {
            return BZ_OK;
         }
      }
      if (s->state >= BZ_X_MAGIC_1) {
         Int32 r = BZ2_decompress ( s );
         if (r == BZ_STREAM_END) {
            if (s->verbosity >= 3)
               VPrintf2 ( "\n    combined CRCs: stored = 0x%08x, computed = 0x%08x", 
                          s->storedCombinedCRC, s->calculatedCombinedCRC );
            if (s->calculatedCombinedCRC != s->storedCombinedCRC)
               return BZ_DATA_ERROR;
            return r;
         }
         if (s->state != BZ_X_OUTPUT) return r;
      }
   }

   AssertH ( 0, 6001 );

   return 0;  /*NOTREACHED*/
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzDecompressEnd)  ( bz_stream *strm )
{
   DState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   if (s->tt   != NULL) BZFREE(s->tt);
   if (s->ll16 != NULL) BZFREE(s->ll16);
   if (s->ll4  != NULL) BZFREE(s->ll4);

   BZFREE(strm->state);
   strm->state = NULL;

   return BZ_OK;
}


#ifndef BZ_NO_STDIO
/*---------------------------------------------------*/
/*--- File I/O stuff                              ---*/
/*---------------------------------------------------*/

#define BZ_SETERR(eee)                    \
{                                         \
   if (bzerror != NULL) *bzerror = eee;   \
   if (bzf != NULL) bzf->lastErr = eee;   \
}

typedef 
   struct {
      FILE*     handle;
      Char      buf[BZ_MAX_UNUSED];
      Int32     bufN;
      Bool      writing;
      bz_stream strm;
      Int32     lastErr;
      Bool      initialisedOk;
   }
   bzFile;


/*---------------------------------------------*/
static Bool myfeof ( FILE* f )
{
   Int32 c = fgetc ( f );
   if (c == EOF) return True;
   ungetc ( c, f );
   return False;
}


/*---------------------------------------------------*/
BZFILE* BZ_API(BZ2_bzWriteOpen) 
                    ( int*  bzerror,      
                      FILE* f, 
                      int   blockSize100k, 
                      int   verbosity,
                      int   workFactor )
{
   Int32   ret;
   bzFile* bzf = NULL;

   BZ_SETERR(BZ_OK);

   if (f == NULL ||
       (blockSize100k < 1 || blockSize100k > 9) ||
       (workFactor < 0 || workFactor > 250) ||
       (verbosity < 0 || verbosity > 4))
      { BZ_SETERR(BZ_PARAM_ERROR); return NULL; };

   if (ferror(f))
      { BZ_SETERR(BZ_IO_ERROR); return NULL; };

   bzf = malloc ( sizeof(bzFile) );
   if (bzf == NULL)
      { BZ_SETERR(BZ_MEM_ERROR); return NULL; };

   BZ_SETERR(BZ_OK);
   bzf->initialisedOk = False;
   bzf->bufN          = 0;
   bzf->handle        = f;
   bzf->writing       = True;
   bzf->strm.bzalloc  = NULL;
   bzf->strm.bzfree   = NULL;
   bzf->strm.opaque   = NULL;

   if (workFactor == 0) workFactor = 30;
   ret = BZ2_bzCompressInit ( &(bzf->strm), blockSize100k, 
                              verbosity, workFactor, 0 );
   if (ret != BZ_OK)
      { BZ_SETERR(ret); free(bzf); return NULL; };

   bzf->strm.avail_in = 0;
   bzf->initialisedOk = True;
   return bzf;   
}



/*---------------------------------------------------*/
void BZ_API(BZ2_bzWrite)
             ( int*    bzerror, 
               BZFILE* b, 
               void*   buf, 
               int     len )
{
   Int32 n, n2, ret;
   bzFile* bzf = (bzFile*)b;

   BZ_SETERR(BZ_OK);
   if (bzf == NULL || buf == NULL || len < 0)
      { BZ_SETERR(BZ_PARAM_ERROR); return; };
   if (!(bzf->writing))
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };
   if (ferror(bzf->handle))
      { BZ_SETERR(BZ_IO_ERROR); return; };

   if (len == 0)
      { BZ_SETERR(BZ_OK); return; };

   bzf->strm.avail_in = len;
   bzf->strm.next_in  = buf;

   while (True) {
      bzf->strm.avail_out = BZ_MAX_UNUSED;
      bzf->strm.next_out = bzf->buf;
      ret = BZ2_bzCompress ( &(bzf->strm), BZ_RUN );
      if (ret != BZ_RUN_OK)
         { BZ_SETERR(ret); return; };

      if (bzf->strm.avail_out < BZ_MAX_UNUSED) {
         n = BZ_MAX_UNUSED - bzf->strm.avail_out;
         n2 = fwrite ( (void*)(bzf->buf), sizeof(UChar), 
                       n, bzf->handle );
         if (n != n2 || ferror(bzf->handle))
            { BZ_SETERR(BZ_IO_ERROR); return; };
      }

      if (bzf->strm.avail_in == 0)
         { BZ_SETERR(BZ_OK); return; };
   }
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzWriteClose)
                  ( int*          bzerror, 
                    BZFILE*       b, 
                    int           abandon,
                    unsigned int* nbytes_in,
                    unsigned int* nbytes_out )
{
   BZ2_bzWriteClose64 ( bzerror, b, abandon, 
                        nbytes_in, NULL, nbytes_out, NULL );
}


void BZ_API(BZ2_bzWriteClose64)
                  ( int*          bzerror, 
                    BZFILE*       b, 
                    int           abandon,
                    unsigned int* nbytes_in_lo32,
                    unsigned int* nbytes_in_hi32,
                    unsigned int* nbytes_out_lo32,
                    unsigned int* nbytes_out_hi32 )
{
   Int32   n, n2, ret;
   bzFile* bzf = (bzFile*)b;

   if (bzf == NULL)
      { BZ_SETERR(BZ_OK); return; };
   if (!(bzf->writing))
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };
   if (ferror(bzf->handle))
      { BZ_SETERR(BZ_IO_ERROR); return; };

   if (nbytes_in_lo32 != NULL) *nbytes_in_lo32 = 0;
   if (nbytes_in_hi32 != NULL) *nbytes_in_hi32 = 0;
   if (nbytes_out_lo32 != NULL) *nbytes_out_lo32 = 0;
   if (nbytes_out_hi32 != NULL) *nbytes_out_hi32 = 0;

   if ((!abandon) && bzf->lastErr == BZ_OK) {
      while (True) {
         bzf->strm.avail_out = BZ_MAX_UNUSED;
         bzf->strm.next_out = bzf->buf;
         ret = BZ2_bzCompress ( &(bzf->strm), BZ_FINISH );
         if (ret != BZ_FINISH_OK && ret != BZ_STREAM_END)
            { BZ_SETERR(ret); return; };

         if (bzf->strm.avail_out < BZ_MAX_UNUSED) {
            n = BZ_MAX_UNUSED - bzf->strm.avail_out;
            n2 = fwrite ( (void*)(bzf->buf), sizeof(UChar), 
                          n, bzf->handle );
            if (n != n2 || ferror(bzf->handle))
               { BZ_SETERR(BZ_IO_ERROR); return; };
         }

         if (ret == BZ_STREAM_END) break;
      }
   }

   if ( !abandon && !ferror ( bzf->handle ) ) {
      fflush ( bzf->handle );
      if (ferror(bzf->handle))
         { BZ_SETERR(BZ_IO_ERROR); return; };
   }

   if (nbytes_in_lo32 != NULL)
      *nbytes_in_lo32 = bzf->strm.total_in_lo32;
   if (nbytes_in_hi32 != NULL)
      *nbytes_in_hi32 = bzf->strm.total_in_hi32;
   if (nbytes_out_lo32 != NULL)
      *nbytes_out_lo32 = bzf->strm.total_out_lo32;
   if (nbytes_out_hi32 != NULL)
      *nbytes_out_hi32 = bzf->strm.total_out_hi32;

   BZ_SETERR(BZ_OK);
   BZ2_bzCompressEnd ( &(bzf->strm) );
   free ( bzf );
}


/*---------------------------------------------------*/
BZFILE* BZ_API(BZ2_bzReadOpen) 
                   ( int*  bzerror, 
                     FILE* f, 
                     int   verbosity,
                     int   small,
                     void* unused,
                     int   nUnused )
{
   bzFile* bzf = NULL;
   int     ret;

   BZ_SETERR(BZ_OK);

   if (f == NULL || 
       (small != 0 && small != 1) ||
       (verbosity < 0 || verbosity > 4) ||
       (unused == NULL && nUnused != 0) ||
       (unused != NULL && (nUnused < 0 || nUnused > BZ_MAX_UNUSED)))
      { BZ_SETERR(BZ_PARAM_ERROR); return NULL; };

   if (ferror(f))
      { BZ_SETERR(BZ_IO_ERROR); return NULL; };

   bzf = malloc ( sizeof(bzFile) );
   if (bzf == NULL) 
      { BZ_SETERR(BZ_MEM_ERROR); return NULL; };

   BZ_SETERR(BZ_OK);

   bzf->initialisedOk = False;
   bzf->handle        = f;
   bzf->bufN          = 0;
   bzf->writing       = False;
   bzf->strm.bzalloc  = NULL;
   bzf->strm.bzfree   = NULL;
   bzf->strm.opaque   = NULL;
   
   while (nUnused > 0) {
      bzf->buf[bzf->bufN] = *((UChar*)(unused)); bzf->bufN++;
      unused = ((void*)( 1 + ((UChar*)(unused))  ));
      nUnused--;
   }

   ret = BZ2_bzDecompressInit ( &(bzf->strm), verbosity, small );
   if (ret != BZ_OK)
      { BZ_SETERR(ret); free(bzf); return NULL; };

   bzf->strm.avail_in = bzf->bufN;
   bzf->strm.next_in  = bzf->buf;

   bzf->initialisedOk = True;
   return bzf;   
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzReadClose) ( int *bzerror, BZFILE *b )
{
   bzFile* bzf = (bzFile*)b;

   BZ_SETERR(BZ_OK);
   if (bzf == NULL)
      { BZ_SETERR(BZ_OK); return; };

   if (bzf->writing)
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };

   if (bzf->initialisedOk)
      (void)BZ2_bzDecompressEnd ( &(bzf->strm) );
   free ( bzf );
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzRead) 
           ( int*    bzerror, 
             BZFILE* b, 
             void*   buf, 
             int     len )
{
   Int32   n, ret;
   bzFile* bzf = (bzFile*)b;

   BZ_SETERR(BZ_OK);

   if (bzf == NULL || buf == NULL || len < 0)
      { BZ_SETERR(BZ_PARAM_ERROR); return 0; };

   if (bzf->writing)
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return 0; };

   if (len == 0)
      { BZ_SETERR(BZ_OK); return 0; };

   bzf->strm.avail_out = len;
   bzf->strm.next_out = buf;

   while (True) {

      if (ferror(bzf->handle)) 
         { BZ_SETERR(BZ_IO_ERROR); return 0; };

      if (bzf->strm.avail_in == 0 && !myfeof(bzf->handle)) {
         n = fread ( bzf->buf, sizeof(UChar), 
                     BZ_MAX_UNUSED, bzf->handle );
         if (ferror(bzf->handle))
            { BZ_SETERR(BZ_IO_ERROR); return 0; };
         bzf->bufN = n;
         bzf->strm.avail_in = bzf->bufN;
         bzf->strm.next_in = bzf->buf;
      }

      ret = BZ2_bzDecompress ( &(bzf->strm) );

      if (ret != BZ_OK && ret != BZ_STREAM_END)
         { BZ_SETERR(ret); return 0; };

      if (ret == BZ_OK && myfeof(bzf->handle) && 
          bzf->strm.avail_in == 0 && bzf->strm.avail_out > 0)
         { BZ_SETERR(BZ_UNEXPECTED_EOF); return 0; };

      if (ret == BZ_STREAM_END)
         { BZ_SETERR(BZ_STREAM_END);
           return len - bzf->strm.avail_out; };
      if (bzf->strm.avail_out == 0)
         { BZ_SETERR(BZ_OK); return len; };
      
   }

   return 0; /*not reached*/
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzReadGetUnused) 
                     ( int*    bzerror, 
                       BZFILE* b, 
                       void**  unused, 
                       int*    nUnused )
{
   bzFile* bzf = (bzFile*)b;
   if (bzf == NULL)
      { BZ_SETERR(BZ_PARAM_ERROR); return; };
   if (bzf->lastErr != BZ_STREAM_END)
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };
   if (unused == NULL || nUnused == NULL)
      { BZ_SETERR(BZ_PARAM_ERROR); return; };

   BZ_SETERR(BZ_OK);
   *nUnused = bzf->strm.avail_in;
   *unused = bzf->strm.next_in;
}
#endif


/*---------------------------------------------------*/
/*--- Misc convenience stuff                      ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
int BZ_API(BZ2_bzBuffToBuffCompress) 
                         ( char*         dest, 
                           unsigned int* destLen,
                           char*         source, 
                           unsigned int  sourceLen,
                           int           blockSize100k, 
                           int           verbosity, 
                           int           workFactor )
{
   bz_stream strm;
   int ret;

   if (dest == NULL || destLen == NULL || 
       source == NULL ||
       blockSize100k < 1 || blockSize100k > 9 ||
       verbosity < 0 || verbosity > 4 ||
       workFactor < 0 || workFactor > 250) 
      return BZ_PARAM_ERROR;

   if (workFactor == 0) workFactor = 30;
   strm.bzalloc = NULL;
   strm.bzfree = NULL;
   strm.opaque = NULL;
   ret = BZ2_bzCompressInit ( &strm, blockSize100k, 
                              verbosity, workFactor, 0 );
   if (ret != BZ_OK) return ret;

   strm.next_in = source;
   strm.next_out = dest;
   strm.avail_in = sourceLen;
   strm.avail_out = *destLen;

   ret = BZ2_bzCompress ( &strm, BZ_FINISH );
   if (ret == BZ_FINISH_OK) goto output_overflow;
   if (ret != BZ_STREAM_END) goto errhandler;

   /* normal termination */
   *destLen -= strm.avail_out;   
   BZ2_bzCompressEnd ( &strm );
   return BZ_OK;

   output_overflow:
   BZ2_bzCompressEnd ( &strm );
   return BZ_OUTBUFF_FULL;

   errhandler:
   BZ2_bzCompressEnd ( &strm );
   return ret;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzBuffToBuffDecompress) 
                           ( char*         dest, 
                             unsigned int* destLen,
                             char*         source, 
                             unsigned int  sourceLen,
                             int           small,
                             int           verbosity )
{
   bz_stream strm;
   int ret;

   if (dest == NULL || destLen == NULL || 
       source == NULL ||
       (small != 0 && small != 1) ||
       verbosity < 0 || verbosity > 4) 
          return BZ_PARAM_ERROR;

   strm.bzalloc = NULL;
   strm.bzfree = NULL;
   strm.opaque = NULL;
   ret = BZ2_bzDecompressInit ( &strm, verbosity, small );
   if (ret != BZ_OK) return ret;

   strm.next_in = source;
   strm.next_out = dest;
   strm.avail_in = sourceLen;
   strm.avail_out = *destLen;

   ret = BZ2_bzDecompress ( &strm );
   if (ret == BZ_OK) goto output_overflow_or_eof;
   if (ret != BZ_STREAM_END) goto errhandler;

   /* normal termination */
   *destLen -= strm.avail_out;
   BZ2_bzDecompressEnd ( &strm );
   return BZ_OK;

   output_overflow_or_eof:
   if (strm.avail_out > 0) {
      BZ2_bzDecompressEnd ( &strm );
      return BZ_UNEXPECTED_EOF;
   } else {
      BZ2_bzDecompressEnd ( &strm );
      return BZ_OUTBUFF_FULL;
   };      

   errhandler:
   BZ2_bzDecompressEnd ( &strm );
   return ret; 
}


/*---------------------------------------------------*/
/*--
   Code contributed by Yoshioka Tsuneo
   (QWF00133@niftyserve.or.jp/tsuneo-y@is.aist-nara.ac.jp),
   to support better zlib compatibility.
   This code is not _officially_ part of libbzip2 (yet);
   I haven't tested it, documented it, or considered the
   threading-safeness of it.
   If this code breaks, please contact both Yoshioka and me.
--*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/*--
   return version like "0.9.0c".
--*/
const char * BZ_API(BZ2_bzlibVersion)(void)
{
   return BZ_VERSION;
}


#ifndef BZ_NO_STDIO
/*---------------------------------------------------*/

#if defined(_WIN32) || defined(OS2) || defined(MSDOS)
#   include <fcntl.h>
#   include <io.h>
#   define SET_BINARY_MODE(file) setmode(fileno(file),O_BINARY)
#else
#   define SET_BINARY_MODE(file)
#endif
static
BZFILE * bzopen_or_bzdopen
               ( const char *path,   /* no use when bzdopen */
                 int fd,             /* no use when bzdopen */
                 const char *mode,
                 int open_mode)      /* bzopen: 0, bzdopen:1 */
{
   int    bzerr;
   char   unused[BZ_MAX_UNUSED];
   int    blockSize100k = 9;
   int    writing       = 0;
   char   mode2[10]     = "";
   FILE   *fp           = NULL;
   BZFILE *bzfp         = NULL;
   int    verbosity     = 0;
   int    workFactor    = 30;
   int    smallMode     = 0;
   int    nUnused       = 0; 

   if (mode == NULL) return NULL;
   while (*mode) {
      switch (*mode) {
      case 'r':
         writing = 0; break;
      case 'w':
         writing = 1; break;
      case 's':
         smallMode = 1; break;
      default:
         if (isdigit((int)(*mode))) {
            blockSize100k = *mode-BZ_HDR_0;
         }
      }
      mode++;
   }
   strcat(mode2, writing ? "w" : "r" );
   strcat(mode2,"b");   /* binary mode */

   if (open_mode==0) {
      if (path==NULL || strcmp(path,"")==0) {
        fp = (writing ? stdout : stdin);
        SET_BINARY_MODE(fp);
      } else {
        fp = fopen(path,mode2);
      }
   } else {
#ifdef BZ_STRICT_ANSI
      fp = NULL;
#else
      fp = fdopen(fd,mode2);
#endif
   }
   if (fp == NULL) return NULL;

   if (writing) {
      /* Guard against total chaos and anarchy -- JRS */
      if (blockSize100k < 1) blockSize100k = 1;
      if (blockSize100k > 9) blockSize100k = 9; 
      bzfp = BZ2_bzWriteOpen(&bzerr,fp,blockSize100k,
                             verbosity,workFactor);
   } else {
      bzfp = BZ2_bzReadOpen(&bzerr,fp,verbosity,smallMode,
                            unused,nUnused);
   }
   if (bzfp == NULL) {
      if (fp != stdin && fp != stdout) fclose(fp);
      return NULL;
   }
   return bzfp;
}


/*---------------------------------------------------*/
/*--
   open file for read or write.
      ex) bzopen("file","w9")
      case path="" or NULL => use stdin or stdout.
--*/
BZFILE * BZ_API(BZ2_bzopen)
               ( const char *path,
                 const char *mode )
{
   return bzopen_or_bzdopen(path,-1,mode,/*bzopen*/0);
}


/*---------------------------------------------------*/
BZFILE * BZ_API(BZ2_bzdopen)
               ( int fd,
                 const char *mode )
{
   return bzopen_or_bzdopen(NULL,fd,mode,/*bzdopen*/1);
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzread) (BZFILE* b, void* buf, int len )
{
   int bzerr, nread;
   if (((bzFile*)b)->lastErr == BZ_STREAM_END) return 0;
   nread = BZ2_bzRead(&bzerr,b,buf,len);
   if (bzerr == BZ_OK || bzerr == BZ_STREAM_END) {
      return nread;
   } else {
      return -1;
   }
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzwrite) (BZFILE* b, void* buf, int len )
{
   int bzerr;

   BZ2_bzWrite(&bzerr,b,buf,len);
   if(bzerr == BZ_OK){
      return len;
   }else{
      return -1;
   }
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzflush) (BZFILE *b)
{
   /* do nothing now... */
   return 0;
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzclose) (BZFILE* b)
{
   int bzerr;
   FILE *fp = ((bzFile *)b)->handle;
   
   if (b==NULL) {return;}
   if(((bzFile*)b)->writing){
      BZ2_bzWriteClose(&bzerr,b,0,NULL,NULL);
      if(bzerr != BZ_OK){
         BZ2_bzWriteClose(NULL,b,1,NULL,NULL);
      }
   }else{
      BZ2_bzReadClose(&bzerr,b);
   }
   if(fp!=stdin && fp!=stdout){
      fclose(fp);
   }
}


/*---------------------------------------------------*/
/*--
   return last error code 
--*/
static char *bzerrorstrings[] = {
       "OK"
      ,"SEQUENCE_ERROR"
      ,"PARAM_ERROR"
      ,"MEM_ERROR"
      ,"DATA_ERROR"
      ,"DATA_ERROR_MAGIC"
      ,"IO_ERROR"
      ,"UNEXPECTED_EOF"
      ,"OUTBUFF_FULL"
      ,"CONFIG_ERROR"
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
};


const char * BZ_API(BZ2_bzerror) (BZFILE *b, int *errnum)
{
   int err = ((bzFile *)b)->lastErr;

   if(err>0) err = 0;
   *errnum = err;
   return bzerrorstrings[err*-1];
}
#endif


/*-------------------------------------------------------------*/
/*--- end                                           bzlib.c ---*/
/*-------------------------------------------------------------*/




////////////////////  end of bzlib.c /////////////////////////////






/*-------------------------------------------------------------*/
/*--- Huffman coding low-level stuff                        ---*/
/*---                                             huffman.c ---*/
/*-------------------------------------------------------------*/

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2002 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must 
     not claim that you wrote the original software.  If you use this 
     software in a product, an acknowledgment in the product 
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@acm.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/


//#include "bzlib_private.h"

/*---------------------------------------------------*/
#define WEIGHTOF(zz0)  ((zz0) & 0xffffff00)
#define DEPTHOF(zz1)   ((zz1) & 0x000000ff)
#define MYMAX(zz2,zz3) ((zz2) > (zz3) ? (zz2) : (zz3))

#define ADDWEIGHTS(zw1,zw2)                           \
   (WEIGHTOF(zw1)+WEIGHTOF(zw2)) |                    \
   (1 + MYMAX(DEPTHOF(zw1),DEPTHOF(zw2)))

#define UPHEAP(z)                                     \
{                                                     \
   Int32 zz, tmp;                                     \
   zz = z; tmp = heap[zz];                            \
   while (weight[tmp] < weight[heap[zz >> 1]]) {      \
      heap[zz] = heap[zz >> 1];                       \
      zz >>= 1;                                       \
   }                                                  \
   heap[zz] = tmp;                                    \
}

#define DOWNHEAP(z)                                   \
{                                                     \
   Int32 zz, yy, tmp;                                 \
   zz = z; tmp = heap[zz];                            \
   while (True) {                                     \
      yy = zz << 1;                                   \
      if (yy > nHeap) break;                          \
      if (yy < nHeap &&                               \
          weight[heap[yy+1]] < weight[heap[yy]])      \
         yy++;                                        \
      if (weight[tmp] < weight[heap[yy]]) break;      \
      heap[zz] = heap[yy];                            \
      zz = yy;                                        \
   }                                                  \
   heap[zz] = tmp;                                    \
}


/*---------------------------------------------------*/
void BZ2_hbMakeCodeLengths ( UChar *len, 
                             Int32 *freq,
                             Int32 alphaSize,
                             Int32 maxLen )
{
   /*--
      Nodes and heap entries run from 1.  Entry 0
      for both the heap and nodes is a sentinel.
   --*/
   Int32 nNodes, nHeap, n1, n2, i, j, k;
   Bool  tooLong;

   Int32 heap   [ BZ_MAX_ALPHA_SIZE + 2 ];
   Int32 weight [ BZ_MAX_ALPHA_SIZE * 2 ];
   Int32 parent [ BZ_MAX_ALPHA_SIZE * 2 ]; 

   for (i = 0; i < alphaSize; i++)
      weight[i+1] = (freq[i] == 0 ? 1 : freq[i]) << 8;

   while (True) {

      nNodes = alphaSize;
      nHeap = 0;

      heap[0] = 0;
      weight[0] = 0;
      parent[0] = -2;

      for (i = 1; i <= alphaSize; i++) {
         parent[i] = -1;
         nHeap++;
         heap[nHeap] = i;
         UPHEAP(nHeap);
      }

      AssertH( nHeap < (BZ_MAX_ALPHA_SIZE+2), 2001 );
   
      while (nHeap > 1) {
         n1 = heap[1]; heap[1] = heap[nHeap]; nHeap--; DOWNHEAP(1);
         n2 = heap[1]; heap[1] = heap[nHeap]; nHeap--; DOWNHEAP(1);
         nNodes++;
         parent[n1] = parent[n2] = nNodes;
         weight[nNodes] = ADDWEIGHTS(weight[n1], weight[n2]);
         parent[nNodes] = -1;
         nHeap++;
         heap[nHeap] = nNodes;
         UPHEAP(nHeap);
      }

      AssertH( nNodes < (BZ_MAX_ALPHA_SIZE * 2), 2002 );

      tooLong = False;
      for (i = 1; i <= alphaSize; i++) {
         j = 0;
         k = i;
         while (parent[k] >= 0) { k = parent[k]; j++; }
         len[i-1] = j;
         if (j > maxLen) tooLong = True;
      }
      
      if (! tooLong) break;

      /* 17 Oct 04: keep-going condition for the following loop used
         to be 'i < alphaSize', which missed the last element,
         theoretically leading to the possibility of the compressor
         looping.  However, this count-scaling step is only needed if
         one of the generated Huffman code words is longer than
         maxLen, which up to and including version 1.0.2 was 20 bits,
         which is extremely unlikely.  In version 1.0.3 maxLen was
         changed to 17 bits, which has minimal effect on compression
         ratio, but does mean this scaling step is used from time to
         time, enough to verify that it works.

         This means that bzip2-1.0.3 and later will only produce
         Huffman codes with a maximum length of 17 bits.  However, in
         order to preserve backwards compatibility with bitstreams
         produced by versions pre-1.0.3, the decompressor must still
         handle lengths of up to 20. */

      for (i = 1; i <= alphaSize; i++) {
         j = weight[i] >> 8;
         j = 1 + (j / 2);
         weight[i] = j << 8;
      }
   }
}


/*---------------------------------------------------*/
void BZ2_hbAssignCodes ( Int32 *code,
                         UChar *length,
                         Int32 minLen,
                         Int32 maxLen,
                         Int32 alphaSize )
{
   Int32 n, vec, i;

   vec = 0;
   for (n = minLen; n <= maxLen; n++) {
      for (i = 0; i < alphaSize; i++)
         if (length[i] == n) { code[i] = vec; vec++; };
      vec <<= 1;
   }
}


/*---------------------------------------------------*/
void BZ2_hbCreateDecodeTables ( Int32 *limit,
                                Int32 *base,
                                Int32 *perm,
                                UChar *length,
                                Int32 minLen,
                                Int32 maxLen,
                                Int32 alphaSize )
{
   Int32 pp, i, j, vec;

   pp = 0;
   for (i = minLen; i <= maxLen; i++)
      for (j = 0; j < alphaSize; j++)
         if (length[j] == i) { perm[pp] = j; pp++; };

   for (i = 0; i < BZ_MAX_CODE_LEN; i++) base[i] = 0;
   for (i = 0; i < alphaSize; i++) base[length[i]+1]++;

   for (i = 1; i < BZ_MAX_CODE_LEN; i++) base[i] += base[i-1];

   for (i = 0; i < BZ_MAX_CODE_LEN; i++) limit[i] = 0;
   vec = 0;

   for (i = minLen; i <= maxLen; i++) {
      vec += (base[i+1] - base[i]);
      limit[i] = vec-1;
      vec <<= 1;
   }
   for (i = minLen + 1; i <= maxLen; i++)
      base[i] = ((limit[i-1] + 1) << 1) - base[i];
}


/*-------------------------------------------------------------*/
/*--- end                                         huffman.c ---*/
/*-------------------------------------------------------------*/


/*////////////////////// end of huffman.c //////////////////////////////////*/



#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>



int verbosityLevel = 3;

static char programName[] = "bzip2smp: ";

void note( int level, const char * format, ... )
{
  fprintf(stderr, "%p: ", (void*)pthread_self());

  if ( verbosityLevel >= level )
  {
    char * str;
    va_list args;

    str = malloc( strlen( format ) + strlen( programName ) + 1 );

    if ( str == NULL )
    {
      fprintf( stderr, "%sfailed to allocate memory\n", programName );
      exit( 1 );
    }

    strcpy( str, programName );
    strcat( str, format );

    va_start( args, format );

    vfprintf( stderr, str, args );

    va_end( args );

    free( str );
  }
}

/******************** end of note.c ****************************/


#ifndef HT_DETECTION
#warning Dont know how to detect hyperthreading on this system, please implement it to counter the need to specify it explicitly
#endif

#define BUFSZ 4096

static pthread_mutex_t inChunksMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t inChunksAvailableCondition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t inChunksFreeCondition = PTHREAD_COND_INITIALIZER;

/* Number of chunks currently available to be allocated and put to the inChunks ring buffer */
static int inChunksFree;
static bz_stream ** inChunks;
static int inChunksCount; /* Total number of elements in the inChunks buffer */
static volatile int inChunksHead = 0; 
static volatile int inChunksTail = -1; /* -1 means that the buffer is empty */ 

/* The output chunks, organized in a ring buffer fasion. The tail chunk is the
one to be written to stdout next, when it is ready. The number of chunks is set
to number of threads * 2 + 1. When a thread finishes compression, it checks if
it finished the compression of a chunk which is to be written next. If so,
it triggers a flush condition.
Note that the chunk is present when its data pointer is non-NULL.
*/

static pthread_mutex_t outChunksMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t outChunksAvailableCondition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t outChunksFlushCondition = PTHREAD_COND_INITIALIZER;
static bz_stream ** outChunks;
static int outChunksCount;
static volatile int outChunksHead = 0; 
static volatile int outChunksTail = -1; /* -1 means that the buffer is empty */ 

static pthread_mutex_t inOutChunksAllocationMutex = PTHREAD_MUTEX_INITIALIZER;


/* Wraps around a negative index within the ring buffer */
int wrapChunkIndex( int idx )
{
  if ( idx < 0 )
    return outChunksCount + idx;
  else
    return idx;
}

static size_t blockSize100k = 9;

void * allocMem( size_t size )
{
  void * result = malloc( size );

  if ( result == NULL )
  {
    note( 0, "Failed to allocate %u bytes of memory\n", size );
    exit( 1 );
  }

  return result;
}

/* The compressing threads' function */
void * threadFunction()
{
  int outChunk;
  bz_stream * strm;

  for( ; ; )
  {
    /* Take the mutex to make the in-out chunk acquisition operation atomic */

    pthread_mutex_lock( &inOutChunksAllocationMutex );

    /* Get the next input chunk from the list of available */
    pthread_mutex_lock( &inChunksMutex );

    if ( inChunksTail == -1 ){
      note( 2, "THREAD INPUT STARVATION_____________\n" );
      pthread_cond_wait( &inChunksAvailableCondition, &inChunksMutex );
    }

    strm = inChunks[ inChunksTail ];
    ++inChunksTail;

    if ( inChunksTail == inChunksCount)  inChunksTail = 0;
    if ( inChunksTail == inChunksHead )  inChunksTail = -1;

    pthread_mutex_unlock( &inChunksMutex );

    /* Allocate the out chunk */

    pthread_mutex_lock( &outChunksMutex );

    if ( outChunksHead == outChunksTail ){
      note( 2, "THREAD OUTPUT STARVATION.............\n" );
      /* No free chunk available, wait for one */
      pthread_cond_wait( &outChunksAvailableCondition, &outChunksMutex );
    }

    if ( outChunksTail == -1 )   outChunksTail = outChunksHead;
    note( 2, "allocating outchunk %d\n", outChunksHead );\
    outChunk = outChunksHead++;

    if ( outChunksHead == outChunksCount )  outChunksHead = 0;
    pthread_mutex_unlock( &outChunksMutex );
    pthread_mutex_unlock( &inOutChunksAllocationMutex );

    /* Now perform the blocksort */

    BZ2_bzCompressDoBlocksort( strm );

    /* Great, now indicate that the in chunk is free and make the
    out chunk available */

    pthread_mutex_lock( &inChunksMutex );
    ++inChunksFree;
    pthread_cond_signal( &inChunksFreeCondition );
    pthread_mutex_unlock( &inChunksMutex );
    pthread_mutex_lock( &outChunksMutex );

    outChunks[ outChunk ] = strm;
    note( 2, "finished outchunk %d, tail is %d\n", outChunk, outChunksTail );

    if ( outChunk == outChunksTail )
      pthread_cond_signal( &outChunksFlushCondition );

    pthread_mutex_unlock( &outChunksMutex );
  }
}

/* This thread is only writing the results to stdout */
void * writerThread()
{
  void * buf = allocMem( blockSize100k * 101000 + 600 );
  size_t blockSize;
  bz_stream * strm;
  bz_stream_state_out savedState;
  int wasStateSaved = 0;

  pthread_mutex_lock( &outChunksMutex );

  for( ; ; )
  {
    if ( outChunksTail == -1 || ! outChunks[ outChunksTail ] )
      pthread_cond_wait( &outChunksFlushCondition, &outChunksMutex );

    pthread_mutex_unlock( &outChunksMutex );

    note( 2, "flushing outchunk %d\n", outChunksTail );

    strm = outChunks[ outChunksTail ];

    if ( wasStateSaved )
      BZ2_bzCompressRestoreOutputState( strm, &savedState );

    blockSize = BZ2_bzCompressStoreBlocksort( strm, buf, strm->avail_in );

    BZ2_bzCompressSaveOutputState( strm, &savedState );
    wasStateSaved = 1;

    BZ2_bzCompressEnd( strm );

    if ( blockSize && fwrite( buf, blockSize, 1, stdout ) != 1 )
    {
      note( 0, "Error writing to stdout\n" );
      exit( 1 );
    }

    free( strm );

    outChunks[ outChunksTail ] = NULL;

    /* Since we have output the tail chunk, increase the tail, notifying
    a thread in case it is waiting for a free chunk.*/
    pthread_mutex_lock( &outChunksMutex );

    ++outChunksTail;

    if ( outChunksTail == outChunksCount )
      outChunksTail = 0;

    pthread_cond_signal( &outChunksAvailableCondition );

    /* If we empty the ring buffer out, mark it as empty */
    if ( outChunksTail == outChunksHead )
      outChunksTail = -1;
  }
}

static char helpText[] =
"\nbzip2smp, a parallelizing bzip2 implementation, version 1.0, 2-Dec-2005\n\n"
"Usage: bzip2smp [-123456789v] [-p#] [--ht|--no-ht] [--help]\n\n"
"The data is read from standard input and the result is output to standard ouput.\n"
"No other modes are supported.\n\n"
"Use -123456789 to specify the bzip2 block size (900k by default).\n\n"
"Use -v to increase verbosity (may be specified more than once).\n\n"
"Use -p# to specify the number of threads to use (default is the number of CPUs\n"
"present in system).\n\n"
"Please note that the use of hyperthreading generally degrades performance due\n"
"to the increased cache miss. Use --ht to make the program halve the number of\n"
"CPUs returned by the system to account to the presence of hyperthreading. "

#ifdef HT_DETECTION
                                                                           "By\n"
"default, this is attempted to be autodetected. Use -v to get a clue on how\n"
"the detection worked for you. In case the misdetection occured, use --no-ht.\n"
"There is no need to bother with the --ht flags if you specify the number of\n"
"threads (-p#) explicitly.\n"

#else

                                                                           "To\n"
"check out the number of CPUs detected, use -v key. There is no need to bother\n"
"with the --ht flag if you specify the number of threads (-p#) explicitly.\n"

#endif
;


/*   ./bzip2smp  -p4 
 */
int main(int argc, char *argv[])
{
  int hyperthreading = -1;
  unsigned int threadsCount = 0;
  pthread_t dummy;
  char inputBuf[ BUFSZ ];
  char * inputBufPtr = inputBuf;
  size_t inputBufLeft = 0;
  bz_stream_state_bs savedState;
  int wasStateSaved = 0;

  char *default_argv[] = {argv[0], "--no-ht", "-1", "-p2", "bzip_input", "output.txt", NULL};
  if (argc == 1)
  {
    argv = default_argv;
    argc = 6;
  }

  int x;

  /* Parse arguments */
  for( x = 1; x < argc; ++x )
  {
    if (argv[x][0] != '-') break;

    if ( !strcmp( argv[ x ], "--help" ) ){
      fprintf( stderr, "%s", helpText );
      return 1;
    }

    if ( !strcmp( argv[ x ], "--ht" ) ){
      hyperthreading = 1;
      continue;
    }

    if ( !strcmp( argv[ x ], "--no-ht" ) ){
      hyperthreading = 0;
      continue;
    }

    if ( argv[ x ][ 0 ] == '-' ){
      char * p = argv[ x ] + 1;
      char c;

      do {
        c = *p++;

        if (  c >= '1' && c <= '9' ){
          blockSize100k = c - '1' + 1;
        }
        else if ( c == 'p' ){
          if ( sscanf( p, "%u", &threadsCount ) != 1 )
          {
            note( 0, "Error parsing the number of threads passed: %s.\n",
                  argv[ x ] + 2 );

            return 1;
          }
          c = 0; /* We don't support parsing past -p# */
        }
        else
        if ( c == 'v' )
        {
          ++verbosityLevel;
        }
        else
          break;
      } while( c );

      if ( !c )
        continue;
    }

    note( 0, "Unrecognized option %s passed.\n", argv[ x ] );

    return 1;
  }

  FILE * input_file, * output_file;
  input_file = freopen(argv[x], "r+", stdin);
  output_file = freopen(argv[x+1], "w+", stdout);

  if ( isatty ( fileno ( stdout ) ) )
  {
    note( 0, "Won't write compressed data to a terminal. Use --help to get help.\n" );
    return 1;
  }

  if ( !threadsCount )
  {
    x = sysconf( _SC_NPROCESSORS_ONLN );

    if ( x == -1 )
    {
      note( 0, "Failed to get the number of processors in the system: %s",
            strerror( errno ) );
  
      return 1;
    }

    threadsCount = x;

    if ( hyperthreading == -1 )
      hyperthreading = isHtPresent();

    if ( hyperthreading )
      threadsCount /= 2;
  
    note( 1, "CPUs detected: %d\n", threadsCount );
  }

  note( 1, "Threads to use: %d\n", threadsCount );

  /* Allocate the input chunks buffer. The actual input chunks are malloc()ed
  dynamically, since they are consumed randomly. The inChunksFree variable
  holds the number of chunks we can malloc(). */

  inChunksCount = threadsCount * 2;
  inChunksFree = inChunksCount;
  inChunks = ( bz_stream ** ) allocMem( sizeof( bz_stream * ) * inChunksCount );

  /* Ok, now allocate output chunks */

  outChunksCount = threadsCount * 2 + 1;
  outChunks = ( bz_stream ** ) allocMem( sizeof( bz_stream * ) *
                                         outChunksCount );

  for( x = outChunksCount; x--; )
    outChunks[ x ] = NULL;

  /* Output chunks allocated. */

  /* Create compression threads */
  for( x = 0; x < (int)threadsCount; ++x )
  {
    if ( pthread_create( &dummy, NULL, &threadFunction, NULL ) )
    {
      note( 0, "Failed to create compression thread(s).\n" );
      return 1;
    }
  }
  /* All compression threads started, and are waiting for some data to eat. */

  /* Start the writer thread */
  if ( pthread_create( &dummy, NULL, &writerThread, NULL ) )
  {
    note( 0, "Failed to create writer thread.\n" );
    return 1;
  }

  /* This main thread acts only as a data reader. The compression threads
  pick the data up and process it, and the writer thread writes down the
  results.*/

  for( ; ; )
  {
    bz_stream * strm = (bz_stream *)allocMem( sizeof( bz_stream ) );
    int lastBlock = 1;

    /* Get an input chunk to place input data to */
    
    pthread_mutex_lock( &inChunksMutex );
    if ( !inChunksFree )
    {
      /* No input chunks currently available, wait for one to become free */
      pthread_cond_wait( &inChunksFreeCondition, &inChunksMutex );
    }

    /* We just use it without advancing the head. Since we're the only one
    who store to the head, it is safe */
    inChunks[ inChunksHead ] = strm;

    pthread_mutex_unlock( &inChunksMutex );

    /* Init the compression */
    strm->bzalloc = NULL;
    strm->bzfree = NULL;
    strm->opaque = NULL;

    x = BZ2_bzCompressInit ( strm, blockSize100k, 0, 0, 1 );

    if ( x != BZ_OK )
    {
      note( 0, "bzip2 compress init returned error code %d\n", x );
      exit( 1 );
    }

    /* Any and all output data will be produced in a writer thread */
    strm->avail_out = 0;

    if ( wasStateSaved )
      BZ2_bzCompressRestoreState( strm, &savedState );

    /* Feed the compressor until it can't handle more */

    for( ; ; )
    {
      if ( inputBufLeft )
      {
        strm->next_in = inputBufPtr;
        strm->avail_in = inputBufLeft;
  
        x = BZ2_bzCompress( strm, BZ_RUN|BZ_STOP_BEFORE_BLOCKSORT );
  
        if ( x == BZ_STOPPED_BEFORE_BLOCKSORT )
        {
          /* The compressor can't handle no more. Leave the rest to the
          compression thread */
          inputBufLeft = strm->avail_in;
          inputBufPtr = strm->next_in;

          lastBlock = 0;
          break;
        }

        if ( x != BZ_RUN_OK )
        {
          note( 0, "bzip2 compressing routine (input) returned "
                   "error code %d\n", x );
          return 1;
        }
        else
          inputBufLeft = 0;
      }
      else
      {
        inputBufLeft = fread( inputBuf, 1, sizeof( inputBuf ), stdin );

        if ( !inputBufLeft )
        {
          /* eof or error */

          if ( feof( stdin ) )
          {
            break;
          }
          else
          {
            /* 0 bytes read, and not on feof -- must be ferror */

            note( 0, "error reading data from stdin\n" );

            return 1;
          }
        }

        inputBufPtr = inputBuf;
      }
    }

    if ( !lastBlock )
    {
      /* Save the state for the future blocks */
      BZ2_bzCompressSaveStateBeforeBlocksort( strm, &savedState );
      wasStateSaved = 1;
    }
    else
    {
      /* Finish up the block, so it goes to the blocksort stage */
      strm->avail_in = 0;
      x = BZ2_bzCompress( strm, BZ_FINISH|BZ_STOP_BEFORE_BLOCKSORT );
      if ( x != BZ_STOPPED_BEFORE_BLOCKSORT )
      {
        note( 0, "Error: bzip2 compressing routine (finish) "
                 "returned error code %d\n", x );
        return 1;
      }
    }

    /* Save the info on whether more blocks will follow or not. This
    would be used to either run or finish the stream */
    /* We reuse avail_in. Way easier than putting our own superstruct. */
    strm->avail_in = lastBlock;

    /* We have made an input chunk on the input chunk ring buffer,
    now let's present it */

    pthread_mutex_lock( &inChunksMutex );

    if ( inChunksTail == -1 )
    {
      inChunksTail = inChunksHead;
      pthread_cond_signal( &inChunksAvailableCondition );
    }

    ++inChunksHead;

    if ( inChunksHead == inChunksCount )
      inChunksHead = 0;

    --inChunksFree;

    pthread_mutex_unlock( &inChunksMutex );

    if ( lastBlock )
      break;
  }

  /* Now that the data is all read, just wait until all in chunks are consumed
  and out chunks are written. */

  note( 2, "waiting for jobs to finish\n" );

  pthread_mutex_lock( &inChunksMutex );
  while ( inChunksFree != inChunksCount )
    pthread_cond_wait( &inChunksFreeCondition, &inChunksMutex );
  pthread_mutex_unlock( &inChunksMutex );

  pthread_mutex_lock( &outChunksMutex );
  while ( outChunksTail != -1 )
    pthread_cond_wait( &outChunksAvailableCondition, &outChunksMutex );
  pthread_mutex_unlock( &outChunksMutex );

  /* Done */

  /* No further semantic cleanup is required.
  Don't care freeing up the resources, the OS must do it anyway. */

  return 0;
}
