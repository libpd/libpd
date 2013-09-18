/* This is a version hacked by Martin Peach 20110120 martin.peach@sympatico.ca */
/* Reformmatted the code and added some debug messages. Changed the name of the class to pdlua */
/** @file lua.c 
 *  @brief pdlua -- a Lua embedding for Pd.
 *  @author Claude Heiland-Allen <claudiusmaximus@goto10.org>
 *  @date 2008
 *  @version 0.6~svn
 *
 * Copyright (C) 2007,2008 Claude Heiland-Allen <claudiusmaximus@goto10.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */ 

/* various C stuff, mainly for reading files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // for open
#include <sys/stat.h> // for open
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h> // for open
#define read _read
#define close _close
#define ssize_t int
#define snprintf _snprintf
#else
#include <sys/fcntl.h> // for open
#include <unistd.h>
#endif
/* we use Lua */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* we use Pd */
#include "m_pd.h"
#include "s_stuff.h" // for sys_register_loader()
#include "m_imp.h" // for struct _class
/* BAD: support for Pd < 0.41 */

#if PD_MAJOR_VERSION == 0
# if PD_MINOR_VERSION >= 41
#  define PDLUA_PD41
/* use new garray support that is 64-bit safe */
#  define PDLUA_ARRAYGRAB garray_getfloatwords
#  define PDLUA_ARRAYTYPE t_word
#  define PDLUA_ARRAYELEM(arr,idx) ((arr)[(idx)].w_float)
# elif PD_MINOR_VERSION >= 40
#  define PDLUA_PD40
/* use old garray support, not 64-bit safe */
#  define PDLUA_ARRAYGRAB garray_getfloatarray
#  define PDLUA_ARRAYTYPE t_float
#  define PDLUA_ARRAYELEM(arr,idx) ((arr)[(idx)])
# elif PD_MINOR_VERSION >= 39
#  define PDLUA_PD39
/* use old garray support, not 64-bit safe */
#  define PDLUA_ARRAYGRAB garray_getfloatarray
#  define PDLUA_ARRAYTYPE t_float
#  define PDLUA_ARRAYELEM(arr,idx) ((arr)[(idx)])
# else
#  error "Pd version is too old, please upgrade"
# endif
#else
# error "Pd version is too new, please file a bug report"
#endif

/* BAD: end of bad section */

/* If defined, PDLUA_DEBUG lets pdlua post a lot of text */
//#define PDLUA_DEBUG

/** Global Lua interpreter state, needed in the constructor. */
static lua_State *L;

/** State for the Lua file reader. */
typedef struct pdlua_readerdata
{
    int         fd; /**< File descriptor to read from. */
    char        buffer[MAXPDSTRING]; /**< Buffer to read into. */
} t_pdlua_readerdata;

/** Pd object data. */
typedef struct pdlua 
{
    t_object                pd; /**< We are a Pd object. */
    int                     inlets; /**< Number of inlets. */
    struct pdlua_proxyinlet *in; /**< The inlets themselves. */
    int                     outlets; /**< Number of outlets. */
    t_outlet                **out; /**< The outlets themselves. */
    t_canvas                *canvas; /**< The canvas that the object was created on. */
} t_pdlua;

/** Proxy inlet object data. */
typedef struct pdlua_proxyinlet
{
    t_pd            pd; /**< Minimal Pd object. */
    struct pdlua    *owner; /**< The owning object to forward inlet messages to. */
    unsigned int    id; /**< The number of this inlet. */
} t_pdlua_proxyinlet;

/** Proxy receive object data. */
typedef struct pdlua_proxyreceive
{
    t_pd            pd; /**< Minimal Pd object. */
    struct pdlua    *owner; /**< The owning object to forward received messages to. */
    t_symbol        *name; /**< The receive-symbol to bind to. */
} t_pdlua_proxyreceive;

/** Proxy clock object data. */
typedef struct pdlua_proxyclock
{
    t_pd            pd; /**< Minimal Pd object. */
    struct pdlua    *owner; /**< Object to forward messages to. */
    t_clock         *clock; /** Pd clock to use. */
} t_pdlua_proxyclock;
/* prototypes*/

static const char *pdlua_reader (lua_State *L, void *rr, size_t *size);
/** Proxy inlet 'anything' method. */
static void pdlua_proxyinlet_anything (t_pdlua_proxyinlet *p, t_symbol *s, int argc, t_atom *argv);
/** Proxy inlet initialization. */
static void pdlua_proxyinlet_init (t_pdlua_proxyinlet *p, struct pdlua *owner, unsigned int id);
/** Register the proxy inlet class with Pd. */
static void pdlua_proxyinlet_setup (void);
/** Proxy receive 'anything' method. */
static void pdlua_proxyreceive_anything (t_pdlua_proxyreceive *r, t_symbol *s, int argc, t_atom *argv);
/** Proxy receive allocation and initialization. */
static t_pdlua_proxyreceive *pdlua_proxyreceive_new (struct pdlua *owner, t_symbol *name);
/** Proxy receive cleanup and deallocation. */
static void pdlua_proxyreceive_free (t_pdlua_proxyreceive *r /**< The proxy receive to free. */);
/** Register the proxy receive class with Pd. */
static void pdlua_proxyreceive_setup (void);
/** Proxy clock 'bang' method. */
static void pdlua_proxyclock_bang (t_pdlua_proxyclock *c);
/** Proxy clock allocation and initialization. */
static t_pdlua_proxyclock *pdlua_proxyclock_new (struct pdlua *owner);
/** Register the proxy clock class with Pd. */
static void pdlua_proxyclock_setup (void);
/** Dump an array of atoms into a Lua table. */
static void pdlua_pushatomtable (int argc, t_atom *argv);
/** Pd object constructor. */
static t_pdlua *pdlua_new (t_symbol *s, int argc, t_atom *argv);
/** Pd object destructor. */
static void pdlua_free (t_pdlua *o );
static void pdlua_stack_dump (lua_State *L);
/** a handler for the open item in the right-click menu (mrpeach 20111025) */
/** Here we find the lua code for the object and open it in an editor */
static void pdlua_menu_open (t_pdlua *o);
/** Lua class registration. This is equivalent to the "setup" method for an ordinary Pd class */
static int pdlua_class_new (lua_State *L);
/** Lua object creation. */
static int pdlua_object_new (lua_State *L);
/** Lua object inlet creation. */
static int pdlua_object_createinlets (lua_State *L);
/** Lua object outlet creation. */
static int pdlua_object_createoutlets (lua_State *L);
/** Lua object receive creation. */
static int pdlua_receive_new (lua_State *L);
/** Lua object receive destruction. */
static int pdlua_receive_free (lua_State *L);
/** Lua object clock creation. */
static int pdlua_clock_new (lua_State *L);
/** Lua proxy clock delay. */
static int pdlua_clock_delay (lua_State *L);
/** Lua proxy clock set. */
static int pdlua_clock_set (lua_State *L);
/** Lua proxy clock unset. */
static int pdlua_clock_unset (lua_State *L);
/** Lua proxy clock destruction. */
static int pdlua_clock_free (lua_State *L);
/** Lua object destruction. */
static int pdlua_object_free (lua_State *L);
/** Dispatch Pd inlet messages to Lua objects. */
static void pdlua_dispatch (t_pdlua *o, unsigned int inlet, t_symbol *s, int argc, t_atom *argv);
/** Dispatch Pd receive messages to Lua objects. */
static void pdlua_receivedispatch (t_pdlua_proxyreceive *r, t_symbol *s, int argc, t_atom *argv);
/** Dispatch Pd clock messages to Lua objects. */
static void pdlua_clockdispatch(t_pdlua_proxyclock *clock);
/** Convert a Lua table into a Pd atom array. */
static t_atom *pdlua_popatomtable (lua_State *L, int *count, t_pdlua *o);
/** Send a message from a Lua object outlet. */
static int pdlua_outlet (lua_State *L);
/** Send a message from a Lua object to a Pd receiver. */
static int pdlua_send (lua_State *L);
/** Set a [value] object's value. */
static int pdlua_setvalue (lua_State *L);
/** Get a [value] object's value. */
static int pdlua_getvalue (lua_State *L);
/** Get a [table] object's array. */
static int pdlua_getarray (lua_State *L);
/** Read from a [table] object's array. */
static int pdlua_readarray (lua_State *L);
/** Write to a [table] object's array. */
static int pdlua_writearray (lua_State *L);
/** Redraw a [table] object's graph. */
static int pdlua_redrawarray (lua_State *L);
/** Post to Pd's console. */
static int pdlua_post (lua_State *L);
/** Report an error from a Lua object to Pd's console. */
static int pdlua_error (lua_State *L);
static void pdlua_setrequirepath (lua_State *L, const char *path);
static void pdlua_clearrequirepath (lua_State *L);
/** Run a Lua script using Pd's path. */
static int pdlua_dofile (lua_State *L);
/** Initialize the pd API for Lua. */
static void pdlua_init (lua_State *L);
/** Pd loader hook for loading and executing Lua scripts. */
static int pdlua_loader (t_canvas *canvas, char *name);
/** Start the Lua runtime and register our loader hook. */
#ifdef _WIN32
__declspec(dllexport)
#endif 
void pdlua_setup (void);
/* end prototypes*/

/* globals */
struct pdlua_proxyinlet;
struct pdlua_proxyreceive;
struct pdlua_proxyclock;
/** Proxy inlet class pointer. */
static t_class *pdlua_proxyinlet_class;
/** Proxy receive class pointer. */
static t_class *pdlua_proxyreceive_class;
/** Proxy clock class pointer. */
static t_class *pdlua_proxyclock_class;

/** Lua file reader callback. */
static const char *pdlua_reader
(
    lua_State *L, /**< Lua interpreter state. */
    void *rr, /**< Lua file reader state. */
    size_t *size /**< How much data we have read. */
)
{
    t_pdlua_readerdata  *r = rr;
    ssize_t             s;
#ifdef PDLUA_DEBUG
    post("pdlua_reader: fd is %d", r->fd);
#endif // PDLUA_DEBUG
    s = read(r->fd, r->buffer, MAXPDSTRING-2);
#ifdef PDLUA_DEBUG
    post("pdlua_reader: s is %ld", s);////////
#endif // PDLUA_DEBUG
    if (s <= 0)
    {
        *size = 0;
        return NULL;
    }
    else
    {
        *size = s;
        return r->buffer;
    }
}

/** Proxy inlet 'anything' method. */
static void pdlua_proxyinlet_anything
(
    t_pdlua_proxyinlet  *p, /**< The proxy inlet that received the message. */
    t_symbol            *s, /**< The message selector. */
    int                 argc, /**< The message length. */
    t_atom              *argv /**< The atoms in the message. */
)
{
    pdlua_dispatch(p->owner, p->id, s, argc, argv);
}

/** Proxy inlet initialization. */
static void pdlua_proxyinlet_init
(
    t_pdlua_proxyinlet  *p, /**< The proxy inlet to initialize. */
    struct pdlua        *owner, /**< The owning object. */
    unsigned int        id /**< The inlet number. */
)
{
    p->pd = pdlua_proxyinlet_class;
    p->owner = owner;
    p->id = id;
}

/** Register the proxy inlet class with Pd. */
static void pdlua_proxyinlet_setup(void)
{
    pdlua_proxyinlet_class = class_new(gensym("pdlua proxy inlet"), 0, 0, sizeof(t_pdlua_proxyinlet), 0, 0);
    class_addanything(pdlua_proxyinlet_class, pdlua_proxyinlet_anything);
}

/** Proxy receive 'anything' method. */
static void pdlua_proxyreceive_anything(
    t_pdlua_proxyreceive    *r, /**< The proxy receive that received the message. */
    t_symbol                *s, /**< The message selector. */
    int                     argc, /**< The message length. */
    t_atom                  *argv /**< The atoms in the message. */
)
{
    pdlua_receivedispatch(r, s, argc, argv);
}

/** Proxy receive allocation and initialization. */
static t_pdlua_proxyreceive *pdlua_proxyreceive_new
(
    struct pdlua    *owner, /**< The owning object. */
    t_symbol        *name /**< The symbol to bind to. */
)
{
    t_pdlua_proxyreceive *r = malloc(sizeof(t_pdlua_proxyreceive));
    r->pd = pdlua_proxyreceive_class;
    r->owner = owner;
    r->name = name;
    pd_bind(&r->pd, r->name);
    return r;
}

/** Proxy receive cleanup and deallocation. */
static void pdlua_proxyreceive_free(t_pdlua_proxyreceive *r /**< The proxy receive to free. */)
{
    pd_unbind(&r->pd, r->name);
    r->pd = NULL;
    r->owner = NULL;
    r->name = NULL;
    free(r);
}

/** Register the proxy receive class with Pd. */
static void pdlua_proxyreceive_setup()
{
    pdlua_proxyreceive_class = class_new(gensym("pdlua proxy receive"), 0, 0, sizeof(t_pdlua_proxyreceive), 0, 0);
    class_addanything(pdlua_proxyreceive_class, pdlua_proxyreceive_anything);
}

/** Proxy clock 'bang' method. */
static void pdlua_proxyclock_bang(t_pdlua_proxyclock *c /**< The proxy clock that received the message. */)
{
    pdlua_clockdispatch(c);
}

/** Proxy clock allocation and initialization. */
static t_pdlua_proxyclock *pdlua_proxyclock_new
(
    struct pdlua *owner /**< The object to forward messages to. */
)
{
    t_pdlua_proxyclock *c = malloc(sizeof(t_pdlua_proxyclock));
    c->pd = pdlua_proxyclock_class;
    c->owner = owner;
    c->clock = clock_new(c, (t_method) pdlua_proxyclock_bang);
    return c;
}

/** Register the proxy clock class with Pd. */
static void pdlua_proxyclock_setup(void)
{
    pdlua_proxyclock_class = class_new(gensym("pdlua proxy clock"), 0, 0, sizeof(t_pdlua_proxyclock), 0, 0);
}

/** Dump an array of atoms into a Lua table. */
static void pdlua_pushatomtable
(
    int     argc, /**< The number of atoms in the array. */
    t_atom  *argv /**< The array of atoms. */
)
{
    int i;

#ifdef PDLUA_DEBUG
    post("pdlua_pushatomtable: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_newtable(L);
    for (i = 0; i < argc; ++i)
    {
        lua_pushnumber(L, i+1);
        switch (argv[i].a_type)
        {
            case A_FLOAT:
                lua_pushnumber(L, argv[i].a_w.w_float);
                break;
            case A_SYMBOL:
                lua_pushstring(L, argv[i].a_w.w_symbol->s_name);
                break;
            case A_POINTER: /* FIXME: check experimentality */
                lua_pushlightuserdata(L, argv[i].a_w.w_gpointer);
            break;
            default:
                error("lua: zomg weasels!");
                lua_pushnil(L);
            break;
        }
        lua_settable(L, -3);
    }
#ifdef PDLUA_DEBUG
    post("pdlua_pushatomtable: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
}

/** Pd object constructor. */
static t_pdlua *pdlua_new
(
    t_symbol    *s, /**< The construction message selector. */
    int         argc, /**< The construction message atom count. */
    t_atom      *argv /**< The construction message atoms. */
)
{
    int i;
#ifdef PDLUA_DEBUG
    post("pdlua_new: s->s_name is %s", s->s_name);
#endif // PDLUA_DEBUG
    for (i = 0; i < argc; ++i)
    {
        switch (argv[i].a_type)
        {
        case A_FLOAT:
#ifdef PDLUA_DEBUG
            post("argv[%d]: %f", i, argv[i].a_w.w_float);
#endif // PDLUA_DEBUG
            break;
        case A_SYMBOL:
#ifdef PDLUA_DEBUG
            post("argv[%d]: %s", i, argv[i].a_w.w_symbol->s_name);
#endif // PDLUA_DEBUG
            break;
        default:
            error("pdlua_new: bad argument type"); // should never happen
            return NULL;
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_new: start with stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_getfield(L, -1, "_constructor");
    lua_pushstring(L, s->s_name);
    pdlua_pushatomtable(argc, argv);
#ifdef PDLUA_DEBUG
    post("pdlua_new: before lua_pcall(L, 2, 1, 0) stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_pcall(L, 2, 1, 0))
    {
        error("pdlua_new: error in constructor for `%s':\n%s", s->s_name, lua_tostring(L, -1));
        lua_pop(L, 2); /* pop the error string and the global "pd" */
        return NULL;
    }
    else
    {
        t_pdlua *object = NULL;
#ifdef PDLUA_DEBUG
        post("pdlua_new: done lua_pcall(L, 2, 1, 0) stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
        if (lua_islightuserdata(L, -1))
        {
            object = lua_touserdata(L, -1);
            lua_pop(L, 2);/* pop the userdata and the global "pd" */
#ifdef PDLUA_DEBUG
            post("pdlua_new: before returning object %p stack top %d", object, lua_gettop(L));
#endif // PDLUA_DEBUG
             return object;
        }
        else
        {
            lua_pop(L, 2);/* pop the userdata and the global "pd" */
#ifdef PDLUA_DEBUG
            post("pdlua_new: done FALSE lua_islightuserdata(L, -1)");
#endif // PDLUA_DEBUG
            return NULL;
        }
    }
}

/** Pd object destructor. */
static void pdlua_free( t_pdlua *o /**< The object to destruct. */)
{
#ifdef PDLUA_DEBUG
    post("pdlua_free: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_getfield (L, -1, "_destructor");
    lua_pushlightuserdata(L, o);
    if (lua_pcall(L, 1, 0, 0))
    {
        error("lua: error in destructor:\n%s", lua_tostring(L, -1));
        lua_pop(L, 1); /* pop the error string */
    }
    lua_pop(L, 1); /* pop the global "pd" */
#ifdef PDLUA_DEBUG
    post("pdlua_free: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return;
}

static void pdlua_stack_dump (lua_State *L)
{
    int i;
    int top = lua_gettop(L);

    for (i = 1; i <= top; i++)
    {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t)
        {
            case LUA_TSTRING:  /* strings */
                printf("`%s'", lua_tostring(L, i));
                break;
    
            case LUA_TBOOLEAN:  /* booleans */
                printf(lua_toboolean(L, i) ? "true" : "false");
                break;
    
            case LUA_TNUMBER:  /* numbers */
                printf("%g", lua_tonumber(L, i));
                break;
    
            default:  /* other values */
                printf("%s", lua_typename(L, t));
                break;
        }
        printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
}

/** a handler for the open item in the right-click menu (mrpeach 20111025) */
/** Here we find the lua code for the object and open it in an editor */
static void pdlua_menu_open(t_pdlua *o)
{
    const char  *name;
    const char  *path;
    char        pathname[FILENAME_MAX];
    t_class     *class;

#ifdef PDLUA_DEBUG
    post("pdlua_menu_open stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    /** Get the scriptname of the object */
    lua_getglobal(L, "pd");
    lua_getfield(L, -1, "_whoami");
    lua_pushlightuserdata(L, o);
    if (lua_pcall(L, 1, 1, 0))
    {
        error("lua: error in whoami:\n%s", lua_tostring(L, -1));
        lua_pop(L, 2); /* pop the error string and the global "pd" */
        return;
    }
    name = luaL_checkstring(L, -1);
#ifdef PDLUA_DEBUG
    post("pdlua_menu_open: L is %p, name is %s stack top is %d", L, name, lua_gettop(L));
#endif // PDLUA_DEBUG
    if (name)
    {
        if (name[strlen(name)-1] == 'x')
        {
            /* pdluax is a class, the particular file should loadable by name alone, we hope */
            sprintf(pathname, "%s", name);
            lua_pop(L, 2); /* pop name and the global "pd" */
        }
        else
        {
            lua_getglobal(L, "pd");
            lua_getfield(L, -1, "_get_class");
            lua_pushlightuserdata(L, o);
            if (lua_pcall(L, 1, 1, 0))
            {
                error("lua: error in get_class:\n%s", lua_tostring(L, -1));
                lua_pop(L, 4); /* pop the error string, global "pd", name, global "pd"*/
                return;
            }
            class = (t_class *)lua_touserdata(L, -1);
            path = class->c_externdir->s_name;
            sprintf(pathname, "%s/%s", path, name);
            lua_pop(L, 4); /* pop class, global "pd", name, global "pd"*/
        }
#if PD_MAJOR_VERSION==0 && PD_MINOR_VERSION<43
        post("Opening %s for editing", pathname);
#else
        logpost(NULL, 3, "Opening %s for editing", pathname);
#endif
        sys_vgui("::pd_menucommands::menu_openfile {%s}\n", pathname);
    }
#ifdef PDLUA_DEBUG
    post("pdlua_menu_open end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
}

/** Lua class registration. This is equivalent to the "setup" method for an ordinary Pd class */
static int pdlua_class_new(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Class name string.
  * \par Outputs:
  * \li \c 1 Pd class pointer.
  * */
{
    const char  *name;
    t_class     *c;

    name = luaL_checkstring(L, 1);
#ifdef PDLUA_DEBUG
    post("pdlua_class_new: L is %p, name is %s stack top is %d", L, name, lua_gettop(L));
#endif // PDLUA_DEBUG
    c = class_new(gensym((char *) name), (t_newmethod) pdlua_new,
        (t_method) pdlua_free, sizeof(t_pdlua), CLASS_NOINLET, A_GIMME, 0);

/* a class with a "menu-open" method will have the "Open" item highlighted in the right-click menu */
    class_addmethod(c, (t_method)pdlua_menu_open, gensym("menu-open"), A_NULL);/* (mrpeach 20111025) */
/**/

    lua_pushlightuserdata(L, c);
#ifdef PDLUA_DEBUG
    post("pdlua_class_new: end stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 1;
}

/** Lua object creation. */
static int pdlua_object_new(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd class pointer.
  * \par Outputs:
  * \li \c 2 Pd object pointer.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_object_new: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_class *c = lua_touserdata(L, 1);
        if (c)
        {
#ifdef PDLUA_DEBUG
            char *path = c->c_externdir->s_name;
            post("pdlua_object_new: path is %s", path);
#endif // PDLUA_DEBUG
            t_pdlua *o = (t_pdlua *) pd_new(c);
            if (o)
            {
                o->inlets = 0;
                o->in = NULL;
                o->outlets = 0;
                o->out = NULL;
                o->canvas = canvas_getcurrent();
                lua_pushlightuserdata(L, o);
#ifdef PDLUA_DEBUG
                post("pdlua_object_new: success end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
                return 1;
            }
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_object_new: fail end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua object inlet creation. */
static int pdlua_object_createinlets(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \li \c 2 Number of inlets.
  * */
{
    int i;

#ifdef PDLUA_DEBUG
    post("pdlua_object_createinlets: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua *o = lua_touserdata(L, 1);
        if (o)
        {
            o->inlets = luaL_checknumber(L, 2);
            o->in = malloc(o->inlets * sizeof(t_pdlua_proxyinlet));
            for (i = 0; i < o->inlets; ++i)
            {
                pdlua_proxyinlet_init(&o->in[i], o, i);
                inlet_new(&o->pd, &o->in[i].pd, 0, 0);
            }
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_object_createinlets: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua object outlet creation. */
static int pdlua_object_createoutlets(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \li \c 2 Number of outlets.
  * */
{
    int i;

#ifdef PDLUA_DEBUG
    post("pdlua_object_createoutlets: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua *o = lua_touserdata(L, 1);
        if (o)
        {
            o->outlets = luaL_checknumber(L, 2);
            if (o->outlets > 0)
            {
                o->out = malloc(o->outlets * sizeof(t_outlet *));
                for (i = 0; i < o->outlets; ++i) o->out[i] = outlet_new(&o->pd, 0);
            }
            else o->out = NULL;
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_object_createoutlets: end stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua object receive creation. */
static int pdlua_receive_new(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \li \c 2 Receive name string.
  * \par Outputs:
  * \li \c 1 Pd receive pointer.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_receive_new: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua *o = lua_touserdata(L, 1);
        if (o)
        {
            const char *name = luaL_checkstring(L, 2);
            if (name)
            {
                t_pdlua_proxyreceive *r =  pdlua_proxyreceive_new(o, gensym((char *) name)); /* const cast */
                lua_pushlightuserdata(L, r);
#ifdef PDLUA_DEBUG
                post("pdlua_receive_new: success end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
                return 1;
            }
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_receive_new: fail end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua object receive destruction. */
static int pdlua_receive_free(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd recieve pointer.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_receive_free: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua_proxyreceive *r = lua_touserdata(L, 1);
        if (r) pdlua_proxyreceive_free(r);
    }
#ifdef PDLUA_DEBUG
    post("pdlua_receive_free: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua object clock creation. */
static int pdlua_clock_new(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \par Outputs:
  * \li \c 1 Pd clock pointer.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_clock_new: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua *o = lua_touserdata(L, 1);
        if (o)
        {
            t_pdlua_proxyclock *c =  pdlua_proxyclock_new(o);
            lua_pushlightuserdata(L, c);
#ifdef PDLUA_DEBUG
            post("pdlua_clock_new: success end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
            return 1;
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_clock_new: fail end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua proxy clock delay. */
static int pdlua_clock_delay(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd clock pointer.
  * \li \c 2 Number of milliseconds to delay.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_clock_delay: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua_proxyclock *c = lua_touserdata(L, 1);
        if (c)
        {
            double delaytime = luaL_checknumber(L, 2);
            clock_delay(c->clock, delaytime);
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_clock_delay: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua proxy clock set. */
static int pdlua_clock_set(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd clock pointer.
  * \li \c 2 Number to set the clock.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_clock_set: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua_proxyclock *c = lua_touserdata(L, 1);
        if (c)
        {
            double systime = luaL_checknumber(L, 2);
            clock_set(c->clock, systime);
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_clock_set: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua proxy clock unset. */
static int pdlua_clock_unset(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd clock pointer.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_clock_unset: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua_proxyclock *c = lua_touserdata(L, 1);
        if (c) clock_unset(c->clock);
    }
#ifdef PDLUA_DEBUG
    post("pdlua_clock_unset: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua proxy clock destruction. */
static int pdlua_clock_free(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd clock pointer.
  * */
{
#ifdef PDLUA_DEBUG
    post("pdlua_clock_free: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua_proxyclock *c = lua_touserdata(L, 1);
        if (c)
        {
            clock_free(c->clock);
            free(c);
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_clock_free: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Lua object destruction. */
static int pdlua_object_free(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * */
{
    int i;

#ifdef PDLUA_DEBUG
    post("pdlua_object_free: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        t_pdlua *o = lua_touserdata(L, 1);
        if (o)
        {
            if (o->in) free(o->in);
            if(o->out)
            {
                for (i = 0; i < o->outlets; ++i) outlet_free(o->out[i]);
                free(o->out);
                o->out = NULL;
            }
        }
    }
#ifdef PDLUA_DEBUG
    post("pdlua_object_free: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Dispatch Pd inlet messages to Lua objects. */
static void pdlua_dispatch
(
    t_pdlua         *o, /**< The object that received the message. */
    unsigned int    inlet, /**< The inlet that the message arrived at. */
    t_symbol        *s, /**< The message selector. */
    int             argc, /**< The message length. */
    t_atom          *argv /**< The atoms in the message. */
)
{
#ifdef PDLUA_DEBUG
    post("pdlua_dispatch: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_getfield (L, -1, "_dispatcher");
    lua_pushlightuserdata(L, o);
    lua_pushnumber(L, inlet + 1); /* C has 0.., Lua has 1.. */
    lua_pushstring(L, s->s_name);
    pdlua_pushatomtable(argc, argv);
    if (lua_pcall(L, 4, 0, 0))
    {
        pd_error(o, "lua: error in dispatcher:\n%s", lua_tostring(L, -1));
        lua_pop(L, 1); /* pop the error string */
    }
    lua_pop(L, 1); /* pop the global "pd" */
#ifdef PDLUA_DEBUG
    post("pdlua_dispatch: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return;  
}

/** Dispatch Pd receive messages to Lua objects. */
static void pdlua_receivedispatch
(
    t_pdlua_proxyreceive    *r, /**< The proxy receive that received the message. */
    t_symbol                *s, /**< The message selector. */
    int                     argc, /**< The message length. */
    t_atom                  *argv /**< The atoms in the message. */
)
{
#ifdef PDLUA_DEBUG
    post("pdlua_receivedispatch: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_getfield (L, -1, "_receivedispatch");
    lua_pushlightuserdata(L, r);
    lua_pushstring(L, s->s_name);
    pdlua_pushatomtable(argc, argv);
    if (lua_pcall(L, 3, 0, 0))
    {
        pd_error(r->owner, "lua: error in receive dispatcher:\n%s", lua_tostring(L, -1));
        lua_pop(L, 1); /* pop the error string */
    }
    lua_pop(L, 1); /* pop the global "pd" */
#ifdef PDLUA_DEBUG
    post("pdlua_receivedispatch: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return;  
}

/** Dispatch Pd clock messages to Lua objects. */
static void pdlua_clockdispatch( t_pdlua_proxyclock *clock)
/**< The proxy clock that received the message. */
{
#ifdef PDLUA_DEBUG
    post("pdlua_clockdispatch: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_getfield (L, -1, "_clockdispatch");
    lua_pushlightuserdata(L, clock);
    if (lua_pcall(L, 1, 0, 0))
    {
        pd_error(clock->owner, "lua: error in clock dispatcher:\n%s", lua_tostring(L, -1));
        lua_pop(L, 1); /* pop the error string */
    }
    lua_pop(L, 1); /* pop the global "pd" */
#ifdef PDLUA_DEBUG
    post("pdlua_clockdispatch: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return;  
}

/** Convert a Lua table into a Pd atom array. */
static t_atom *pdlua_popatomtable
(
    lua_State   *L, /**< Lua interpreter state.
  * \par Inputs:
  * \li \c -1 Table to convert.
  * */
    int         *count, /**< Where to store the array length. */
    t_pdlua     *o /**< Object reference for error messages. */
)
{
    int         i;
    int         ok = 1;
    t_float     f;
    const char  *s;
    void        *p;
    size_t      sl;
    t_atom      *atoms = NULL;

#ifdef PDLUA_DEBUG
    post("pdlua_popatomtable: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_istable(L, -1))
    {
        *count = lua_objlen(L, -1);
        if (*count > 0) atoms = malloc(*count * sizeof(t_atom));
        i = 0;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            if (i == *count)
            {
                pd_error(o, "lua: error: too many table elements");
                ok = 0;
                break;
            }
            switch (lua_type(L, -1))
            {
                case (LUA_TNUMBER):
                    f = lua_tonumber(L, -1);
                    SETFLOAT(&atoms[i], f);
                    break;
                case (LUA_TSTRING):
                    s = lua_tolstring(L, -1, &sl);
                    if (s)
                    {
                        if (strlen(s) != sl) pd_error(o, "lua: warning: symbol munged (contains \\0 in body)");
                        SETSYMBOL(&atoms[i], gensym((char *) s));
                    }
                    else
                    {
                        pd_error(o, "lua: error: null string in table");
                        ok = 0;
                    }
                    break;
                case (LUA_TLIGHTUSERDATA): /* FIXME: check experimentality */
                    p = lua_touserdata(L, -1);
                    SETPOINTER(&atoms[i], p);
                    break;
                default:
                    pd_error(o, "lua: error: table element must be number or string or pointer");
                    ok = 0;
                    break;
            }
            lua_pop(L, 1);
            ++i;
        }
        if (i != *count)
        {
            pd_error(o, "lua: error: too few table elements");
            ok = 0;
        }
    }
    else 
    {
        pd_error(o, "lua: error: not a table");
        ok = 0;
    }
    lua_pop(L, 1);
#ifdef PDLUA_DEBUG
    post("pdlua_popatomtable: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (ok) return atoms;
    if (atoms) free(atoms);
    return NULL;
}

/** Send a message from a Lua object outlet. */
static int pdlua_outlet(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \li \c 2 Outlet number.
  * \li \c 3 Message selector string.
  * \li \c 4 Message atom table.
  * */
{
    t_pdlua         *o;
    int             out;
    size_t          sl;
    const char      *s;
    t_symbol        *sym;
    int             count;
    t_atom          *atoms;

#ifdef PDLUA_DEBUG
    post("pdlua_outlet: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        o = lua_touserdata(L, 1);
        if (o) 
        {
            if (lua_isnumber(L, 2)) out = lua_tonumber(L, 2) - 1; /* C has 0.., Lua has 1.. */
            else
            {
                pd_error(o, "lua: error: outlet must be a number");
                lua_pop(L, 4); /* pop all the arguments */
                return 0;
            }
            if (0 <= out && out < o->outlets) 
            {
                if (lua_isstring(L, 3)) 
                {
                    s = lua_tolstring(L, 3, &sl);
                    sym = gensym((char *) s); /* const cast */
                    if (s)
                    {
                        if (strlen(s) != sl) pd_error(o, "lua: warning: symbol munged (contains \\0 in body)");
                        lua_pushvalue(L, 4);
                        atoms = pdlua_popatomtable(L, &count, o);
                        if (count == 0 || atoms) outlet_anything(o->out[out], sym, count, atoms);
                        else pd_error(o, "lua: error: no atoms??");
                        if (atoms) 
                        {
                            free(atoms);
                            lua_pop(L, 4); /* pop all the arguments */
                            return 0;
                        }
                    }
                    else pd_error(o, "lua: error: null selector");
                }
                else pd_error(o, "lua: error: selector must be a string");
            }
            else pd_error(o, "lua: error: outlet out of range");
        }
        else error("lua: error: no object to outlet from");
    }
    else error("lua: error: bad arguments to outlet");
    lua_pop(L, 4); /* pop all the arguments */
#ifdef PDLUA_DEBUG
    post("pdlua_outlet: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Send a message from a Lua object to a Pd receiver. */
static int pdlua_send(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Receiver string.
  * \li \c 2 Message selector string.
  * \li \c 3 Message atom table.
  * */

{
    size_t          receivenamel;
    const char      *receivename;
    t_symbol        *receivesym;
    size_t          selnamel;
    const char      *selname;
    t_symbol        *selsym;
    int             count;
    t_atom          *atoms;

#ifdef PDLUA_DEBUG
    post("pdlua_send: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_isstring(L, 1)) 
    {
        receivename = lua_tolstring(L, 1, &receivenamel);
        receivesym = gensym((char *) receivename); /* const cast */
        if (receivesym) 
        {
            if (strlen(receivename) != receivenamel) error("lua: warning: symbol munged (contains \\0 in body)");
            if (lua_isstring(L, 2)) 
            {
                selname = lua_tolstring(L, 2, &selnamel);
                selsym = gensym((char *) selname); /* const cast */
                if (selsym)
                {
                    if (strlen(selname) != selnamel) error("lua: warning: symbol munged (contains \\0 in body)");
                    lua_pushvalue(L, 3);
                    atoms = pdlua_popatomtable(L, &count, NULL);
                    if ((count == 0 || atoms) && (receivesym->s_thing)) typedmess(receivesym->s_thing, selsym, count, atoms);
                    else error("lua: error: no atoms??");
                    if (atoms) 
                    {
                        free(atoms);
#ifdef PDLUA_DEBUG
                        post("pdlua_send: success end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
                        return 0;
                    }
                }
                else error("lua: error: null selector");
            }
            else error("lua: error: selector must be a string");
        }
        else error("lua: error: null receive name");
    }
    else error("lua: error: receive name must be string");
#ifdef PDLUA_DEBUG
    post("pdlua_send: fail end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Set a [value] object's value. */
static int pdlua_setvalue(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Value name string.
  * \li \c 2 Value number.
  * \par Outputs:
  * \li \c 1 success (usually depends on a [value] existing or not).
  */
{
    const char  *str = luaL_checkstring(L, 1);
    t_float     val = luaL_checknumber(L, 2);
    int         err = value_setfloat(gensym(str), val);

#ifdef PDLUA_DEBUG
    post("pdlua_setvalue: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_pushboolean(L, !err);
#ifdef PDLUA_DEBUG
    post("pdlua_setvalue: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 1;
}

/** Get a [value] object's value. */
static int pdlua_getvalue(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Value name string.
  * \par Outputs:
  * \li \c 1 Value number, or nil for failure.
  * */
{
    const char  *str = luaL_checkstring(L, 1);
    t_float     val;
    int         err = value_getfloat(gensym(str), &val);

#ifdef PDLUA_DEBUG
    post("pdlua_getvalue: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (!err) lua_pushnumber(L, val);
    else lua_pushnil(L);
#ifdef PDLUA_DEBUG
    post("pdlua_getvalue: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 1;
}

/** Get a [table] object's array. */
static int pdlua_getarray(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Table name string.
  * \par Outputs:
  * \li \c 1 Table length, or < 0 for failure.
  * \li \c 2 Table pointer, or nil for failure.
  * */
{
    t_garray        *a;
    int             n;
    PDLUA_ARRAYTYPE *v;
    const char      *str = luaL_checkstring(L, 1);

#ifdef PDLUA_DEBUG
    post("pdlua_getarray: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (!(a = (t_garray *) pd_findbyclass(gensym(str), garray_class))) 
    {
        lua_pushnumber(L, -1);
#ifdef PDLUA_DEBUG
        post("pdlua_getarray: end 1. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
        return 1;
    }
    else if (!PDLUA_ARRAYGRAB(a, &n, &v)) 
    {
        lua_pushnumber(L, -2);
#ifdef PDLUA_DEBUG
        post("pdlua_getarray: end 2. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
        return 1;
    }
    else 
    {
        lua_pushnumber(L, n);
        lua_pushlightuserdata(L, v);
#ifdef PDLUA_DEBUG
        post("pdlua_getarray: end 3. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
        return 2;
    }
}

/** Read from a [table] object's array. */
static int pdlua_readarray(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Table length number.
  * \li \c 2 Table array pointer.
  * \li \c 3 Table index number.
  * \par Outputs:
  * \li \c 1 Table element value, or nil for index out of range.
  * */
{
    int             n = luaL_checknumber(L, 1);
    PDLUA_ARRAYTYPE *v = lua_islightuserdata(L, 2) ? lua_touserdata(L, 2) : NULL;
    int             i = luaL_checknumber(L, 3);

#ifdef PDLUA_DEBUG
    post("pdlua_readarray: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (0 <= i && i < n && v) 
    {
        lua_pushnumber(L, PDLUA_ARRAYELEM(v, i));
#ifdef PDLUA_DEBUG
        post("pdlua_readarray: end 1. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
        return 1;
    }
#ifdef PDLUA_DEBUG
    post("pdlua_readarray: end 2. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Write to a [table] object's array. */
static int pdlua_writearray(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Table length number.
  * \li \c 2 Table array pointer.
  * \li \c 3 Table index number.
  * \li \c 4 Table element value number.
  * */
{
    int                 n = luaL_checknumber(L, 1);
    PDLUA_ARRAYTYPE     *v = lua_islightuserdata(L, 2) ? lua_touserdata(L, 2) : NULL;
    int                 i = luaL_checknumber(L, 3);
    t_float             x = luaL_checknumber(L, 4);

#ifdef PDLUA_DEBUG
    post("pdlua_writearray: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (0 <= i && i < n && v) PDLUA_ARRAYELEM(v, i) = x;
#ifdef PDLUA_DEBUG
    post("pdlua_writearray: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Redraw a [table] object's graph. */
static int pdlua_redrawarray(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Table name string.
  * */
{
    t_garray    *a;
    const char  *str = luaL_checkstring(L, 1);

#ifdef PDLUA_DEBUG
    post("pdlua_redrawarray: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if ((a = (t_garray *) pd_findbyclass(gensym(str), garray_class))) garray_redraw(a);
#ifdef PDLUA_DEBUG
    post("pdlua_redrawarray: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Post to Pd's console. */
static int pdlua_post(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Message string.
  * */
{
    const char *str = luaL_checkstring(L, 1);
#ifdef PDLUA_DEBUG
    post("pdlua_post: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    post("%s", str);
#ifdef PDLUA_DEBUG
    post("pdlua_post: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Report an error from a Lua object to Pd's console. */
static int pdlua_error(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \li \c 2 Message string.
  * */
{
    t_pdlua     *o;


    const char  *s;

#ifdef PDLUA_DEBUG
    post("pdlua_error: stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (lua_islightuserdata(L, 1))
    {
        o = lua_touserdata(L, 1);
        if (o)
        {
            s = luaL_checkstring(L, 2);
            if (s) pd_error(o, "%s", s);
            else pd_error(o, "lua: error: null string in error function");
        }
        else error("lua: error: null object in error function");
    }
    else error("lua: error: bad arguments to error function");
#ifdef PDLUA_DEBUG
    post("pdlua_error: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

static void pdlua_setrequirepath
( /* FIXME: documentation (is this of any use at all?) */
    lua_State   *L,
    const char  *path
)
{
#ifdef PDLUA_DEBUG
    post("pdlua_setrequirepath: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_pushstring(L, "_setrequirepath");
    lua_gettable(L, -2);
    lua_pushstring(L, path);
    if (lua_pcall(L, 1, 0, 0) != 0)
    {
        error("lua: internal error in `pd._setrequirepath': %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
#ifdef PDLUA_DEBUG
    post("pdlua_setrequirepath: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
}

static void pdlua_clearrequirepath
( /* FIXME: documentation (is this of any use at all?) */
    lua_State *L
)
{
#ifdef PDLUA_DEBUG
    post("pdlua_clearrequirepath: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    lua_getglobal(L, "pd");
    lua_pushstring(L, "_clearrequirepath");
    lua_gettable(L, -2);
    if (lua_pcall(L, 0, 0, 0) != 0)
    {
        error("lua: internal error in `pd._clearrequirepath': %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
#ifdef PDLUA_DEBUG
    post("pdlua_clearrequirepath: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
}

/** Run a Lua script using Pd's path. */
static int pdlua_dofile(lua_State *L)
/**< Lua interpreter state.
  * \par Inputs:
  * \li \c 1 Pd object pointer.
  * \li \c 2 Filename string.
  * \par Outputs:
  * \li \c * Determined by the script.
  * */
{
    char                buf[MAXPDSTRING];
    char                *ptr;
    t_pdlua_readerdata  reader;
    int                 fd;
    int                 n;
    const char          *filename;
    t_pdlua             *o;

#ifdef PDLUA_DEBUG
    post("pdlua_dofile: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    n = lua_gettop(L);
    if (lua_islightuserdata(L, 1))
    {
        o = lua_touserdata(L, 1);
        if (o)
        {
            filename = luaL_optstring(L, 2, NULL);
            fd = canvas_open(o->canvas, filename, "", buf, &ptr, MAXPDSTRING, 1);
            if (fd >= 0)
            {
#ifdef PDLUA_DEBUG
                post("pdlua_dofile path is %s", buf);
#endif // PDLUA_DEBUG
                //pdlua_setpathname(o, buf);/* change the scriptname to include its path */
                pdlua_setrequirepath(L, buf);
                reader.fd = fd;
                if (lua_load(L, pdlua_reader, &reader, filename))
                {
                    close(fd);
                    pdlua_clearrequirepath(L);
                    lua_error(L);
                }
                else
                {
                    if (lua_pcall(L, 0, LUA_MULTRET, 0))
                    {
                        pd_error(o, "lua: error running `%s':\n%s", filename, lua_tostring(L, -1));
                        lua_pop(L, 1);
                        close(fd);
                        pdlua_clearrequirepath(L);
                    }
                    else
                    {
                        /* succeeded */
                        close(fd);
                        pdlua_clearrequirepath(L);
                    }
                }
            }
            else pd_error(o, "lua: error loading `%s': canvas_open() failed", filename);
        }
        else error("lua: error in object:dofile() - object is null");
    }
    else error("lua: error in object:dofile() - object is wrong type");
    lua_pushstring(L, buf); /* return the path as well so we can open it later with pdlua_menu_open() */
#ifdef PDLUA_DEBUG
    post("pdlua_dofile end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    
    return lua_gettop(L) - n;
}

/** Initialize the pd API for Lua. */
static void pdlua_init(lua_State *L)
/**< Lua interpreter state. */
{
    lua_newtable(L);
    lua_setglobal(L, "pd");
    lua_getglobal(L, "pd");
    lua_pushstring(L, "_iswindows");
#ifdef _WIN32
    lua_pushboolean(L, 1);
#else
    lua_pushboolean(L, 0);
#endif // _WIN32
    lua_settable(L, -3);
    lua_pushstring(L, "_register");
    lua_pushcfunction(L, pdlua_class_new);
    lua_settable(L, -3);
    lua_pushstring(L, "_create");
    lua_pushcfunction(L, pdlua_object_new);
    lua_settable(L, -3);
    lua_pushstring(L, "_createinlets");
    lua_pushcfunction(L, pdlua_object_createinlets);
    lua_settable(L, -3);
    lua_pushstring(L, "_createoutlets");
    lua_pushcfunction(L, pdlua_object_createoutlets);
    lua_settable(L, -3);
    lua_pushstring(L, "_destroy");
    lua_pushcfunction(L, pdlua_object_free);
    lua_settable(L, -3);
    lua_pushstring(L, "_outlet");
    lua_pushcfunction(L, pdlua_outlet);
    lua_settable(L, -3);
    lua_pushstring(L, "_createreceive");
    lua_pushcfunction(L, pdlua_receive_new);
    lua_settable(L, -3);
    lua_pushstring(L, "_receivefree");
    lua_pushcfunction(L, pdlua_receive_free);
    lua_settable(L, -3);
    lua_pushstring(L, "_createclock");
    lua_pushcfunction(L, pdlua_clock_new);
    lua_settable(L, -3);
    lua_pushstring(L, "_clockfree");
    lua_pushcfunction(L, pdlua_clock_free);
    lua_settable(L, -3);
    lua_pushstring(L, "_clockset");
    lua_pushcfunction(L, pdlua_clock_set);
    lua_settable(L, -3);
    lua_pushstring(L, "_clockunset");
    lua_pushcfunction(L, pdlua_clock_unset);
    lua_settable(L, -3);
    lua_pushstring(L, "_clockdelay");
    lua_pushcfunction(L, pdlua_clock_delay);
    lua_settable(L, -3);
    lua_pushstring(L, "_dofile");
    lua_pushcfunction(L, pdlua_dofile);
    lua_settable(L, -3);
    lua_pushstring(L, "send");
    lua_pushcfunction(L, pdlua_send);
    lua_settable(L, -3);
    lua_pushstring(L, "getvalue");
    lua_pushcfunction(L, pdlua_getvalue);
    lua_settable(L, -3);
    lua_pushstring(L, "setvalue");
    lua_pushcfunction(L, pdlua_setvalue);
    lua_settable(L, -3);
    lua_pushstring(L, "_getarray");
    lua_pushcfunction(L, pdlua_getarray);
    lua_settable(L, -3);
    lua_pushstring(L, "_readarray");
    lua_pushcfunction(L, pdlua_readarray);
    lua_settable(L, -3);
    lua_pushstring(L, "_writearray");
    lua_pushcfunction(L, pdlua_writearray);
    lua_settable(L, -3);
    lua_pushstring(L, "_redrawarray");
    lua_pushcfunction(L, pdlua_redrawarray);
    lua_settable(L, -3);
    lua_pushstring(L, "post");
    lua_pushcfunction(L, pdlua_post);
    lua_settable(L, -3);
    lua_pushstring(L, "_error");
    lua_pushcfunction(L, pdlua_error);
    lua_settable(L, -3);
    lua_pop(L, 1);
#ifdef PDLUA_DEBUG
    post("pdlua_init: end. stack top is %d", lua_gettop(L));
#endif // PDLUA_DEBUG
}

/** Pd loader hook for loading and executing Lua scripts. */
static int pdlua_loader
(
    t_canvas    *canvas, /**< Pd canvas to use to find the script. */
    char        *name /**< The name of the script (without .pd_lua extension). */
)
{
    char                dirbuf[MAXPDSTRING];
    char                *ptr;
    int                 fd;
    t_pdlua_readerdata  reader;

#ifdef PDLUA_DEBUG
    post("pdlua_loader: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    fd = canvas_open(canvas, name, ".pd_lua", dirbuf, &ptr, MAXPDSTRING, 1);
    if (fd >= 0)
    {
        class_set_extern_dir(gensym(dirbuf));
        pdlua_setrequirepath(L, dirbuf);
        reader.fd = fd;
        if (lua_load(L, pdlua_reader, &reader, name) || lua_pcall(L, 0, 0, 0))
        {

            error("lua: error loading `%s':\n%s", name, lua_tostring(L, -1));
            lua_pop(L, 1);
            close(fd);
            pdlua_clearrequirepath(L);
            class_set_extern_dir(&s_);
#ifdef PDLUA_DEBUG
            post("pdlua_loader: script error end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
            return 0;
        }
        close(fd);
        pdlua_clearrequirepath(L);
        class_set_extern_dir(&s_);
#ifdef PDLUA_DEBUG
        post("pdlua_loader: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
        return 1;
    }
#ifdef PDLUA_DEBUG
    post("pdlua_loader: no file end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    return 0;
}

/** Start the Lua runtime and register our loader hook. */
#ifdef _WIN32
__declspec(dllexport)
#endif 
void pdlua_setup(void)
{
    char                pd_lua_path[MAXPDSTRING];
    t_pdlua_readerdata  reader;
    int                 fd;
    int                 result;
    char*               pdluaver = "pdlua 0.7.1 (GPL) 2012 Martin Peach, based on";
    char*               luaver = "lua 0.6~svn (GPL) 2008 Claude Heiland-Allen <claudiusmaximus@goto10.org>";
    char                compiled[MAXPDSTRING];

    snprintf(compiled, MAXPDSTRING, "pdlua: compiled for pd-%d.%d on %s %s",
             PD_MAJOR_VERSION, PD_MINOR_VERSION, __DATE__, __TIME__);


#if PD_MAJOR_VERSION==0 && PD_MINOR_VERSION<43
    post(pdluaver);
    post(luaver);
    post(compiled);
#else
    logpost(NULL, 3, pdluaver);
    logpost(NULL, 3, luaver);
    logpost(NULL, 3, compiled);
#endif
    pdlua_proxyinlet_setup();
#ifdef PDLUA_DEBUG
    post("pdlua pdlua_proxyinlet_setup done");
#endif // PDLUA_DEBUG
    pdlua_proxyreceive_setup();
#ifdef PDLUA_DEBUG
    post("pdlua pdlua_proxyreceive_setup done");
#endif // PDLUA_DEBUG
    pdlua_proxyclock_setup();
#ifdef PDLUA_DEBUG
    post("pdlua pdlua_proxyclock_setup done");
#endif // PDLUA_DEBUG
    L = lua_open();
#ifdef PDLUA_DEBUG
    post("pdlua lua_open done L = %p", L);
#endif // PDLUA_DEBUG
    luaL_openlibs(L);
#ifdef PDLUA_DEBUG
    post("pdlua luaL_openlibs done");
#endif // PDLUA_DEBUG
    pdlua_init(L);
#ifdef PDLUA_DEBUG
    post("pdlua pdlua_init done");
#endif // PDLUA_DEBUG
    /* "pd.lua" is the Lua part of pdlua, want to keep the C part minimal */
    /* canvas_open can't find pd.lua unless we give the path to pd beforehand like pd -path /usr/lib/extra/pdlua */
    /* To avoid this we can use c_externdir from m_imp.h, struct _class: t_symbol *c_externdir; */
    /* c_externdir is the directory the extern was loaded from and is also the directory contining pd.lua */
    sprintf(pd_lua_path, "%s/pd.lua", pdlua_proxyinlet_class->c_externdir->s_name); /* the full path to pd.lua */
#ifdef PDLUA_DEBUG
    post("pd_lua_path %s", pd_lua_path);
#endif // PDLUA_DEBUG
    fd = sys_open(pd_lua_path, O_RDONLY);
/*    fd = canvas_open(canvas_getcurrent(), "pd", ".lua", buf, &ptr, MAXPDSTRING, 1);  looks all over and rarely succeeds */
#ifdef PDLUA_DEBUG
    post ("pd.lua loaded from %s", pd_lua_path);
    post("pdlua canvas_open done fd = %d", fd);
    post("pdlua_setup: stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG
    if (fd >= 0)
    { /* pd.lua was opened */
        reader.fd = fd;
        result = lua_load(L, pdlua_reader, &reader, "pd.lua");
#ifdef PDLUA_DEBUG
        post ("pdlua lua_load returned %d", result);
#endif // PDLUA_DEBUG
        if (0 == result)
        {
            result = lua_pcall(L, 0, 0, 0);
#ifdef PDLUA_DEBUG
            post ("pdlua lua_pcall returned %d", result);
#endif // PDLUA_DEBUG
        }
        if (0 != result)
        //if (lua_load(L, pdlua_reader, &reader, "pd.lua") || lua_pcall(L, 0, 0, 0))
        {
            error("lua: error loading `pd.lua':\n%s", lua_tostring(L, -1));
            error("lua: loader will not be registered!");
            error("lua: (is `pd.lua' in Pd's path list?)");
            lua_pop(L, 1);
            close(fd);
        }
        else
        {
            close(fd);
            sys_register_loader(pdlua_loader);
        }
    }
    else
    {
        error("lua: error loading `pd.lua': canvas_open() failed");
        error("lua: loader will not be registered!");
    }
#ifdef PDLUA_DEBUG
    post("pdlua_setup: end. stack top %d", lua_gettop(L));
#endif // PDLUA_DEBUG

}

/* EOF */
