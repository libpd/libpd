/* Copyright (c) 2008 Steve Sinclair <radarsat1@gmail.com>
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," that comes with Pd.  
 */

/*
 * this code adds an external "loader" to Miller S. Puckette's "pure data",
 * which allows the loading of libraries/externals from internet URLs
 *
 * the infrastructure of this file is based on hcsteiner's "libdir" loader
 */


#ifdef __WIN32__
# define MSW
#endif

#include "m_pd.h"

#if (PD_MINOR_VERSION >= 40)

#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "baseurl.h"
#include <curl/curl.h>

#include <stdint.h>
#include "md5.h"
#include "md5.c"

typedef void (*t_urlloader_setup)(void);

/* definitions taken from s_loader.c  */
typedef int (*loader_t)(t_canvas *canvas, char *classname);
void sys_register_loader(loader_t loader);
void class_set_extern_dir(t_symbol *s);

/* taken from m_class.c */
int pd_setloadingabstraction(t_symbol *sym);
void canvas_popabstraction(t_canvas *x);

/* ==================================================== */

#define CACHE_TIMEOUT_SECONDS 100

typedef struct _urlloader
{
  t_object x_obj;
} t_urlloader;
static t_class *urlloader_class;

static char *version = "0.0.1";

typedef struct _list
{
    struct _list *next;
} t_list;

typedef struct _url_object
{
    t_list list;
    t_symbol *name;
    char md5[16];
} t_url_object;

typedef struct _url
{
    t_list list;
    t_symbol *url;
    t_url_object *objects;
} t_url;
static t_url *baseurl_list = NULL;

char cachedir[MAXPDSTRING]="";
#define FILEPERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIRPERM (FILEPERM|S_IXUSR|S_IXGRP|S_IXOTH)
char *get_cache_dir(void)
{
    if (cachedir[0]==0) {
        strcpy(cachedir, getenv("HOME"));
        strcat(cachedir, "/.pd/cache");
    }

    return cachedir;
}

FILE* create_and_open(const char *filename, const char *mode)
{
    char str[MAXPDSTRING], *ss=str;
    const char *sf=filename;
    struct stat st;
    while (*sf) {
        do {
            *ss = *sf;
            ss++;
            sf++;
        } while (*sf && *sf != '/');
        *ss = 0;

        if (stat(str, &st)<0) {
            if (*sf) {
                if (mkdir(str, DIRPERM)<0) {
                    post("[urlloader] Error creating %s", str);
                    return 0;
                }
            }
            else
                return fopen(str, mode);
        }
        else if (!*sf)
            return fopen(str, mode);
    }

    return 0;
}

/** Return the full path to a filename in the cache */
void cache_path(char path[MAXPDSTRING], t_url *u, const char *filename)
{
    char *s;
    strncpy(path, get_cache_dir(), MAXPDSTRING);
#ifdef WIN32
    strncat(path, "\\", MAXPDSTRING);
#else
    strncat(path, "/", MAXPDSTRING);
#endif
    s = path + strlen(path);
    strncat(path, u->url->s_name, MAXPDSTRING);
    while (*s) {
        if (*s == '/') *s = '_';
        s++;
    }
#ifdef WIN32
    strncat(path, "\\", MAXPDSTRING);
#else
    strncat(path, "/", MAXPDSTRING);
#endif
    strncat(path, filename, MAXPDSTRING);
}

/** Open a file from the cache or download it first if needed. */
FILE* download_and_open_from_cache(t_url *u, const char *filename, const char *mode)
{
    char str_url[MAXPDSTRING];
    char cache_name[MAXPDSTRING];
    struct stat st;

    cache_path(cache_name, u, filename);

    if (stat(cache_name, &st)<0
        || ((time(NULL)-st.st_mtime) > CACHE_TIMEOUT_SECONDS))
    { 
        CURL *c = curl_easy_init();
        FILE *f=0;
        if (!c) {
            post("[urlloader] Error initializing curl.");
            return 0;
        }

        f = create_and_open(cache_name, "wb");
        if (!f) {
            post("[urlloader] Error opening %s for write.", cache_name);
            return 0;
        }

        /* Download the index file */
        snprintf(str_url, MAXPDSTRING, "%s/%s", u->url->s_name, filename);
        curl_easy_setopt(c, CURLOPT_URL, str_url);
        curl_easy_setopt(c, CURLOPT_WRITEDATA, f);
        if (curl_easy_perform(c)) {
            post("[urlloader] Error downloading %s", str_url);
        }
        curl_easy_cleanup(c);

        post("[urlloader] Downloaded %s to %s", str_url, cache_name);

        fclose(f);
    }

    if (mode)
        return fopen(cache_name, mode);
    else
        // indicate success without opening the file
        return (FILE*)1;
}

/** Append an item to a struct beginning with a t_list.
 *  If allowdup is false (0), second item must be a t_symbol*.
 *  If duplicate, returns the item without modifying it. */
void *list_append(void *list, void *item, int size, int allowdup)
{
    t_list** l = list;
    t_symbol *s1, *s2;
    if (!*l) {
        *l = getbytes(size);
    }
    else {
        while (*l) {
            if (!allowdup) {
                s1 = *(t_symbol**)(((char*)*l)+sizeof(t_list));
                s2 = *(t_symbol**)(((char*)item)+sizeof(t_list));
                if (s1==s2 || strcmp(s1->s_name, s2->s_name)==0)
                    return *l;
            }
            if (!(*l)->next)
                break;
            l = &(*l)->next;
        }
        (*l)->next = getbytes(size);
        l = &(*l)->next;
    }

    if (*l) {
        memcpy(*l, item, size);
        (*l)->next = 0;
    }

    return *l;
}

t_url *url_add(t_atom *url)
{
    t_url u;
    u.url = url->a_w.w_symbol;
    u.objects = 0;
    return list_append(&baseurl_list, &u, sizeof(t_url), 0);
}

void url_parse_index_line(t_url *u, char* line)
{
    char *s = line, *name, hex[3];
    int i=0;
    t_url_object o;
    while ((*s==' ' || *s=='\t') && *s) {s++;}   /* skip whitespace */
    if (!*s) return;
    while (!(*s==' ' || *s=='\t') && *s) {       /* remember MD5 sum */
        hex[0] = *s; s++;
        hex[1] = *s; s++;
        hex[2] = 0;
        o.md5[i++] = strtol(hex, 0, 16);
    }
    if (!*s) return;
    while ((*s==' ' || *s=='\t') && *s) {s++;}   /* skip whitespace */
    if (!*s) return;
    name = s;
    while (!(*s==' ' || *s=='\t') && *s) {s++;}  /* skip md5sum for now */
    *s = 0;

    o.name = gensym(name);
    list_append(&u->objects, &o, sizeof(t_url_object), 0);
}

/** Download index for a given URL. */
int url_update_index(t_url *u)
{
    char buffer[MAXPDSTRING], *line=buffer;
    int bufpos=0;

    // open and read the downloaded or cached file.
    FILE *f = download_and_open_from_cache(u, "pd.index", "rb");
    if (!f) {
        post("[urlloader] Error opening index for %s", u->url->s_name);
        return -1;
    }

    post("[urlloader] Index for %s opened.", u->url->s_name);
    while (!feof(f)) {
        int sz = fread(buffer+bufpos, 1, 256, f);
        int end = bufpos + sz;
        while (bufpos < end) {
            while (buffer[bufpos]!='\n' && buffer[bufpos]!='\r'
                   && bufpos < end)
                ++bufpos;
            if (bufpos < end || feof(f)) {
                buffer[bufpos] = 0;
                url_parse_index_line(u, line);
                ++bufpos;
                line = buffer+bufpos;
            }
        }
    }

    fclose(f);
    return 0;
}

/** Download any indexes that are missing. */
static int cache_update_index(void)
{
    t_url *u;
    int rc;

    u = baseurl_list;
    while (u) {
        rc = 0;
        if (u->objects == NULL)
            rc = url_update_index(u);
        if (rc) {
            post("[urlloader] Error updating index for %s", u->url->s_name);
            return rc;
        }
        u = (t_url*)u->list.next;
    }

    return 0;
}

/** Search all objects in the canvas and its owner for baseurl objects.
 *  Add the found object URLs to the global list of t_urls.
 */
static void canvas_find_baseurls(t_canvas *canvas)
{
    t_gobj *o = canvas->gl_list;
    while (o) {
        if (strcmp(class_getname(o->g_pd),"baseurl")==0)
            url_add(&((t_baseurl*)o)->url);
        o = o->g_next;
    }
    if (canvas->gl_owner)
        canvas_find_baseurls(canvas->gl_owner);
}

/** Find a given class name in the global t_url list. */
static t_url* url_find_class(char *classname, t_url_object **object)
{
    t_url *u = baseurl_list;
    t_url_object *o;
    while (u) {
        o = u->objects;
        while (o) {
            char *ext = strstr(o->name->s_name, ".pd");
            if (!ext) continue;
            if (strncmp(o->name->s_name, classname,
                        ext - o->name->s_name)==0)
                break;
            o = (t_url_object*)o->list.next;
        }
        if (o) break;
        u = (t_url*)u->list.next;
    }
    if (object) *object = o;
    return u;
}

/** Load an abstraction specified by an absolute path name. */
int canvas_load_from_absolute_path(t_canvas *canvas, t_symbol *classname,
                                   char *path, char *ext)
{
    char dirbuf[MAXPDSTRING], *nameptr;
    t_pd *current = s__X.s_thing;
    int fd=-1;

    fd = canvas_open(canvas, path, ext,
                     dirbuf, &nameptr, MAXPDSTRING, 0);

    if (fd < 0)
        return 0;
    
    close(fd);
    if (!pd_setloadingabstraction(classname)) {
        canvas_setargs(0, 0);  /* TODO argc, argv */
        binbuf_evalfile(gensym(nameptr), gensym(dirbuf));
        if (s__X.s_thing != current)
            canvas_popabstraction((t_canvas *)(s__X.s_thing));
        canvas_setargs(0, 0);
    }
    else {
        error("%s: can't load abstraction within itself\n", classname);
        return 0;
    }

    return 1;
}

/**
 * the loader
 *
 * @param canvas the context of the object to be created
 * @param classname the name of the object (external, library) to be created
 * @return 1 on success, 0 on failure
 */
static int urlloader_loader(t_canvas *canvas, char *classname)
{
  int result=0;
  t_url *u;
  t_url_object *o;
  char class_filename[MAXPDSTRING];
  char class_cachepath[MAXPDSTRING];
  char tmpstr[MAXPDSTRING];
  FILE *f;

  static int already_loading=0;
  if(already_loading)return 0;
  already_loading=1;

  /* loader */
  post("[urlloader] Asked to load %s for canvas %s", classname, canvas->gl_name->s_name);
  canvas_find_baseurls(canvas);
  cache_update_index();

  post("[urlloader] Known URLs:");
  u = baseurl_list;
  while (u) {
      post("[urlloader] %s", u->url->s_name);
      o = u->objects;
      while (o) {
          post("[urlloader] - %s", o->name->s_name);
          o = (t_url_object*)o->list.next;
      }
      u = (t_url*)u->list.next;
  }

  u = url_find_class(classname, &o);
  if (!(u && o)) {
      already_loading=0;
      return result;
  }

  post("[urlloader] Found: %s.pd at %s", classname, u->url->s_name);

  snprintf(class_filename, MAXPDSTRING, "%s.pd", classname);
  if (f=download_and_open_from_cache(u, class_filename, "rb"))
  {
      char resblock[16];
      int okay=1, i;
      if (md5_stream(f, resblock)) {
          post("[urlloader] Error generating MD5 sum for %s", class_filename);
          result = 0;
          already_loading = 0;
          fclose(f);
          return result;
      }
      fclose(f);

      for (i=0; i<16; i++)
          okay &= (resblock[i]==o->md5[i]);
      if (!okay) {
          post("[urlloader] MD5 error for %s", class_filename);
          result = 0;
          already_loading = 0;
          return result;
      }

      // download GPG signature
      snprintf(tmpstr, MAXPDSTRING, "%s.asc", class_filename);
      if (!download_and_open_from_cache(u, tmpstr, 0)) {
          post("[urlloader] Error downloading GPG signature for %s",
               class_filename);
          result = 0;
          already_loading = 0;
          return result;
      }

      // check GPG signature
      cache_path(class_cachepath, u, classname);
      snprintf(tmpstr, MAXPDSTRING, "gpg --verify %s.pd.asc %s.pd",
               class_cachepath, class_cachepath);
      if (system(tmpstr)) {
          post("[urlloader] Error verifying GPG signature for %s",
               class_filename);
          result = 0;
          already_loading = 0;
          return result;
      }
           

      // load pd patch
      canvas_load_from_absolute_path(canvas, gensym(classname),
                                     class_cachepath, ".pd");
      result = 1;
  }

  already_loading=0;
  return result;
}

#endif /* PD_MINOR_VERSION>=40 */

static void*urlloader_new(t_symbol *s, int argc, t_atom *argv)
{
  t_urlloader*x = (t_urlloader*)pd_new(urlloader_class);
  return (x);
}

void urlloader_setup(void)
{
  /* relies on t.grill's loader functionality, fully added in 0.40 */
  post("url loader %s",version);  
  post("\twritten by Steve Sinclair");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);

  curl_global_init(CURL_GLOBAL_ALL);

#if (PD_MINOR_VERSION >= 40)
  sys_register_loader(urlloader_loader);
#else
  error("to function, this needs to be compiled against Pd 0.40 or higher,\n");
  error("\tor a version that has sys_register_loader()");
#endif

  urlloader_class = class_new(gensym("urlloader"), (t_newmethod)urlloader_new, 0, sizeof(t_urlloader), CLASS_NOINLET, A_GIMME, 0);
}
