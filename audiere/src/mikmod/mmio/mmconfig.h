/*
  Configuration File Utilities
   - Load and save configuration files for game, applications,
     and most any other program.

  By Jake Stine of Divine Entertainment.  Sometime near the end of
  the second millenium.

              -- Prototype definitions and some such --
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "mmio.h"

#define MMCONF_MAXNAMELEN        128

#define MMCONF_CASE_INSENSITIVE    1
#define MMCONF_AUTOFIX             2

typedef struct MMCFG_SUBSEC
{   const CHAR  *name;         // name
    uint         line;         // line (of the tag itself)
    uint         insertpos;    // insert position (makes sure things get inserted in order
                               // and not in reverse-order).
} MMCFG_SUBSEC;


// =====================================================================================
    typedef struct MM_CONFIG
// =====================================================================================
{
    uint      flags;
    CHAR     *fname;           // filename to save to (load from)
    CHAR    **line,            // storage for each line of the configuration file
             *work,            // general workspace for variable checks, etc.
            **work2;           // workspace used by array-readers.

    uint      length;          // length of the config file, in lines
    int       cursubsec;       // current working subsection

    uint      buflen;          // insertion buffer length between var and '='
    BOOL      changed;         // changed-flag (indicates whether or not we save on exit)

    uint      numsubsec;
    MMCFG_SUBSEC *subsec;      // list of all subsections (name and line)

    //MMSTREAM *fp;

    uint      subsec_alloc;
    uint      length_alloc;

    MM_ALLOC *allochandle;

} MM_CONFIG;



// Shit used to read existing configuration file entries!

extern MM_CONFIG *_mmcfg_initfn(CHAR *fname);
extern void      _mmcfg_exit(MM_CONFIG *conf);
extern void      _mmcfg_enable_autofix(MM_CONFIG *conf, uint buflen);

extern BOOL      _mmcfg_set_subsection(MM_CONFIG *conf, const CHAR *var);
extern BOOL      _mmcfg_set_subsection_int(MM_CONFIG *conf, uint var, CHAR name[MMCONF_MAXNAMELEN]);
extern int       _mmcfg_findvar(MM_CONFIG *conf, const CHAR *var);

extern BOOL      _mmcfg_request_string(MM_CONFIG *conf, const CHAR *var, CHAR *val);
extern int       _mmcfg_request_integer(MM_CONFIG *conf, const CHAR *var, int val);
extern BOOL      _mmcfg_request_boolean(MM_CONFIG *conf, const CHAR *var, BOOL val);
extern int       _mmcfg_request_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, int val);

extern int       _mmcfg_reqarray_string(MM_CONFIG *conf, const CHAR *var, uint nent, CHAR **val);
extern int       _mmcfg_reqarray_integer(MM_CONFIG *conf, const CHAR *var, uint nent, int *val);
extern int       _mmcfg_reqarray_boolean(MM_CONFIG *conf, const CHAR *var, uint nent, BOOL *val);
extern int       _mmcfg_reqarray_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, uint nent, int *val);

extern CHAR     *_mmcfg_fixname(CHAR *dest, const CHAR *var);


// --> Shit we use to add stuff to configuration files!

extern void      _mmcfg_reassign_string(MM_CONFIG *conf, const CHAR *var, const CHAR *val);
extern void      _mmcfg_reassign_integer(MM_CONFIG *conf, const CHAR *var, int val);
extern void      _mmcfg_reassign_boolean(MM_CONFIG *conf, const CHAR *var, BOOL val);
extern void      _mmcfg_reassign_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, int val);

extern void      _mmcfg_resarray_string(MM_CONFIG *conf, const CHAR *var, uint nent, CHAR *val[]);
extern void      _mmcfg_resarray_enum(MM_CONFIG *conf, const CHAR *var, const CHAR **enu, uint nent, int val[]);

extern void      _mmcfg_insert_subsection(MM_CONFIG *conf, const CHAR *var);
extern void      _mmcfg_insert(MM_CONFIG *conf, int line, const CHAR *var, const CHAR *val);
extern void      _mmcfg_reconstruct(MM_CONFIG *conf, int line, const CHAR *var, const CHAR *val);
extern BOOL      _mmcfg_save(MM_CONFIG *conf);
extern void      _mmcfg_madechange(MM_CONFIG *conf);

extern void      _mmcfg_insert_array(MM_CONFIG *conf, int line, const CHAR *var, uint cnt, const CHAR **val);
extern void      _mmcfg_reconstruct_array(MM_CONFIG *conf, int line, const CHAR *var, uint cnt, const CHAR **val);

#endif

