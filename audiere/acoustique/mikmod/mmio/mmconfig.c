/*
 
 Mikmod Portable System Management Facilities (the MMIO)

  By Jake Stine of Divine Entertainment (1996-2000) and

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 ------------------------------------------------------
 module: mmconfig.c

 Routines for loading the individual parts of a configuration file. This
 configuration code adheres to the basic standard of unix/windows-style
 conf/ini files.

 A configuration file would look something like this -

 [Video]
 page_flipping       = enabled
 physical_resolution = 640x480

 # this line is a comment, ignored by the reader.
 [Audio]
 master_volume       = 60
 music_volume        = 100
 soundfx_volume      = 50

 Notes:
  Don't put '=' or '[' or ']' in  the variable names.  Those are the char-
  acters the parser uses for delimiting.  The rvalue (right-side value) can
  contain any character except '#' however.
*/

#include "mmio.h"
#include "mmconfig.h"
#include <string.h>

#define WhiteSpace(x)     ((x <= 32 && x) || (x == 255))
#define NotWhiteSpace(x)  (!(x <= 32) && (x != 255))


#ifndef CFGLEN_THRESHOLD
#define CFGLEN_THRESHOLD   256       // configuration line-by-line
#endif

#ifndef SUBSEC_THRESHOLD
#define SUBSEC_THRESHOLD    16       // subsection info allocations
#endif


#define MMCONF_ARRAY_DELIMITOR  ','
#define MMCONF_REQARRAY_MAX      8


// =====================================================================================
    static BOOL ParseSubSection(MM_CONFIG *conf, CHAR *line)
// =====================================================================================
{
    int i=0,cpos;

    while(WhiteSpace(line[i])) i++;

    switch(line[i])
    {   case NULL:
        case  '#': break;

        case  '[':
            i++; cpos = 0;
            while((line[i] != ']') && line[i]) i++;
            if(!line[i])
            {   _mmlog("mmconfig > Line %d: Missing closing bracket",conf->length);
                return 0;
            }
        break;

        default:
            while((line[i] != '=') && line[i]) i++;
            if(!line[i])
            {   _mmlog("mmconfig > Line %d: Meaningless use of an expression",conf->length);
            }
            while(WhiteSpace(line[i])) i++;
            if(!line[i])
            {   _mmlog("mmconfig > Line %d: Missing value to right of variable",conf->length);
            }
        break;
    }
    return -1;
}


// =====================================================================================
    static BOOL AllocLine(MM_CONFIG *conf, uint numlines)
// =====================================================================================
{
    if((conf->length+numlines) >= conf->length_alloc)
    {   uint  i;

        if((conf->line = (CHAR **)_mm_realloc(conf->allochandle, conf->line, (conf->length_alloc + CFGLEN_THRESHOLD) * (sizeof(CHAR *)))) == NULL)
        {   _mmcfg_exit(conf);
            return 0;
        }

        i = conf->length_alloc;
        conf->length_alloc += CFGLEN_THRESHOLD;
        for(; i<conf->length_alloc; i++) conf->line[i] = NULL;
    }
    return -1;
}


// =====================================================================================
    static BOOL AllocSubsection(MM_CONFIG *conf, const CHAR *str, uint line)
// =====================================================================================
// Add a subsection to the subsection list.  Note that this does not alter the
// configuration file in any way.  This function should only be called if the
// subsection already exists in the configuration file.
{
    if(conf->numsubsec >= conf->subsec_alloc)
    {   conf->subsec = (MMCFG_SUBSEC *)_mm_realloc(conf->allochandle, conf->subsec, (conf->subsec_alloc + SUBSEC_THRESHOLD) * sizeof(MMCFG_SUBSEC));
        conf->subsec_alloc += SUBSEC_THRESHOLD;
    }

    conf->subsec[conf->numsubsec].name      = _mm_strdup(conf->allochandle, str);
    conf->subsec[conf->numsubsec].line      = line;
    conf->subsec[conf->numsubsec].insertpos = 1;
    conf->numsubsec++;

    return -1;
}


// =====================================================================================
    static BOOL ListSubsections(MM_CONFIG *conf)
// =====================================================================================
// Creates a list of all subsections in this configuration file.  This allows
// quicker indexing and access, and also allows the end user program to util-
// ize dynamic subsection names.
{
    uint    i,cpos,vlen;

    for(i=0; i<conf->length; i++)
    {   CHAR  *line;
    
        if(line = conf->line[i])
        {   cpos=0; while(WhiteSpace(line[cpos])) cpos++;
            if(line[cpos] == '[')
            {   cpos++; vlen=0;
                while(line[cpos] != ']' && line[cpos])
                {   conf->work[vlen] = line[cpos];
                    cpos++; vlen++;
                }
                conf->work[vlen] = 0;

                AllocSubsection(conf, conf->work, i);
            }
        }
    }

    return -1;
}


// =====================================================================================
    static MM_CONFIG *_mmcfg_initfp(MMSTREAM *fp)
// =====================================================================================
// loads in the entire configuration file and processes it for optimized
// indexing of the sub-section indexes.  Not to be called by the user!
{
    int        cpos;                  // character position on line
    MM_CONFIG  *conf;

    MM_ALLOC  *allochandle = _mmalloc_create("mmconfig", NULL);

    conf              = (MM_CONFIG *)_mm_calloc(allochandle, 1,sizeof(MM_CONFIG));
    conf->allochandle = allochandle;
    conf->work        = (CHAR *)_mm_malloc(conf->allochandle, 512);

    // read in all lines of the configuration file and mark each of the
    // bracketed sub-headers for future search referencing. ALSO: Check
    // syntax for validity.  If the syntax has major errors, then the 
    // configuration loading will be cancelled.

    if(fp)
    {   UBYTE      c,c2;

        c = _mm_read_UBYTE(fp);

        do
        {   cpos = 0;
            while(c!=13 && c!=10 && !_mm_feof(fp))
            {   conf->work[cpos] = c;
                cpos++;
                c = _mm_read_UBYTE(fp);
            }

            AllocLine(conf, 2);

            if(cpos==0)
                conf->line[conf->length] = NULL;
            else
            {   // check the syntax of this line for validity
                conf->work[cpos] = 0;
                if(!ParseSubSection(conf, conf->work))
                {   _mmcfg_exit(conf);
                    return NULL;
                }

                conf->line[conf->length] = _mm_strdup(conf->allochandle, conf->work);
            }

            c2 = _mm_read_UBYTE(fp);
            if(c2!=c && (c2==10 || c2==13))
                c = _mm_read_UBYTE(fp);
            else
                c = c2;
        
            conf->length++;
        } while(!_mm_feof(fp));
    }

    ListSubsections(conf);

    return conf;
}


// =====================================================================================
    MM_CONFIG *_mmcfg_initfn(CHAR *fname)
// =====================================================================================
// opens the specified filename for reading / writing and then calls the
// main intialization procedure [detailed below].

{
    MMSTREAM  *fp;
    MM_CONFIG  *conf;

    fp = _mm_fopen(fname,"rb");

    conf = _mmcfg_initfp(fp);
    _mm_fclose(fp);

    if(!conf) return NULL;

    //conf->fp    = _mm_fopen(fname, "wb");
    conf->fname = _mm_strdup(conf->allochandle, fname);

    return conf;
}


// =====================================================================================
    void _mmcfg_exit(MM_CONFIG *conf)
// =====================================================================================
// Frees up all resources allocated by the configuration libraries and saves
// any changes made to the configuration file.
{
    if(!conf) return;
    if(conf->changed) _mmcfg_save(conf);

    _mmalloc_close(conf->allochandle);
}


// =====================================================================================
    void _mmcfg_enable_autofix(MM_CONFIG *conf, uint buflen)
// =====================================================================================
// Enables configuration autofix, and sets the default buffer elngth to the given value.
{
    if(conf)
    {   conf->flags |= MMCONF_AUTOFIX;
        conf->buflen = (buflen > 64) ? 64 : buflen;
    }
}

#define ValidCharacter(val)  ((val != '=') && (val != '[') && (val != ']') && (val != ',') && (val != ';'))

// =====================================================================================
    CHAR *_mmcfg_fixname(CHAR *dest, const CHAR *var)
// =====================================================================================
// Search through and strip out bad instances of characters '][=' - if these are the only
// characters in the name, then a suitable typed-out version is created.  The destination
// string should be 48 characters long.
//
// If the name sucks so much that this thing just can't make a legal name out of it, then
// it returns NULL, otherwise it returns a pointer to the string passed in 'dest.'
{
    uint    i, valid, len = strlen(var);
    
    if(!var || !dest) return NULL;
    
    // Step 1.  Check to see if we can just strip chars or if we sub in names
    // This requires checking to see if the name contains any valid characters at all.
    
    for(i=0, valid=0; i<len; i++)
    {
        if(ValidCharacter(var[i]))
        valid++;
    }

    // If valid == length, then no illegal chars and we go home!
    
    if(valid == len)
        strcpy(dest, var);
    else
    {   if(valid > 3)
        {   for(i=0, valid=0; i<len; i++)
            {   if(ValidCharacter(var[i]))
                    dest[valid++] = var[i];
            }
        } else
        {   for(i=0, valid=0; i<len && valid<48; i++)
            {   switch(var[i])
                {
                    case '=':
                        memcpy(&dest[valid],"Equals",6);
                        valid += 6;
                    break;
                
                    case '[':
                        memcpy(&dest[valid],"OpenBracket", 11);
                        valid += 11;
                    break;
                
                    case ']':
                        memcpy(&dest[valid],"CloseBracket", 12);
                        valid += 12;
                    break;

                    case ',':
                        memcpy(&dest[valid],"Comma", 12);
                        valid += 12;
                    break;

                    case ';':
                        memcpy(&dest[valid],"SemiColon", 12);
                        valid += 12;
                    break;


                    default:
                        dest[valid++] = var[i];
                    break;
                }
            }
        }
        dest[valid] = 0;
    }


    return dest;
}


// =====================================================================================
    BOOL _mmcfg_set_subsection_int(MM_CONFIG *conf, uint var, CHAR name[MMCONF_MAXNAMELEN])
// =====================================================================================
// set the active sub-section based on a simple integer index.  All requests for variable
// information will take place under this sub-section only.
{
    if(!conf) return FALSE;
    
    if(var < conf->numsubsec)
        conf->cursubsec = var;
    else
    {   _mmlogd1("Config > Invalid subsection range specified: %d",var);
        name[0] = 0;
        return FALSE;
    }

    strcpy(name, conf->subsec[var].name);

    return TRUE;
}


// =====================================================================================
    BOOL _mmcfg_set_subsection(MM_CONFIG *conf, const CHAR *var)
// =====================================================================================
// set the active sub-section.  All requests for variable information will
// take place under this sub-section only.  A second call to this function
// will change the sub-section - no other red-tape involved.
{
    uint    i;
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var) return 0;

    strncpy(work, var, MMCONF_MAXNAMELEN);
    work[MMCONF_MAXNAMELEN] = 0;

    if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(work);

    for(i=0; i<conf->numsubsec; i++)
    {   if(!strcmp(work, conf->subsec[i].name))
        {   conf->cursubsec = i;
            return TRUE;
        }
    }

    _mmlogd1("mmconfig > Requested subsection '%s' was not found.", var);

    if(conf->flags & MMCONF_AUTOFIX)
    {
        // Subsections are always inserted at the end of the file, so set the
        // current subsection accordingly and add a new one:

        conf->cursubsec = conf->numsubsec;
        _mmcfg_insert_subsection(conf, var);
    }

    return FALSE;
}


// =====================================================================================
    void __inline _mmconf_rtrim(CHAR *trim)
// =====================================================================================
// Good old rtrim, just like from the days of old in BASIC.
{
    int  i = strlen(trim)-1;

    while(i>=0 && WhiteSpace(trim[i])) i--;

    trim[i+1] = 0;
}


// =====================================================================================
    int _mmcfg_findvar(MM_CONFIG *conf, const CHAR *var)
// =====================================================================================
// searches for the given variable within the currently selected subsection
// and returns the line on which the variable is found.
// Returns -1 if the variable is not found.
{
    uint   lpos,cpos,vpos;
    CHAR  *line;

    for(lpos=conf->subsec[conf->cursubsec].line+1; lpos<conf->length; lpos++)
    {   if((line = conf->line[lpos]))
        {   cpos=0; while(WhiteSpace(line[cpos])) cpos++;
            switch(line[cpos])
            {   case  '[':  return -1;

                case NULL:
                case  '#':  break;

                default:
                    vpos = 0;
                    while((line[cpos] != '=') && line[cpos])
                    {   conf->work[vpos] = line[cpos];
                        cpos++; vpos++;
                    }

                    conf->work[vpos] = 0;
                    _mmconf_rtrim(conf->work);          // strip the right-side whitespace

                    if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(conf->work);
                    if(!strcmp(conf->work,var)) return lpos;
                break;
            }
        }
    }
    return -1;
}


// =====================================================================================
// The following three functions are designed to request certain types of
// information from any specified variable.  Only the currently indexed
// sub-section scope is searched.  The second parameter is the "default" value
// for the variable in question.

// If the search fails, the default value passed is again returned, and the
// cofiguration file is "patched" to contain the missing variable, properly
// assigned the default value.


// =====================================================================================
    BOOL _mmcfg_request_string(MM_CONFIG *conf, const CHAR *var, CHAR *val)
// =====================================================================================
{
    uint    lpos,cpos,vpos;
    CHAR   *line;
   
    for(lpos=conf->subsec[conf->cursubsec].line+1; lpos<conf->length; lpos++)
    {   if(line = conf->line[lpos])
        {   cpos=0; while(WhiteSpace(line[cpos])) cpos++;
            switch(line[cpos])
            {   case  '[':          // end of subsection
                    if(conf->flags & MMCONF_AUTOFIX)
                        _mmcfg_insert(conf, -1, var, val);
                    return 0;
                break;

                case NULL:
                case  '#':  break;

                default:
                    vpos = 0;
                    while((line[cpos] != '=') && line[cpos])
                    {   conf->work[vpos] = line[cpos];
                        cpos++; vpos++;
                    }

                    conf->work[vpos] = 0;
                    _mmconf_rtrim(conf->work);          // strip the right-side whitespace

                    if(!strcmp(conf->work,var))
                    {   
                        uint  cnt=0;

                        while(line[cpos] && (line[cpos] != '=')) cpos++;
                        cpos++;                                     // skip the equals
                        while(WhiteSpace(line[cpos])) cpos++;       // and skip the whitespace

                        // Special Logic Addition: Lets handle '#' comments that are on the
                        // same line with (but after) the data segement.  Basic task is to
                        // copy only up to the hash, and to remove whitespace at the end
                        // of the resultant.

                        if(line[cpos])
                        {   vpos = 0;
                            while(line[cpos] && (line[cpos] != '#') && (line[cpos] != MMCONF_ARRAY_DELIMITOR))
                            {   conf->work[vpos] = line[cpos];
                                cpos++; vpos++;
                            }
                            
                            conf->work[vpos] = 0;
                            _mmconf_rtrim(conf->work);          // strip the right-side whitespace
                            //reqline = lpos;
                            strcpy(val, conf->work);

                            cnt++;
                        }
                        return cnt;
                    }
                break;
            }
        }
    }

    if(conf->flags & MMCONF_AUTOFIX)
        _mmcfg_insert(conf, -1, var, val);

    return 0;
}


// =====================================================================================
    BOOL _mmcfg_reqarray_string(MM_CONFIG *conf, const CHAR *var, uint nent, CHAR *val[])
// =====================================================================================
{
    uint    lpos,cpos,vpos;
    CHAR   *line;
   
    for(lpos=conf->subsec[conf->cursubsec].line+1; lpos<conf->length; lpos++)
    {   if(line = conf->line[lpos])
        {   cpos=0; while(WhiteSpace(line[cpos])) cpos++;
            switch(line[cpos])
            {   case  '[':  return 0;  // end of subsection
                case NULL:
                case  '#':  break;

                default:
                    vpos = 0;
                    while((line[cpos] != '=') && line[cpos])
                    {   conf->work[vpos] = line[cpos];
                        cpos++; vpos++;
                    }

                    conf->work[vpos] = 0;
                    _mmconf_rtrim(conf->work);          // strip the right-side whitespace

                    if(!strcmp(conf->work,var))
                    {   
                        BOOL  stillmore = TRUE;
                        uint  cnt=0;

                        while(line[cpos] && (line[cpos] != '=')) cpos++;
                        cpos++;                                     // skip the equals

                        while(stillmore)
                        {
                            while(WhiteSpace(line[cpos])) cpos++;       // and skip the whitespace

                            // Special Logic Addition: Lets handle '#' comments that are on the
                            // same line with (but after) the data segement.  Basic task is to
                            // copy only up to the hash, and to remove whitespace at the end
                            // of the resultant.

                            if(line[cpos])
                            {   vpos = 0;
                                while(line[cpos] && (line[cpos] != '#') && (line[cpos] != MMCONF_ARRAY_DELIMITOR))
                                {   conf->work[vpos] = line[cpos];
                                    cpos++; vpos++;
                                }

                                if(line[cpos] != MMCONF_ARRAY_DELIMITOR) stillmore = FALSE;

                                conf->work[vpos] = 0;
                                _mmconf_rtrim(conf->work);          // strip the right-side whitespace
                                //reqline = lpos;
                                strcpy(val[cnt], conf->work);

                                cnt++; cpos++;
                            } else stillmore = FALSE;
                        }
                        return cnt;
                    }
                break;
            }
        }
    }

    if(conf->flags & MMCONF_AUTOFIX)
        _mmcfg_insert_array(conf, -1, var, nent, val);

    return 0;
}


// =====================================================================================
    int _mmcfg_request_integer(MM_CONFIG *conf, const CHAR *var, int val)
// =====================================================================================
{
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var || !conf->subsec) return val;

    sprintf(work,"%d",val);
    if(_mmcfg_request_string(conf, var, work)) val = atol(work);

    return val;
}


static CHAR *val_enabled  = "enabled",
            *val_true     = "true",
            *val_yes      = "yes",
            *val_on       = "on",

            *val_disabled = "disabled",
            *val_false    = "false",
            *val_off      = "off",
            *val_no       = "no";


// =====================================================================================
    BOOL _mmcfg_request_boolean(MM_CONFIG *conf, const CHAR *var, BOOL val)
// =====================================================================================
{
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var || !conf->subsec) return val;

    sprintf(work,"%s",val ? val_true : val_false);

    if(_mmcfg_request_string(conf, var, work))
    {   if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(work);

        if(strcmp(work,val_enabled)==0 || strcmp(work,val_true)==0 || strcmp(work,val_yes)==0 || strcmp(work,val_on)==0)
            val = 1;
        else if(strcmp(work,val_disabled)==0 || strcmp(work,val_false)==0 || strcmp(work,val_off)==0 || strcmp(work,val_no)==0)
            val = 0;
    }
    return val;
}


// =====================================================================================
    int _mmcfg_request_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, int val)
// =====================================================================================
{
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var || !conf->subsec) return val;

    strncpy(work, enu[val], MMCONF_MAXNAMELEN);
    work[MMCONF_MAXNAMELEN] = 0;

    if(_mmcfg_request_string(conf, var, work))
    {   int   i=0;

        if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(work);
        while(enu[i])
        {   if(!strcmp(work, enu[i]))
            {   val = i;  break;  }
            i++;
        }
    }
    return val;
}


// =====================================================================================
    static BOOL AllocWork2(MM_CONFIG *conf, uint count)
// =====================================================================================
// Returns TRUE on success, FALSE on failure (allocation failed, not enough memory)
{
    uint   i;
    
    if(!conf) return FALSE;

    if(!conf->work2) conf->work2 = (CHAR **)_mm_calloc(conf->allochandle, MMCONF_REQARRAY_MAX, sizeof(CHAR *));

    for(i=0; i<count; i++)
        if(!conf->work2[i]) conf->work2[i] = (CHAR *)_mm_calloc(conf->allochandle, MMCONF_MAXNAMELEN,sizeof(CHAR));

    return TRUE;
}


// =====================================================================================
    int _mmcfg_reqarray_boolean(MM_CONFIG *conf, const CHAR *var, uint nent, BOOL *val)
// =====================================================================================
// Loads an array of boolean values assigned to the given 'var' variable.  Each value
// is delimited by a semi-colon.  For example:  "PancakeMan = yes, no, enabled, disabled"
// Returns the number of values actually found and loaded (never greater than 'nent')
{
    uint    cnt;

    if(!conf || !var || !conf->subsec) return 0;
    if(nent > MMCONF_REQARRAY_MAX) nent = MMCONF_REQARRAY_MAX;

    AllocWork2(conf, nent);

    for(cnt=0; cnt<nent; cnt++)
        sprintf(conf->work2[cnt],"%s",val ? val_yes : val_no);

    if(cnt = _mmcfg_reqarray_string(conf, var, nent, conf->work2))
    {   uint   t;

        for(t=0; t<cnt; t++)
        {   int   i=0;

            if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(conf->work2[t]);

            if(strcmp(conf->work2[t],val_enabled)==0 || strcmp(conf->work2[t],val_true)==0 || strcmp(conf->work2[t],val_yes)==0 || strcmp(conf->work2[t],val_on)==0)
                val[t] = 1;
            else if(strcmp(conf->work2[t],val_disabled)==0 || strcmp(conf->work2[t],val_false)==0 || strcmp(conf->work2[t],val_off)==0 || strcmp(conf->work2[t],val_no)==0)
                val[t] = 0;
        }
    }
    return cnt;
}


// =====================================================================================
    int _mmcfg_reqarray_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, uint nent, int *val)
// =====================================================================================
// Loads an array of enumeration values assigned to the given 'var' variable.  Each value
// is delimited by a semi-colon.  For example:  "PancakeMan = You; Me; Everybody"
// Returns the number of values actually found and loaded (never greater than 'nent')
{
    uint    cnt;

    if(!conf || !var || !conf->subsec) return 0;
    if(nent > MMCONF_REQARRAY_MAX) nent = MMCONF_REQARRAY_MAX;

    AllocWork2(conf, nent);

    for(cnt=0; cnt<nent; cnt++)
        sprintf(conf->work2[cnt],"%s",enu[val[cnt]]);

    if(cnt = _mmcfg_reqarray_string(conf, var, nent, conf->work2))
    {   uint   t;

        for(t=0; t<cnt; t++)
        {   int   i=0;

            if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(conf->work2[t]);

            while(enu[i])
            {   if(!strcmp(conf->work2[t], enu[i]))
                {   val[t] = i;  break;  }
                i++;
            }
            val[t] = i;
        }
    }
    return cnt;
}


// =====================================================================================
    static void makesubname(CHAR *work, const CHAR *var)
// =====================================================================================
{
    uint   llen;
    strncpy(&work[1], var, MMCONF_MAXNAMELEN);
    llen = strlen(var);
    if(llen > MMCONF_MAXNAMELEN) llen = MMCONF_MAXNAMELEN;
    work[0]      = '[';
    work[llen+1] = ']';
    work[llen+2] = 0;
}


// =====================================================================================
    void _mmcfg_insert_subsection(MM_CONFIG *conf, const CHAR *var)
// =====================================================================================
// Inserts a new subsection into the configruation file.
// Insertion occurs either at the specified line, or if -1 is given, at the end of the
// file.
{
    CHAR    work[MMCONF_MAXNAMELEN+4];

    _mmcfg_madechange(conf);

    AllocLine(conf, 3);

    //conf->line[line] = strdup("# The following subsection was inserted during game initialization");
    //sprintf(work,"[%s]",var);

    makesubname(work, var);

    conf->line[conf->length]   = NULL;
    conf->line[conf->length+1] = _mm_strdup(conf->allochandle, work);

    // Add our subsection to the subsection list!

    AllocSubsection(conf, var, conf->length+1);

    conf->length += 2;
}


// =====================================================================================
    void _mmcfg_insert(MM_CONFIG *conf, int line, const CHAR *var, const CHAR *val)
// =====================================================================================
// Inserts a new variable / assignment entry into the current subsection.
// Insertion occurs either at the specified line, or if -1 is given, at the top of the
// subsection.
{
    if(line == -1)
        line = conf->subsec[conf->cursubsec].line + conf->subsec[conf->cursubsec].insertpos++;

    AllocLine(conf, 2);
    if(line < (int)conf->length)
        memcpy(&conf->line[line+1],&conf->line[line],(conf->length - line) * sizeof(UBYTE *));

    conf->line[line] = NULL;
    _mmcfg_reconstruct(conf, line, var, val);
    conf->length += 1;
}


// =====================================================================================
    void _mmcfg_insert_array(MM_CONFIG *conf, int line, const CHAR *var, uint cnt, const CHAR **val)
// =====================================================================================
// Inserts a new variable / assignment entry with an array assignment into the current
// subsection. Insertion occurs either at the specified line, or if -1 is given, at the
// top of the subsection.
{
    if(line == -1)
        line = conf->subsec[conf->cursubsec].line + conf->subsec[conf->cursubsec].insertpos++;

    AllocLine(conf, 2);
    if(line < (int)conf->length)
        memcpy(&conf->line[line+1],&conf->line[line],(conf->length - line) * sizeof(UBYTE *));

    //sprintf(conf->work, "%45s %s", 
    //conf->line[line] = strdup("# The following variable was inserted during game initialization");

    conf->line[line] = NULL;
    _mmcfg_reconstruct_array(conf, line, var, cnt, val);
    conf->length += 1;
}


// =====================================================================================
    void _mmcfg_reconstruct(MM_CONFIG *conf, int line, const CHAR *var, const CHAR *val)
// =====================================================================================
// Fixes errors in a configuration file and markes the file for saving when before it is
// closed - creating a backup file if first modification for the current session is being
// made.
//
// Input:
//   line  -  Line to find the variable (-1 for unknown)
//   var   -  variable to insert
//   val   -  value to assign to variable
{
    CHAR   erg[384];
    
    _mmcfg_madechange(conf);

    // If line is -1, then look for existence of the variable.

    if(line == -1)
    {   if((line = _mmcfg_findvar(conf,var)) == -1)
        {  _mmcfg_insert(conf,-1,var,val);
           return;
        }
    }

    _mm_free(conf->allochandle, conf->line[line]);

    // This is a bit of fun code.  I have a configurable formatter, so I had to
    // do this in order to get it set properly without sprintf thinking I was
    // trying to do something else.. :)

    if(conf->buflen > 48) conf->buflen = 48;
    
    sprintf(erg, " -%ds = %s", conf->buflen, val);
    erg[0] = '%';
    sprintf(conf->work, erg,var);
    conf->line[line] = _mm_strdup(conf->allochandle, conf->work);
}


// =====================================================================================
    void _mmcfg_reconstruct_array(MM_CONFIG *conf, int line, const CHAR *var, uint cnt, const CHAR **val)
// =====================================================================================
// Fixes errors in a configuration file and markes the file for saving when before it is
// closed - creating a backup file if first modification for the current session is being
// made.
//
// Input:
//   line  -  Line to find the variable (-1 for unknown)
//   var   -  variable to insert
//   val   -  value to assign to variable
{
    CHAR   erg[384];
    uint   i;
    
    _mmcfg_madechange(conf);
    if(line == -1)
    {   if((line = _mmcfg_findvar(conf,var)) == -1)
        {  _mmcfg_insert_array(conf,-1,var,cnt,val);
           return;
        }
    }

    _mm_free(conf->allochandle, conf->line[line]);

    // This is a bit of fun code.  I have a configurable formatter, so I had to
    // do this in order to get it set properly without sprintf thinking I was
    // trying to do something else.. :)

    if(conf->buflen > 48) conf->buflen = 48;

    sprintf(erg, " -%ds = ", conf->buflen);
    erg[0] = '%';
    for(i=0; i<cnt; i++)
    {   strcat(erg,val[i]);
        if(i<cnt-1)
        {   uint  t  = strlen(erg);
            erg[t]   = MMCONF_ARRAY_DELIMITOR;
            erg[t+1] = 0;
        }
    }

    sprintf(conf->work, erg, var);
    conf->line[line] = _mm_strdup(conf->allochandle, conf->work);
}


// =====================================================================================
    void _mmcfg_reassign_string(MM_CONFIG *conf, const CHAR *var, const CHAR *val)
// =====================================================================================
// Reassigns a value to an already existing variable.  Maintains the user's formatting.
{
    int    lpos,cpos;
    CHAR  *line;

    _mmcfg_madechange(conf);
    if((lpos = _mmcfg_findvar(conf, var)) == -1)
    {   _mmcfg_insert(conf, -1,var,val);
        return;
    }
    line = conf->line[lpos];  cpos = 0;
    while((line[cpos] != '=') && (line[cpos])) cpos++;
    cpos++;   // skip the equals
    while(WhiteSpace(line[cpos])) cpos++;
    line[cpos] = 0;

    // TODO: Logic to maintain user's end-of-line commenting!
    // [...]

    strcpy(conf->work,line); strcat(conf->work,val);
    _mm_free(conf->allochandle, line);  conf->line[lpos] = _mm_strdup(conf->allochandle, conf->work);
}


// =====================================================================================
    void _mmcfg_reassign_integer(MM_CONFIG *conf, const CHAR *var, int val)
// =====================================================================================
{
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var || !conf->subsec) return;
    sprintf(work,"%d",val);
    _mmcfg_reassign_string(conf, var, work);
}


// =====================================================================================
    void _mmcfg_reassign_boolean(MM_CONFIG *conf, const CHAR *var, BOOL val)
// =====================================================================================
{
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var || !conf->subsec) return;
    sprintf(work,"%s",val ? val_true : val_false);
    _mmcfg_reassign_string(conf, var, work);
}


// =====================================================================================
    void _mmcfg_reassign_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, int val)
// =====================================================================================
{
    CHAR    work[MMCONF_MAXNAMELEN+1];

    if(!conf || !var || !conf->subsec) return;

    strncpy(work, enu[val], MMCONF_MAXNAMELEN);
    work[MMCONF_MAXNAMELEN] = 0;
    _mmcfg_reassign_string(conf, var, work);
}


// =====================================================================================
    void _mmcfg_resarray_string(MM_CONFIG *conf, const CHAR *var, uint nent, CHAR *val[])
// =====================================================================================
// Reassigns a value to an already existing variable.  Maintains the user's formatting
// as much as possible...
{
    uint   t;
    int    lpos,cpos;
    CHAR  *line;

    _mmcfg_madechange(conf);
    if((lpos = _mmcfg_findvar(conf, var)) == -1)
    {   _mmcfg_insert_array(conf, -1,var,nent,val);
        return;
    }
    line = conf->line[lpos];  cpos = 0;
    while((line[cpos] != '=') && (line[cpos])) cpos++;
    cpos++;   // skip the equals
    while(WhiteSpace(line[cpos])) cpos++;
    line[cpos] = 0;

    // TODO: Logic to maintain user's end-of-line commenting!
    // [...]

    strcpy(conf->work,line);
    for(t=0; t<nent; t++)
    {   uint   c;
        strcat(conf->work, val[t]);
        c = strlen(conf->work);
        
        if(t != nent-1)
        {   conf->work[c]   = MMCONF_ARRAY_DELIMITOR;
            conf->work[c+1] = 0;
        }
    }

    _mm_free(conf->allochandle, line);  conf->line[lpos] = _mm_strdup(conf->allochandle, conf->work);
}


// =====================================================================================
    void _mmcfg_resarray_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, uint nent, int val[])
// =====================================================================================
{
    uint   cnt;

    if(!conf || !var || !conf->subsec) return;
    if(nent > MMCONF_REQARRAY_MAX) nent = MMCONF_REQARRAY_MAX;

    AllocWork2(conf, nent);

    for(cnt=0; cnt<nent; cnt++)
        sprintf(conf->work2[cnt],"%s",enu[val[cnt]]);

    _mmcfg_resarray_string(conf, var, nent, conf->work2);

    return;
}


//static CHAR _mmcfg_backerr[] = "mmconfig > Error creating backup file!\n";

// =====================================================================================
    void _mmcfg_madechange(MM_CONFIG *conf)
// =====================================================================================
// Before any change to the configuration file has been made, this routine is called.  
// This is the routine that makes sure a backup is made and that a new replacement
// configuration file is made.
{
    if(!conf->changed)
    {   
        /*CHAR   backini[255];
        FILE *fp;  

        sprintf(backini,"%s.bak", conf->name);
        _mmlog("mmconfig > Changes made. Generating backup copy of configuration file.\n");
        if((fp = fopen(backini,"wb")) == NULL)
            _mmlog(_mmcfg_backerr);
        else
        {   if(!_mmcfg_save(conf))
               _mmlog(_mmcfg_backerr);
            fclose(fp);
        }*/

        conf->changed = 1;
    }
}


// =====================================================================================
    BOOL _mmcfg_save(MM_CONFIG *conf)
// =====================================================================================
// saves the configuration file (stored in conf->line) to the specified file.
{
    uint      i;
    MMSTREAM *fp;

    fp = _mm_fopen(conf->fname,"wb");

    for(i=0; i<conf->length; i++)
        _mm_fputs(fp, conf->line[i]);

    _mm_fclose(fp);

    return 1;
}


