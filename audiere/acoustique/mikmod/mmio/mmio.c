/*
 
 Mikmod Portable System Management Facilities (the MMIO)

  By Jake Stine of Divine Entertainment (1996-2000) and
     Jean-Paul Mikkers (1993-1996).

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 --------------------------------------------------
 module: mmio.c

 Miscellaneous portable I/O routines.. used to solve some portability
 issues (like big/little endian machines and word alignment in structures).

 Notes:
  Expanded to allow for streaming from a memory block as well as from
  a file.  Also, set up in a manner which allows easy use of packfiles
  without having to overload any functions (faster and easier!).

 Portability:
  All systems - all compilers

 -----------------------------------
 The way this module works - By Jake Stine [Air Richter]


 - _mm_read_I_UWORD and _mm_read_M_UWORD have distinct differences:
   the first is for reading data written by a little endian (intel) machine,
   and the second is for reading big endian (Mac, RISC, Alpha) machine data.

 - _mm_write functions work the same as the _mm_read functions.

 - _mm_read_string is for reading binary strings.  It is basically the same
   as an fread of bytes.
*/                                                                                     

#include "mmio.h"
#include <string.h>

#ifndef COPY_BUFSIZE
#define COPY_BUFSIZE  1024
#endif

static UBYTE  _mm_cpybuf[COPY_BUFSIZE];


// =====================================================================================
    MMSTREAM *_mmstream_createfp(FILE *fp, int iobase)
// =====================================================================================
// Creates an MMSTREAM structure from the given file pointer and seekposition
{
    MMSTREAM     *mmf;

    mmf = (MMSTREAM *)_mm_malloc(NULL, sizeof(MMSTREAM));

    mmf->fp      = fp;
    mmf->iobase  = iobase;
    mmf->dp      = NULL;
    mmf->seekpos = 0;

    mmf->fread    = fread;
    mmf->fwrite   = fwrite;
    mmf->fgetc    = fgetc;
    mmf->fputc    = fputc;
    mmf->fseek    = fseek;
    mmf->ftell    = ftell;
    mmf->feof     = feof;

    return mmf;
}


// =====================================================================================
    MMSTREAM *_mmstream_createmem(void *data, int iobase)
// =====================================================================================
// Creates an MMSTREAM structure from the given memory pointer and seekposition
{
    MMSTREAM *mmf;

    mmf = (MMSTREAM *)_mm_malloc(NULL, sizeof(MMSTREAM));

    mmf->dp        = (UBYTE *)data;
    mmf->iobase    = iobase;
    mmf->fp        = NULL;
    mmf->seekpos   = 0;

    mmf->fread    = fread;
    mmf->fwrite   = fwrite;
    mmf->fgetc    = fgetc;
    mmf->fputc    = fputc;
    mmf->fseek    = fseek;
    mmf->ftell    = ftell;
    mmf->feof     = feof;

    return mmf;
}


// =====================================================================================
    void _mmstream_setapi(MMSTREAM *mmf, 
           int (__cdecl *fread)(void *buffer, size_t size, size_t count, FILE *stream),
           int (__cdecl *fwrite)(const void *buffer, size_t size, size_t count, FILE *stream),
           int (__cdecl *fgetc)(FILE *stream),
           int (__cdecl *fputc)(int c, FILE *stream),
           int (__cdecl *fseek)(FILE *stream, long offset, int origin),
           int (__cdecl *ftell)(FILE *stream),
           int (__cdecl *feof)(FILE *stream))
// =====================================================================================
{
    if(!mmf) return;

    mmf->fread    = fread;
    mmf->fwrite   = fwrite;
    mmf->fgetc    = fgetc;
    mmf->fputc    = fputc;
    mmf->fseek    = fseek;
    mmf->ftell    = ftell;
    mmf->feof     = feof;
}


// =====================================================================================
    void _mmstream_delete(MMSTREAM *mmf)
// =====================================================================================
{
    _mm_free(NULL, mmf);
}


// =====================================================================================
    void StringWrite(const CHAR *s, MMSTREAM *fp)
// =====================================================================================
// Specialized file output procedure.  Writes a UWORD length and then a
// string of the specified length (no NULL terminator) afterward.
{
    int slen;

    if(s==NULL)
    {   _mm_write_I_UWORD(0,fp);
    } else
    {   _mm_write_I_UWORD(slen = strlen(s),fp);
        _mm_write_UBYTES((UBYTE *)s,slen,fp);
    }
}


// =====================================================================================
    CHAR *StringRead(MMSTREAM *fp)
// =====================================================================================
// Reads strings written out by StringWrite above:  a UWORD length followed by length 
//characters.  A NULL is added to the string after loading.
{
    CHAR  *s;
    UWORD len;

    len = _mm_read_I_UWORD(fp);
    if(len==0)
    {   s = _mm_calloc(NULL, 16, sizeof(CHAR));
    } else
    {   if((s = (CHAR *)_mm_malloc(NULL, len+1)) == NULL) return NULL;
        _mm_read_UBYTES((UBYTE *)s,len,fp);
        s[len] = 0;
    }

    return s;
}


// =====================================================================================
    MMSTREAM *_mm_fopen(const CHAR *fname, const CHAR *attrib)
// =====================================================================================
{
    FILE     *fp;
    MMSTREAM *mfp;

    if((fp=fopen(fname,attrib)) == NULL)
    {   //CHAR   sbuf[256];
        // this should check attributes and build a more appropriate error!
        
        //sprintf(sbuf,"Error opening file: %s",fname);
        //_mmerr_set(MMERR_OPENING_FILE, sbuf);
        _mmlogd2("Error opening file: %s > %s",fname, _sys_errlist[errno]);
        return NULL;
    }

    mfp           = _mm_calloc(NULL, 1,sizeof(MMSTREAM));
    mfp->fp       = fp;
    mfp->iobase   = 0;
    mfp->dp       = NULL;

    mfp->fread    = fread;
    mfp->fwrite   = fwrite;
    mfp->fgetc    = fgetc;
    mfp->fputc    = fputc;
    mfp->fseek    = fseek;
    mfp->ftell    = ftell;
    mfp->feof     = feof;

    return mfp;
}


// =====================================================================================
    void _mm_fclose(MMSTREAM *mmfile)
// =====================================================================================
{
    if(mmfile)
    {   if(mmfile->fp) fclose(mmfile->fp);
        _mm_free(NULL, mmfile);
    }
}


// =====================================================================================
    int _mm_fseek(MMSTREAM *stream, long offset, int whence)
// =====================================================================================
{
    if(!stream) return 0;

    if(stream->fp)
    {   // file mode...
        return stream->fseek(stream->fp,(whence==SEEK_SET) ? offset+stream->iobase : offset, whence);
    } else
    {   long   tpos;
        switch(whence)
        {   case SEEK_SET: tpos = offset;                   break;
            case SEEK_CUR: tpos = stream->seekpos + offset; break;
            case SEEK_END: /*tpos = stream->length + offset;*/  break; // not supported!
        }
        if((tpos < 0) /*|| (stream->length && (tpos > stream->length))*/) return 1; // seek failed
        stream->seekpos = tpos;
    }

    return 0;
}


// =====================================================================================
    long _mm_ftell(MMSTREAM *stream)
// =====================================================================================
{
   if(!stream) return 0;
   return (stream->fp) ? (stream->ftell(stream->fp) - stream->iobase) : stream->seekpos;
}


// =====================================================================================
    BOOL _mm_feof(MMSTREAM *stream)
// =====================================================================================
{
    if(!stream) return 1;
    if(stream->fp) return feof(stream->fp);

    return 0;
}


// =====================================================================================
    BOOL _mm_fexist(CHAR *fname)
// =====================================================================================
{
   FILE *fp;
   
   if((fp=fopen(fname,"r")) == NULL) return 0;
   fclose(fp);

   return 1;
}


// =====================================================================================
    long _mm_flength(FILE *stream)
// =====================================================================================
{
   long tmp,tmp2;

   tmp = ftell(stream);
   fseek(stream,0,SEEK_END);
   tmp2 = ftell(stream);
   fseek(stream,tmp,SEEK_SET);
   return tmp2-tmp;
}


// =====================================================================================
    BOOL _mm_copyfile(FILE *fpi, FILE *fpo, uint len)
// =====================================================================================
// Copies a given number of bytes from the source file to the destination file.  Copy 
// begins whereever the current read pointers are for the given files.  Returns 0 on error.
{
    ULONG todo;

    while(len)
    {   todo = (len > COPY_BUFSIZE) ? COPY_BUFSIZE : len;
        if(!fread(_mm_cpybuf, todo, 1, fpi))
        {   _mmerr_set(MMERR_END_OF_FILE, "Unexpected End of File");
            return 0;
        }
        if(!fwrite(_mm_cpybuf, todo, 1, fpo))
        {   _mmerr_set(MMERR_DISK_FULL, "Disk Full Error. Ouch.");
            return 0;
        }
        len -= todo;
    }

    return -1;
}


// =====================================================================================
    void _mm_write_string(const CHAR *data, MMSTREAM *fp)
// =====================================================================================
{
    if(data) _mm_write_UBYTES((UBYTE *)data, strlen(data), fp);
}


// =====================================================================================
    void _mm_fputs(MMSTREAM *fp, CHAR *data)
// =====================================================================================
{
   if(data) _mm_write_UBYTES((UBYTE *)data, strlen(data), fp);

#ifndef __UNIX__
   _mm_write_UBYTE(13,fp);
#endif
   _mm_write_UBYTE(10,fp);
}


// =====================================================================================
//                            THE OUTPUT (WRITE) SECTION
// =====================================================================================


#define filewrite_SBYTE(x,y)      y->fputc((int)x,y->fp)
#define filewrite_UBYTE(x,y)      y->fputc((int)x,y->fp)
#define datawrite_SBYTE(x,y)      y->dp[y->seekpos++] = (UBYTE)x
#define datawrite_UBYTE(x,y)      y->dp[y->seekpos++] = x

#define filewrite_I_SWORD(x,fp) filewrite_SBYTE(x,fp); filewrite_UBYTE(x>>8,fp)
#define filewrite_I_UWORD(x,fp) filewrite_SBYTE(x,fp); filewrite_UBYTE(x>>8,fp)
#define datawrite_I_SWORD(x,fp) datawrite_SBYTE(x,fp); datawrite_UBYTE(x>>8,fp)
#define datawrite_I_UWORD(x,fp) datawrite_SBYTE(x,fp); datawrite_UBYTE(x>>8,fp)
#define filewrite_I_SLONG(x,fp) filewrite_I_SWORD(x,fp); filewrite_I_UWORD(x>>16,fp)
#define filewrite_I_ULONG(x,fp) filewrite_I_UWORD(x,fp); filewrite_I_UWORD(x>>16,fp)
#define datawrite_I_SLONG(x,fp) datawrite_I_SWORD(x,fp); datawrite_I_UWORD(x>>16,fp)
#define datawrite_I_ULONG(x,fp) datawrite_I_UWORD(x,fp); datawrite_I_UWORD(x>>16,fp)
#define filewrite_M_SWORD(x,fp) filewrite_SBYTE(x>>8,fp); filewrite_UBYTE(x,fp)
#define filewrite_M_UWORD(x,fp) filewrite_UBYTE(x>>8,fp); filewrite_UBYTE(x,fp)
#define datawrite_M_SWORD(x,fp) datawrite_SBYTE(x>>8,fp); datawrite_UBYTE(x,fp)
#define datawrite_M_UWORD(x,fp) datawrite_UBYTE(x>>8,fp); datawrite_UBYTE(x,fp)
#define filewrite_M_SLONG(x,fp) filewrite_M_SWORD(x>>16,fp); filewrite_M_UWORD(x,fp)
#define filewrite_M_ULONG(x,fp) filewrite_M_UWORD(x>>16,fp); filewrite_M_UWORD(x,fp)
#define datawrite_M_SLONG(x,fp) datawrite_M_SWORD(x>>16,fp); datawrite_M_UWORD(x,fp)
#define datawrite_M_ULONG(x,fp) datawrite_M_UWORD(x>>16,fp); datawrite_M_UWORD(x,fp)


void _mm_write_SBYTE(SBYTE data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_SBYTE(data,fp); } else { datawrite_SBYTE(data,fp); }
}

void _mm_write_UBYTE(UBYTE data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_UBYTE(data,fp); } else { datawrite_UBYTE(data,fp); }
}

#ifdef _MSC_VER
#pragma optimize( "g", off )
#endif

void _mm_write_M_UWORD(UWORD data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_M_UWORD(data,fp); } else { datawrite_M_UWORD(data,fp); }
}

void _mm_write_I_UWORD(UWORD data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_I_UWORD(data,fp); } else { datawrite_I_UWORD(data,fp); }
}

void _mm_write_M_ULONG(ULONG data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_M_ULONG(data,fp); } else { datawrite_M_ULONG(data,fp); }
}

void _mm_write_I_ULONG(ULONG data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_I_ULONG(data,fp); } else { datawrite_I_ULONG(data,fp); }
}

void _mm_write_M_SWORD(SWORD data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_M_SWORD(data,fp); } else { datawrite_M_SWORD(data,fp); }
}

void _mm_write_I_SWORD(SWORD data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_I_SWORD(data,fp); } else { datawrite_I_SWORD(data,fp); }
}

void _mm_write_M_SLONG(SLONG data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_M_SLONG(data,fp); } else { datawrite_M_SLONG(data,fp); }
}

void _mm_write_I_SLONG(SLONG data, MMSTREAM *fp)
{
    if(fp->fp) { filewrite_I_SLONG(data,fp); } else { datawrite_I_SLONG(data,fp); }
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif


#define filewrite_SBYTES(x,y,z)  z->fwrite((void *)x,1,y,z->fp)
#define filewrite_UBYTES(x,y,z)  z->fwrite((void *)x,1,y,z->fp)
#define datawrite_SBYTES(x,y,z)  memcpy(&z->dp[z->seekpos],(void *)x,y)
#define datawrite_UBYTES(x,y,z)  memcpy(&z->dp[z->seekpos],(void *)x,y)

#ifdef MM_BIG_ENDIAN
#define filewrite_M_SWORDS(x,y,z) z->fwrite((void *)x,1,y*sizeof(SWORD),z->fp)
#define filewrite_M_UWORDS(x,y,z) z->fwrite((void *)x,1,y*sizeof(UWORD),z->fp)
#define filewrite_M_SLONGS(x,y,z) z->fwrite((void *)x,1,y*sizeof(SLONG),z->fp)
#define filewrite_M_ULONGS(x,y,z) z->fwrite((void *)x,1,y*sizeof(ULONG),z->fp)
#define datawrite_M_SWORDS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(SWORD))
#define datawrite_M_UWORDS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(UWORD))
#define datawrite_M_SLONGS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(SLONG))
#define datawrite_M_ULONGS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(ULONG))
#else
#define filewrite_I_SWORDS(x,y,z) z->fwrite((void *)x,1,y*sizeof(SWORD),z->fp)
#define filewrite_I_UWORDS(x,y,z) z->fwrite((void *)x,1,y*sizeof(UWORD),z->fp)
#define filewrite_I_SLONGS(x,y,z) z->fwrite((void *)x,1,y*sizeof(SLONG),z->fp)
#define filewrite_I_ULONGS(x,y,z) z->fwrite((void *)x,1,y*sizeof(ULONG),z->fp)
#define datawrite_I_SWORDS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(SWORD))
#define datawrite_I_UWORDS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(UWORD))
#define datawrite_I_SLONGS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(SLONG))
#define datawrite_I_ULONGS(x,y,z) memcpy(&z->dp[z->seekpos],(void *)x,y*sizeof(ULONG))
#endif


#define DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN(type_name, type)   \
void                                                             \
_mm_write_##type_name##S (const type *data, int number, MMSTREAM *fp)  \
{                                                                \
    if(fp->fp)                                                   \
    {   while(number>0)                                          \
        {   filewrite_##type_name(*data, fp);                    \
            number--;  data++;                                   \
        }                                                        \
    } else                                                       \
    {   while(number>0)                                          \
        {   datawrite_##type_name(*data, fp);                    \
            number--;  data++;                                   \
        }                                                        \
    }                                                            \
}


#define DEFINE_MULTIPLE_WRITE_FUNCTION_NORM(type_name, type)     \
void                                                             \
_mm_write_##type_name##S (const type *data, int number, MMSTREAM *fp)  \
{                                                                \
    if(fp->fp)                                                   \
    {   filewrite_##type_name##S(data,number,fp);                \
    } else                                                       \
    {   datawrite_##type_name##S(data,number,fp);                \
        fp->seekpos += number;                                   \
    }                                                            \
}

DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (SBYTE, SBYTE)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (UBYTE, UBYTE)


#ifdef _MSC_VER
#pragma optimize( "g", off )
#endif

#ifdef MM_BIG_ENDIAN
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (I_SWORD, SWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (I_UWORD, UWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (I_SLONG, SLONG)
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (I_ULONG, ULONG)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (M_SWORD, SWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (M_UWORD, UWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (M_SLONG, SLONG)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (M_ULONG, ULONG)
#else
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (M_SWORD, SWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (M_UWORD, UWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (M_SLONG, SLONG)
DEFINE_MULTIPLE_WRITE_FUNCTION_ENDIAN (M_ULONG, ULONG)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (I_SWORD, SWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (I_UWORD, UWORD)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (I_SLONG, SLONG)
DEFINE_MULTIPLE_WRITE_FUNCTION_NORM   (I_ULONG, ULONG)
#endif

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif


// =====================================================================================
//                              THE INPUT (READ) SECTION
// =====================================================================================

#define fileread_SBYTE(x)         (SBYTE)x->fgetc(x->fp)
#define fileread_UBYTE(x)         (UBYTE)x->fgetc(x->fp)
#define dataread_SBYTE(y)         y->dp[y->seekpos++]
#define dataread_UBYTE(y)         y->dp[y->seekpos++]

#define fileread_I_SWORD(fp) ((SWORD)(fileread_UBYTE(fp) | (fileread_UBYTE(fp)<<8)))
#define fileread_I_UWORD(fp) ((UWORD)(fileread_UBYTE(fp) | (fileread_UBYTE(fp)<<8)))
#define dataread_I_SWORD(fp) ((SWORD)(dataread_UBYTE(fp) | (dataread_UBYTE(fp)<<8)))
#define dataread_I_UWORD(fp) ((UWORD)(dataread_UBYTE(fp) | (dataread_UBYTE(fp)<<8)))
#define fileread_I_SLONG(fp) ((SLONG)(fileread_I_UWORD(fp) | (fileread_I_UWORD(fp)<<16)))
#define fileread_I_ULONG(fp) ((ULONG)(fileread_I_UWORD(fp) | (fileread_I_UWORD(fp)<<16)))
#define dataread_I_SLONG(fp) ((SLONG)(dataread_I_UWORD(fp) | (dataread_I_UWORD(fp)<<16)))
#define dataread_I_ULONG(fp) ((ULONG)(dataread_I_UWORD(fp) | (dataread_I_UWORD(fp)<<16)))
#define fileread_M_SWORD(fp) ((SWORD)((fileread_UBYTE(fp)<<8) | fileread_UBYTE(fp)))
#define fileread_M_UWORD(fp) ((UWORD)((fileread_UBYTE(fp)<<8) | fileread_UBYTE(fp)))
#define dataread_M_SWORD(fp) ((SWORD)((dataread_UBYTE(fp)<<8) | dataread_UBYTE(fp)))
#define dataread_M_UWORD(fp) ((UWORD)((dataread_UBYTE(fp)<<8) | dataread_UBYTE(fp)))
#define fileread_M_SLONG(fp) ((SLONG)((fileread_M_UWORD(fp)<<16) | fileread_M_UWORD(fp)))
#define fileread_M_ULONG(fp) ((ULONG)((fileread_M_UWORD(fp)<<16) | fileread_M_UWORD(fp)))
#define dataread_M_SLONG(fp) ((SLONG)((dataread_M_UWORD(fp)<<16) | dataread_M_UWORD(fp)))
#define dataread_M_ULONG(fp) ((ULONG)((dataread_M_UWORD(fp)<<16) | dataread_M_UWORD(fp)))


// ============
//   UNSIGNED
// ============

UBYTE _mm_read_UBYTE(MMSTREAM *fp)
{
    return((fp->fp) ? fileread_UBYTE(fp) : dataread_UBYTE(fp));
}

SBYTE _mm_read_SBYTE(MMSTREAM *fp)
{
    return((fp->fp) ? fileread_SBYTE(fp) : dataread_SBYTE(fp));
}


#ifdef _MSC_VER
#pragma optimize( "g", off )
#endif

UWORD _mm_read_I_UWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_UWORD(fp) : dataread_I_UWORD(fp);
}

UWORD _mm_read_M_UWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_UWORD(fp) : dataread_M_UWORD(fp);
}

ULONG _mm_read_I_ULONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_ULONG(fp) : dataread_I_ULONG(fp);
}

ULONG _mm_read_M_ULONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_ULONG(fp) : dataread_M_ULONG(fp);
}

SWORD _mm_read_M_SWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_SWORD(fp) : dataread_M_SWORD(fp);
}

SWORD _mm_read_I_SWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_SWORD(fp) : dataread_I_SWORD(fp);
}

SLONG _mm_read_M_SLONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_SLONG(fp) : dataread_M_SLONG(fp);
}

SLONG _mm_read_I_SLONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_SLONG(fp) : dataread_I_SLONG(fp);
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif


int _mm_read_string(CHAR *buffer, int number, MMSTREAM *fp)
{
    if(fp->fp)
    {   fp->fread(buffer,sizeof(CHAR),number,fp->fp);
        return !feof(fp->fp);
    } else
    {   memcpy(buffer,&fp->dp[fp->seekpos], sizeof(CHAR) * number);
        fp->seekpos += number;
        return 0;
    }
}

#define fileread_SBYTES(x,y,z)  z->fread((void *)x,1,y,z->fp)
#define fileread_UBYTES(x,y,z)  z->fread((void *)x,1,y,z->fp)
#define dataread_SBYTES(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y)
#define dataread_UBYTES(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y)

#ifdef MM_BIG_ENDIAN
#define fileread_M_SWORDS(x,y,z)  z->fread((void *)x,1,y*sizeof(SWORD),z->fp)
#define fileread_M_UWORDS(x,y,z)  z->fread((void *)x,1,y*sizeof(UWORD),z->fp)
#define fileread_M_SLONGS(x,y,z)  z->fread((void *)x,1,y*sizeof(SLONG),z->fp)
#define fileread_M_ULONGS(x,y,z)  z->fread((void *)x,1,y*sizeof(ULONG),z->fp)
#define dataread_M_SWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SWORD))
#define dataread_M_UWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(UWORD))
#define dataread_M_SLONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SLONG))
#define dataread_M_ULONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(ULONG))
#else
#define fileread_I_SWORDS(x,y,z)  z->fread((void *)x,1,y*sizeof(SWORD),z->fp)
#define fileread_I_UWORDS(x,y,z)  z->fread((void *)x,1,y*sizeof(UWORD),z->fp)
#define fileread_I_SLONGS(x,y,z)  z->fread((void *)x,1,y*sizeof(SLONG),z->fp)
#define fileread_I_ULONGS(x,y,z)  z->fread((void *)x,1,y*sizeof(ULONG),z->fp)
#define dataread_I_SWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SWORD))
#define dataread_I_UWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(UWORD))
#define dataread_I_SLONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SLONG))
#define dataread_I_ULONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(ULONG))
#endif


#define DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN(type_name, type)    \
int                                                              \
_mm_read_##type_name##S (type *buffer, int number, MMSTREAM *fp) \
{                                                                \
    if(fp->fp)                                                   \
    {   while(number>0)                                          \
        {   *buffer = fileread_##type_name(fp);                  \
            number--;  buffer++;                                 \
        }                                                        \
    } else                                                       \
    {   while(number>0)                                          \
        {   *buffer = dataread_##type_name(fp);                  \
            number--;  buffer++;                                 \
        }                                                        \
    }                                                            \
    return !_mm_feof(fp);                                        \
}


#define DEFINE_MULTIPLE_READ_FUNCTION_NORM(type_name, type)      \
int                                                              \
_mm_read_##type_name##S (type *buffer, int number, MMSTREAM *fp) \
{                                                                \
    if(fp->fp)                                                   \
    {   fileread_##type_name##S(buffer,number,fp);               \
    } else                                                       \
    {   dataread_##type_name##S(buffer,number,fp);               \
        fp->seekpos += number;                                   \
    }                                                            \
    return !_mm_feof(fp);                                        \
}

DEFINE_MULTIPLE_READ_FUNCTION_NORM   (SBYTE, SBYTE)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (UBYTE, UBYTE)

#ifdef _MSC_VER
#pragma optimize( "g", off )
#endif

#ifdef MM_BIG_ENDIAN
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_ULONG, ULONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_ULONG, ULONG)
#else
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_ULONG, ULONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_ULONG, ULONG)
#endif
