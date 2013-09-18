/*
 * hexloader
 * Copyright (c) 2007-2009 IOhannes m zmölnig @ IEM
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," that comes with Pd.  
 */

/*
 * this code adds an external "loader" to Miller S. Puckette's "pure data",
 * which allows the loading of libraries/externals with special characters
 * in the classname
 *
 * the infrastructure of this file is based on hcsteiner's "libdir" loader
 */


#ifdef __WIN32__
# define MSW
#endif

#include "m_pd.h"

#if (PD_MINOR_VERSION >= 40)

#if (PD_MINOR_VERSION < 42)
// with 0.42 the loader_t made it into s_stuff.h
# define MISSING_LOADER_T 1
#endif

#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef DL_OPEN
# include <dlfcn.h>
#endif
#ifdef UNISTD
# include <stdlib.h>
# include <unistd.h>
#endif
#ifdef _WIN32
# include <io.h>
# include <windows.h>
#endif
#ifdef __APPLE__
# include <mach-o/dyld.h> 
#endif


/* for now hexloading abstractions does not work very well, 
 * as it chokes when the hexloaded abstractions has nested abstractions
 * 
 * if you really want to enable hexloading patches, 
 * do so via the Makefile by defining HEXLOADER_PATCHES
 *
 */
//#define HEXLOADER_PATCHES

#ifdef HEXLOADER_PATCHES
void canvas_popabstraction(t_canvas *x);
static void*hexloader_fakenew(t_symbol *s, int argc, t_atom *argv);
t_pd pd_objectmaker;    /* factory for creating "object" boxes */
#endif

typedef struct _filepath
{
  t_symbol*filename;
  t_symbol*pathname;
} t_filepath;



typedef void (*t_hexloader_setup)(void);

#ifdef MISSING_LOADER_T
/* definitions taken from s_loader.c, since they weren't in header orignally */
typedef int (*loader_t)(t_canvas *canvas, char *classname);
void sys_register_loader(loader_t loader);
void class_set_extern_dir(t_symbol *s);
#endif

/* ==================================================== */

typedef struct _hexloader
{
  t_object x_obj;
} t_hexloader;
static t_class *hexloader_class;

static char *version = "1.6";


static char*hex_dllextent[] = {
#ifdef __FreeBSD__
  ".b_i386", ".pd_freebsd",
#endif
#ifdef __linux__
# ifdef __x86_64__
  ".l_ia64",
# else
  ".l_i386",
# endif
  ".pd_linux",
#endif /* linux */
#ifdef __APPLE__
# ifndef MACOSX3
  ".d_fat",
# else
  ".d_ppc",
# endif
  ".pd_darwin", 
#endif
#ifdef __WIN32__
  ".m_i386", ".dll",
#endif
  0}; /* zero-terminated list of extensions */



static  char *patch_extent[]={
  ".pd", 
  ".pat", 
  0};


/**
 * object-names containing non-alphanumerics like [||~]
 * can (sometimes) be not represented on filesystems (e.g.
 * "|" is a forbidden character) and more often they
 * cannot be used to construct a valid setup-function
 * ("||~_setup" or "||_tilde_setup" are really bad).
 * the way the "~" is handled, is non-generic and thus
 * sub-optimal.
 *
 * as a solution me and hcs proposed an encoding into
 * alphanumeric-values, using a hexadecimal representation
 * of all characters but [0-9A-Za-z_] (e.g. "+" is ascii
 * 43 and is thus represented by "0x2b" (hex-value all
 * lowercase and prepended with "0x")
 *
 * e.g. if we have a new class "mtx_||", pd first attempts
 * to find a file called "mtx_||.dll". if it succeeds, it
 * will try to call the "mtx_||_setup()" function.
 * if that fails we suggest to try and call a function
 * "setup_mtx_0x7c0x7c()" (the keyword setup is now at the
 * beginning of the function-name, in order to prevent the
 * names starting with numbers and in order to distinguish
 * between the normal setup-methods).
 * if no "mtx_||.dll" can be found, pd should then search
 * for a file "mtx_0x7c0x7c.dll" (guaranteed to be
 * representable on any filesystem); search this file for
 * the 2 setup-functions.
 * if all fails try to find "mtx_||.pd" and then
 * "mtx_0x7c0x7c.pd"
 */

/*
 * ideally this loader could somehow call the object-instantiator recursively
 * but with changed classnames;
 * this would allow us to use the hexloader stuff for all kinds of other loaders
 * including abstractions
 */


/* -------------------- utilities --------------------- */

/* --- symlist_t: a linked list of symbols --- */

/* linked list of loaders */
typedef struct symlist_ {
    t_symbol* sym;
    struct symlist_ *next;
} symlist_t;
static symlist_t*g_abstractionclasses=NULL;

static symlist_t*symlist_find(symlist_t*syms, t_symbol*sym) {
  symlist_t*dummy=syms;
  for(; dummy; dummy=dummy->next) {
    //    post("checking '%s' vs '%s'", sym, dummy->sym);
    if (sym==dummy->sym) {
      // we already have this entry!
      //      post("found sym %s=%s", sym, dummy->sym);
      return syms;
    }
  }
  return NULL;
}


static symlist_t*symlist_add(symlist_t*syms, t_symbol*sym) {
  symlist_t*dummy=syms;
  symlist_t*last=0;

  //  symlist_print("adder contained:", syms);

  if(NULL==sym)return syms;

  if(!dummy) {
    dummy=(symlist_t*)getbytes(sizeof(symlist_t));
    dummy->next=0;
    dummy->sym=sym;
    return dummy;
  }

  for(; dummy; dummy=dummy->next) {
    //    post("checking '%s' vs '%s'", sym, dummy->sym);
    if (sym==dummy->sym) {
      // we already have this entry!
      //      post("found sym %s=%s", sym, dummy->sym);
      return syms;
    }
    last=dummy;
  }
  dummy=last;

  dummy->next=(symlist_t*)getbytes(sizeof(symlist_t));
  dummy=dummy->next;
  dummy->next=0;
  dummy->sym=sym;

  //  symlist_print("adder contains::", syms);

  return syms;
}

/* --- namelist_t: a linked list of names --- */

/* linked list of loaders */
typedef struct namelist_ {
    char* name;
    struct namelist_ *next;
} namelist_t;

static void namelist_print(char*prefix, namelist_t*names) {
    for(; names; names=names->next) {
      if(prefix)startpost("%s:: ", prefix);
      post("%s",names->name);
    }
}

static namelist_t*namelist_add(namelist_t*names, char*name) {
  namelist_t*dummy=names;
  namelist_t*last=0;

  //  namelist_print("adder contained:", names);

  if(name==0)return names;

  if(!dummy) {
    dummy=(namelist_t*)getbytes(sizeof(namelist_t));
    dummy->next=0;
    dummy->name=name;
    return dummy;
  }

  for(; dummy; dummy=dummy->next) {
    //    post("checking '%s' vs '%s'", name, dummy->name);
    if (!strncmp(name, dummy->name, MAXPDSTRING)) {
      // we already have this entry!
      //      post("found name %s=%s", name, dummy->name);
      return names;
    }
    last=dummy;
  }
  dummy=last;

  dummy->next=(namelist_t*)getbytes(sizeof(namelist_t));
  dummy=dummy->next;
  dummy->next=0;
  dummy->name=name;

  //  namelist_print("adder contains::", names);

  return names;
}

static namelist_t*namelist_addlist(namelist_t*names, namelist_t*nl) {
  namelist_t*dummy=0;
  if(nl==0)return names;
  if(!names)return nl;

  /* try to add each entry in nl */
  for(dummy=nl; dummy->next; dummy=dummy->next) {
    names=namelist_add(names, dummy->name);
  }

  return names;
}

static void namelist_clear(namelist_t*names) {
  namelist_t*dummy=0;

  while(names) {
    dummy=names->next;
    names->next=0;
    names->name=0; /* we dont care since the names are allocated in the symboltable anyhow */
    freebytes(names, sizeof(namelist_t));
    names=dummy;
  }
}

/* --- filelist_t: a linked list of filenames, each associated with a list of setupfunction names --- */

typedef struct filelist_ {
  char *name;
  namelist_t*setupfun;
  struct filelist_ *next;
} filelist_t;

static void filelist_print(char*prefix, filelist_t*files) {
  for(; files; files=files->next) {
    if(prefix)startpost("%s: ", prefix);
    post("%s",files->name);
    namelist_print("\t", files->setupfun);
  }
}

static filelist_t*filelist_add(filelist_t*files, char*name, namelist_t*setupfun) {
  filelist_t *last=0, *dummy=files;
  if(name==0)return files;

  if(!dummy) {
    dummy=(filelist_t*)getbytes(sizeof(filelist_t));
    dummy->next=0;
    dummy->name=name;
    dummy->setupfun=namelist_addlist(0, setupfun);
    return dummy;
  }

  for(; dummy; dummy=dummy->next) {
    if (!strncmp(name, dummy->name, MAXPDSTRING)) {
      // we already have this entry!
      /* add additional setup-functions to this name */
      dummy->setupfun=namelist_addlist(dummy->setupfun, setupfun);
      return files;
    }
    last=dummy;
  }
  dummy=last;

  /* this is a new entry, add it to the list */

  dummy->next=(filelist_t*)getbytes(sizeof(filelist_t));
  dummy=dummy->next;
  dummy->next=0;
  dummy->name=name;
  dummy->setupfun=namelist_addlist(0, setupfun);

  return files;
}

static filelist_t*filelist_addlist(filelist_t*files, filelist_t*nl) {
  filelist_t*dummy=0;
  if(nl==0)return files;
  if(!files)return nl;

  /* try to add each entry in nl */
  for(dummy=nl; dummy->next; dummy=dummy->next) {
    files=filelist_add(files, dummy->name, dummy->setupfun);
  }

  return files;
}

static void filelist_clear(filelist_t*files) {
  filelist_t*dummy=0;

  while(files) {
    dummy=files->next;
    namelist_clear(files->setupfun);
    files->setupfun=0;
    files->name=0; /* we dont care since the files are allocated in the symboltable anyhow */
    files->next=0;
    freebytes(files, sizeof(filelist_t));
    files=dummy;
  }
}



/* ---------------- normalize names ------------------- */


/**
 * replace everything but [a-zA-Z0-9_] by "0x%x" 
 * @return the normalized version of org
 */
static char*hexloader_normalize(char*org, int skipslash)
{
  char*orgname=org;
  char altname[MAXPDSTRING];
  t_symbol*s=0;

  int count=0;
  int i=0;

  for(i=0; i<MAXPDSTRING; i++)
    altname[i]=0;

  i=0;
  while(*orgname && i<MAXPDSTRING)
    {
      char c=*orgname;
      if((c>='0' && c<='9')|| /* [0-9] */
         (c>='A' && c<='Z')|| /* [A-Z] */
         (c>='a' && c<='z')||/* [a-z] */
         (c=='_') ||        /* [_] */
         (skipslash && c=='/')
         )
        {
          altname[i]=c;
          i++;
        }
      else /* a "bad" character */
        {
          sprintf(altname+i, "0x%02x", c);
          i+=4;
          count++;
        }
      orgname++;
    }

  s=gensym(altname);
  //  post("normalize=%s", s->s_name);
  return s->s_name;
}

/**
 * replace only / \ : * ? " < > | by 0x%x since these are forbidden on some filesystems
 * @return the normalized version of org
 */
static char*hexloader_fsnormalize(char*org)
{
  char*orgname=org;
  char altname[MAXPDSTRING];
  t_symbol*s=0;

  char forbiddenchars[]={ 
    //    '/', '\\', ':', '*', '?', '"', '<', '>', '|',
    '\\', ':', '*', '?', '"', '<', '>', '|',
    0};

  int count=0;
  int i=0;

  for(i=0; i<MAXPDSTRING; i++)
    altname[i]=0;

  i=0;
  while(*orgname && i<MAXPDSTRING)
    {
      char c=*orgname;
      char*forbidden=forbiddenchars;
      int found=0;

      while(*forbidden) {
        if(c==*forbidden) {
          sprintf(altname+i, "0x%02x", c);
          i+=4;
          count++;
          found=1;
          break;
        }
        forbidden++;
      }
      if(!found)
        {
          altname[i]=c;
          i++;
        }
      orgname++;      
    }

  s=gensym(altname);
  //  post("fsnormalize=%s", s->s_name);
  return s->s_name;
}


static filelist_t*hexloader_deslashify(filelist_t*names, char*prefix, char*postfix) {
  filelist_t*result=names;
  int i=0;
  char*cp=0;
  char buf[MAXPDSTRING];
  t_symbol*s;

  snprintf(buf, MAXPDSTRING, "%s%s", prefix, postfix);
  s=gensym(buf);

  result=filelist_add(result, s->s_name, 0);

  for(cp=postfix; *cp; cp++) {
    if(*cp=='/') {
      char *postfix2=postfix+i+1;
      char buf2[MAXPDSTRING];

      snprintf(buf2, MAXPDSTRING, "%s", prefix);
      strncat(buf2, postfix, i);
      strcat(buf2, "/");
      result=hexloader_deslashify(result, buf2, postfix2);

      snprintf(buf2, MAXPDSTRING, "%s", prefix);
      strncat(buf2, postfix, i);
      strcat(buf2, "0x2f");
      result=hexloader_deslashify(result, buf2, postfix2);

    }
    i++;
  }

  return result;
}


static namelist_t*hexloader_fsnormalize_list(namelist_t*names, char*org) {
  char*simple=hexloader_fsnormalize(org);
  names=namelist_add(names, org);
  names=namelist_add(names, simple);

  return names;
}



/* ---------------- core code ------------------- */

static namelist_t*hexloader_getaltsetups(namelist_t*names, char*name) {
  char sbuf[MAXPDSTRING];
  t_symbol*s;
  snprintf(sbuf, MAXPDSTRING, "%s_setup", name);
  s=gensym(sbuf);
  names=namelist_add(names, s->s_name);
  snprintf(sbuf, MAXPDSTRING, "setup_%s", name);
  s=gensym(sbuf);
  names=namelist_add(names, s->s_name);
  return names;
}
static namelist_t*hexloader_getaltsetup(namelist_t*names, char*name) {
  namelist_t*result=names;
  char*strippedname=name;
  char*cp=name;

  while(*cp++) {
    if(*cp=='/'){
      strippedname=cp+1;
    }
  }
 
  result=hexloader_getaltsetups(result, strippedname);
  result=hexloader_getaltsetups(result, hexloader_normalize(strippedname, 0));
  //  post("added to %s\n", name);
  return result;
}

static filelist_t*hexloader_fillaltsetupfuns(filelist_t*files) {
  filelist_t*f;
  for(f=files; f; f=f->next) {
    f->setupfun=namelist_addlist(f->setupfun, hexloader_getaltsetup(f->setupfun, f->name));
  }
  return files;
}

/**
 * @return a 0-terminated array of all filenames we consider to be alternative names and associated setup-functions
 */

static filelist_t*hexloader_getalternatives(char*org) {
  filelist_t*files=0;

  files=filelist_add(files, org, 0);
  files=filelist_add(files, hexloader_fsnormalize(org), 0);
  files=filelist_add(files, hexloader_normalize(org, 1),0);
  files=filelist_add(files, hexloader_normalize(org, 0),0);
  files=hexloader_deslashify(files, "", org);

  hexloader_fillaltsetupfuns(files);
  //  filelist_print("ALTER: ", files);

  return files;
}

static int hexloader_doload(char*filename, char*setupfun) {
    t_hexloader_setup makeout=0;

#ifdef DL_OPEN
    void *dlobj = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if (!dlobj)
    {
      verbose(2, "%s: %s", filename, dlerror());
      class_set_extern_dir(&s_);
      return (0);
    }
    makeout = (t_hexloader_setup)dlsym(dlobj,  setupfun);
#elif defined __WIN32__
    HINSTANCE ntdll;
    sys_bashfilename(filename, filename);
    ntdll = LoadLibrary(filename);
    if (!ntdll)
    {
      verbose(2, "%s: couldn't load", filename);
      class_set_extern_dir(&s_);
      return (0);
    }
    makeout = (t_hexloader_setup)GetProcAddress(ntdll, setupfun);  
#else
    error("alas! somebody (you?) has compiled [hexloader] without support for loading externals...how should i load?");
#endif

    if (!makeout)
    {
      verbose(2, "hexload object: Symbol \"%s\" not found", setupfun);
      class_set_extern_dir(&s_);
      return 0;
    }
    (*makeout)();
    return (1);
}

/**
 * open a file (given via pathname+filename) as dll and call various
 * setupfunctions (as can be calculated from the altnames array
 * @param pathname the path of the file
 * @param filename the name (without path) of the file
 * @param altnames a zero-terminated array of possible non-generic parts of the setup-function
 * @return 1 on success, 0 otherwise
 */
static int hexloader_loadfile(char*pathname, char*filename, namelist_t*altnames) 
{
  char setupfun[MAXPDSTRING];
  char fullfile[MAXPDSTRING];
  namelist_t*altname=altnames;

  sprintf(fullfile, "%s/%s", pathname, filename);

  while(altname) {
    verbose(0, "hexloader trying %s (%s)", fullfile, altname->name);
    if(hexloader_doload(fullfile, altname->name))
      return 1;

    altname=altname->next;
  }

  return 0;
}



/**
 * try to open a file (given via pathname+filename) as a patcher
 * TODO: make this work....
 * @param pathname the path of the file
 * @param filename the name (without path) of the file
 * @param altclassname the alternative classname we currently use...
 * @return 1 on success, 0 otherwise
 */


/* this only gets called if we have already found an abstraction-file */
static t_filepath*hexloader_loadpatch(char*pathname, char*filename, char*altclassname, char*realclassname)
{
  char fullfile[MAXPDSTRING];
  t_symbol*realname=gensym(realclassname);
  t_filepath*result=NULL;
  sprintf(fullfile, "%s/%s", pathname, filename);

#ifdef HEXLOADER_PATCHES
  if(symlist_find(g_abstractionclasses, realname)) {
    // already in the list
  } else {
    class_addcreator((t_newmethod)hexloader_fakenew, realname, A_GIMME, 0);
    g_abstractionclasses=symlist_add(g_abstractionclasses, realname);
  }

  result=getbytes(sizeof(t_filepath));
  result->filename=gensym(filename);
  result->pathname=gensym(pathname);

  
#else
  error("BUG: hexloader not loading abstraction: %s (not yet implemented)", fullfile);
#endif /* HEXLOADER_PATCHES */
  return result;
}
/**
 * the actual loader:
 *  example-nomenclature:
 *   "class": the original class-name (e.g. containing weird characters)
 *   "CLASS": the normalized class-name (with all weirdness replaced by hex-representations
 *   "ext"  : the external-extension (e.g. "dll" or "pd_linux" or...)
 *  example:
 *   trying to create an object [class] (and pd fails for whatever reasons, and thus callsus)
 *   - search for a file "class.ext" in our search-path
 *    + if found
 *     try to call "class_setup()" function 
 *     if fails, try to call "setup_CLASS()" function
 *    (if fails, try to call "CLASS_setup()" function)
 *    - "class.ext" file not found
 *   - search for a file "CLASS.ext" in our search-path
 *    + if found
 *     try to call "class_setup()" function 
 *     if fails, try to call "setup_CLASS()" function
 *    (if fails, try to call "CLASS_setup()" function)
 *   - if everything fails, return...
 *
 * @param canvas the context of the object to be created
 * @param classname the name of the object (external, library) to be created
 * @return 1 on success, 0 on failure
 */

static int hexloader_trylibraries(t_canvas*x, filelist_t*altnames0) {
  int fd = -1;
  char dirbuf[MAXPDSTRING];
  char*nameptr;
  filelist_t*altnames=altnames0;
  
  /* try binaries */
  while(altnames) {
    char*altname=altnames->name;
    int dll_index=0;
    char*dllextent=hex_dllextent[dll_index];
    while(dllextent!=0) {
      if(NULL!=x)
        fd=canvas_open(x, altname, dllextent, dirbuf, &nameptr, MAXPDSTRING, 0);
      else
        fd = open_via_path(".", altname, dllextent, dirbuf, &nameptr, MAXPDSTRING, 0);

      if (fd >= 0) {
        close (fd);

        if(hexloader_loadfile(dirbuf, nameptr, altnames->setupfun)) {
          return 1;
        }
      }
      dll_index++;
      dllextent=hex_dllextent[dll_index];
    }

    altnames=altnames->next;
  }

  return 0;
}
static t_filepath*hexloader_trypatches(t_canvas*x, filelist_t*altnames0, char*classname) {
  int fd = -1;
  char dirbuf[MAXPDSTRING];
  char*nameptr;
  filelist_t*altnames=altnames0;

  /* try patches */
  altnames=altnames0;
  while(altnames) {
    char*altname=altnames->name;
    int extindex=0;
    char*extent=patch_extent[extindex];
    if(strcmp(altname, classname)) { /* we only try if it is worth trying... */
      while(extent!=0) {
        if(NULL!=x)
          fd=canvas_open(x, altname, extent, dirbuf, &nameptr, MAXPDSTRING, 0);
        else
          fd = open_via_path(".", altname, extent, dirbuf, &nameptr, MAXPDSTRING, 0);
        if (fd >= 0) {
          close (fd);
          t_filepath*fp=hexloader_loadpatch(dirbuf, nameptr, altname, classname);
          if(fp) {
            return fp;
          }
        }
        extindex++;
        extent=patch_extent[extindex];
      }
    }
    altnames=altnames->next;
  }

  return 0;
}


/**
 * this is the actual loader:
 * we first try to load an external with alternative (hexified) names
 * if this fails, we fall back to load a patch with these names
 */
static int hexloader_doloader(t_canvas *canvas, filelist_t*altnames0, char*classname)
{
  t_filepath*fp=0;
  if(hexloader_trylibraries(canvas, altnames0))
    return 1;

  fp=hexloader_trypatches(canvas, altnames0, classname);
  if(fp) {
    freebytes(fp, sizeof(t_filepath));
    return 1;
  }

  return 0;
}

/**
 * recursive use of our powers:
 * try to load the object with alternative (hexified) names using other available loaders
 * when it comes to us, we just return 0...
 */
static int hexloader_callothers(t_canvas *canvas, filelist_t*altnames) {
  int result=0;
  while(altnames) {
    char*altname=altnames->name;
    verbose(2, "calling sys_load with '%s'", altname);
    result=sys_load_lib(canvas, altname);
    if(result==1) {
      return 1;
    }
    altnames=altnames->next;
  }
  return 0;
}


/**
 * the loader
 *
 * @param canvas the context of the object to be created
 * @param classname the name of the object (external, library) to be created
 * @return 1 on success, 0 on failure
 */
static int hexloader_loader(t_canvas *canvas, char *classname)
{
  filelist_t*altnames=0;
  int result=0;

  static int already_loading=0;
  if(already_loading)return 0;
  already_loading=1;

  /* get alternatives */
  altnames=hexloader_getalternatives(classname);

  /* try other loaders with our power */
  if(result==0){
    result=hexloader_callothers(canvas, altnames);
  }

  /* do the loading ourselves */
  if(result==0) {
    result=hexloader_doloader(canvas, altnames, classname);
  }

  /* clean up */
  filelist_clear(altnames); 

  already_loading=0;
  return result;
}

#ifdef HEXLOADER_PATCHES
static void*hexloader_fakenew(t_symbol*s, int argc, t_atom*argv) {
  t_pd*current = s__X.s_thing;
  t_filepath*fp=0;
  filelist_t*altnames=0;

  verbose(1, "hexloader: disguising as '%s'", s->s_name);

  if(!pd_objectmaker) {
    error("BUG: no pd_objectmaker found");
    return 0;
  }

  /* get alternatives */
  altnames=hexloader_getalternatives(s->s_name);
  /* do the loading */
  fp=hexloader_trypatches(NULL, altnames, s->s_name);
  /* clean up */
  filelist_clear(altnames); 

  if(fp) {
    canvas_setargs(argc, argv);
    /* this fails when the object is loaded the first time
     * since m_class.c:new_anything() has set the "tryingalready" flag
     * the next time the object is created, it will work, since pd_objectmaker already knows about us and thus new_anything() never get's called
     */
    /* ideas how to fix this:
     * - a simple hack is to register the abstractions to be hexloaded beforehand (but we have to know them before they are requested!)
     * - override the anything-method of pd_objectmaker "new_anything()" and call it ourselves (this makes the entire sys_loader hook absurd)
     *
     * claude's "abstraction caching" (http://lists.puredata.info/pipermail/pd-dev/2008-10/012334.html) works because he is injecting the code right
     * before sys_load_lib() and the "tryingalready" guards
     */
    binbuf_evalfile(fp->filename, fp->pathname);
    freebytes(fp, sizeof(t_filepath));
    if (s__X.s_thing != current)
      canvas_popabstraction((t_canvas *)(s__X.s_thing));
    canvas_setargs(0, 0);
    return pd_newest();
  } else 
    return 0;
}
#endif /* HEXLOADER_PATCHES */

#endif /* PD_MINOR_VERSION>=40 */


static void*hexloader_new(t_symbol *s, int argc, t_atom *argv)
{
  t_hexloader*x = (t_hexloader*)pd_new(hexloader_class);
  return (x);
}

void hexloader_setup(void)
{
  /* relies on t.grill's loader functionality, fully added in 0.40 */
  post("hexloader %s",version);  
  post("\twritten by IOhannes m zmölnig, IEM <zmoelnig@iem.at>");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);

#if (PD_MINOR_VERSION >= 40)
  sys_register_loader(hexloader_loader);
#else
  error("to function, this needs to be compiled against Pd 0.40 or higher,\n");
  error("\tor a version that has sys_register_loader()");
#endif

  hexloader_class = class_new(gensym("hexloader"), (t_newmethod)hexloader_new, 0, sizeof(t_hexloader), CLASS_NOINLET, A_GIMME, 0);
}
