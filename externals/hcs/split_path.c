/* (C) Guenter Geiger <geiger@epy.co.at> */

/* I started with stripdir.c and turned it into split_path.c <hans@eds.org> */

#include <m_pd.h>
#include <string.h>
#include <sys/param.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

static char *version = "$Revision: 1.1 $";

/* ------------------------ split_path ----------------------------- */

static t_class *split_path_class;

typedef struct _split_path
{
    t_object   x_obj;
	t_outlet   *x_path_outlet;
	t_outlet   *x_filename_outlet;
} t_split_path;


void split_path_symbol(t_split_path *x, t_symbol *s)
{
	int length = strlen(s->s_name);
	char path_buffer[MAXPATHLEN] = "";
		
	while (length--)
		if (*(s->s_name + length) == '/') 
			break;
	if (length < MAXPATHLEN)
		outlet_symbol(x->x_filename_outlet,gensym(s->s_name + length + 1));
	else 
		error("[split_path] filename name too long. The limit is %d characters",MAXPATHLEN);
	while (length > 0)
	{
		length--;
		if(*(s->s_name + length) != '/') break;
	}
	if (length < MAXPATHLEN)
	{
		strncpy(path_buffer, s->s_name, length + 1);
		outlet_symbol(x->x_path_outlet,gensym(path_buffer));
	}
	else
	{
		error("[split_path] path name too long. The limit is %d characters",MAXPATHLEN);
	}
}

static void *split_path_new()
{
    t_split_path *x = (t_split_path *)pd_new(split_path_class);
	x->x_path_outlet = (t_outlet *)outlet_new(&x->x_obj, &s_symbol);
	x->x_filename_outlet = (t_outlet *)outlet_new(&x->x_obj, &s_symbol);
	return (x);
}

void split_path_setup(void)
{
    split_path_class = class_new(gensym("split_path"), (t_newmethod)split_path_new, 0,
				sizeof(t_split_path), 0,0);
    class_addsymbol(split_path_class,split_path_symbol);
    logpost(NULL, 4, "[split_path] %s",version);  
    logpost(NULL, 4, "\twritten by Hans-Christoph Steiner <hans@eds.org>");
}


