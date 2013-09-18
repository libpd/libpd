
#include "tof.h"
#define SLASH '/'

static char path_buf_a[MAXPDSTRING];
static char path_buf_b[MAXPDSTRING];

static t_class *path_class;

typedef struct _path {
  t_object  x_obj;
  //char       buffer[MAXPDSTRING];
  t_outlet* outlet1;
  //t_canvas* canvas;
  t_symbol* dir;
  t_symbol* mode;
  t_symbol* dirmode;
  t_symbol* s_current;
  t_symbol* s_relative;
} t_path;



static int path_is_absolute(char *dir, int length)
{
    if ((length && dir[0] == '/') || (dir[0] == '~')
#ifdef MSW
        || dir[0] == '%' || (length > 2 && (dir[1] == ':' && dir[2] == '/'))
#endif
        )
    {
        return 1;
    } else {
        return 0;            
    }
}


// Code from http://www.codeguru.com/cpp/misc/misc/fileanddirectorynaming/article.php/c263#more
static void getRelativeFilename(char* relativeFilename, char *currentDirectory, char *absoluteFilename)
{
	// declarations - put here so this should work in a C compiler
	int afMarker = 0, rfMarker = 0;
	int cdLen = 0, afLen = 0;
	int i = 0;
	int levels = 0;
	//static char relativeFilename[MAX_FILENAME_LEN+1];

	cdLen = strlen(currentDirectory);
	afLen = strlen(absoluteFilename);


	// Handle DOS names that are on different drives:
	if(currentDirectory[0] != absoluteFilename[0])
	{
		// not on the same drive, so only absolute filename will do
		strcpy(relativeFilename, absoluteFilename);
		return;
	}

	// they are on the same drive, find out how much of the current directory
	// is in the absolute filename
	i = 1;//ABSOLUTE_NAME_START;
	while(i < afLen && i < cdLen && currentDirectory[i] == absoluteFilename[i])
	{
		i++;
	}

	if(i == cdLen && (absoluteFilename[i] == SLASH || absoluteFilename[i-1] == SLASH))
	{
		// the whole current directory name is in the file name,
		// so we just trim off the current directory name to get the
		// current file name.
		if(absoluteFilename[i] == SLASH)
		{
			// a directory name might have a trailing slash but a relative
			// file name should not have a leading one...
			i++;
		}

		strcpy(relativeFilename, &absoluteFilename[i]);
		return;
	}


	// The file is not in a child directory of the current directory, so we
	// need to step back the appropriate number of parent directories by
	// using "..\"s.  First find out how many levels deeper we are than the
	// common directory
	afMarker = i;
	levels = 1;

	// count the number of directory levels we have to go up to get to the
	// common directory
	while(i < cdLen)
	{
		i++;
		if(currentDirectory[i] == SLASH)
		{
			// make sure it's not a trailing slash
			i++;
			if(currentDirectory[i] != '\0')
			{
				
				levels++;
			}
		}
	}
	
    
    
	// move the absolute filename marker back to the start of the directory name
	// that it has stopped in.
	while(afMarker > 0 && absoluteFilename[afMarker-1] != SLASH)
	{
		afMarker--;
	}

	

	// add the appropriate number of "..\"s.
	rfMarker = 0;
	for(i = 0; i < levels; i++)
	{
		relativeFilename[rfMarker++] = '.';
		relativeFilename[rfMarker++] = '.';
		relativeFilename[rfMarker++] = SLASH;
	}

	// copy the rest of the filename into the result string
	strcpy(&relativeFilename[rfMarker], &absoluteFilename[afMarker]);

	//return relativeFilename;
}


// BANG: output the current root path
static void path_bang(t_path *x)
{
		
	outlet_symbol(x->outlet1, x->dir  );
	
	 
}

static void path_symbol(t_path *x, t_symbol *s) {
	
	
	strcpy(path_buf_a,s->s_name);
	int length = strlen(path_buf_a);
	t_symbol* result;
	
	if ( x->mode == x->s_relative ) {
		
		// TRANSFORM ABSOLUTE PATHS TO RELATIVE PATHS
		
		// Is this a relative path?
		// Checks for a starting / or a : as second character
		if ( path_is_absolute(path_buf_a,length  )) {
			getRelativeFilename(path_buf_b,x->dir->s_name,path_buf_a);
			result = gensym(path_buf_b);
		} else {
			result = gensym(path_buf_a);
		}
		
	} else {
		
		// TRANFORM RELATIVE PATHS TO ABSOLUTE PATHS
		
		// Is this a relative path?
		if ( path_is_absolute(path_buf_a,length)  ) {
			result = gensym(path_buf_a);
		} else {
			// remove extra ../
			strcpy(path_buf_b,x->dir->s_name);
			char* dir = path_buf_b;
			char* p;
			int l = strlen(dir);
			if (dir[l-1] == '/') dir[l-1] = '\0';
			char* file = path_buf_a;
			while( strncmp(file,"../",3) == 0 ) {
				file = file + 3;
				p = strrchr(dir,'/');
				if (p) *p = '\0';
			}
			
			strcat(dir,"/");
			strcat(dir,file);
			result = gensym(dir);
		}
	}	

	outlet_symbol(x->outlet1, result);
}






static void path_free(t_path *x)
{
		
  //binbuf_free(x->binbuf);
    
}

void *path_new(t_symbol *s, int argc, t_atom *argv)
{
  t_path *x = (t_path *)pd_new(path_class);
    
  
    x->s_current = gensym("current");
	x->s_relative = gensym("relative");
	
	int i;
	t_symbol * mode_temp;
	for ( i = 0; i < argc; i++) {
		if ( IS_A_SYMBOL(argv,i)  ) {
			mode_temp = atom_getsymbol(argv+i);
			if ( mode_temp == x->s_current ) x->dirmode = x->s_current;
			if ( mode_temp == x->s_relative) x->mode = x->s_relative;
	    }
	}
	
	if ( x->dirmode == x->s_current) {
	x->dir = tof_get_dir(tof_get_canvas());
	} else {
	  x->dir = tof_get_dir(tof_get_root_canvas(tof_get_canvas()));
	  
	}
	strcpy(path_buf_a,x->dir->s_name);
	
	//strcat(path_buf_a,"/");
	x->dir = gensym(path_buf_a);
	
  /*
   inlet_new(&x->x_obj, &x->x_obj.ob_pd,
        gensym("float"), gensym("set"));

  floatinlet_new(&x->x_obj, &x->inc);
 */  

  x->outlet1 = outlet_new(&x->x_obj, &s_float);
  
  

  return (void *)x;
}

void path_setup(void) {
  path_class = class_new(gensym("path"),
        (t_newmethod)path_new,
        (t_method)path_free, sizeof(t_path),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang(path_class, path_bang);
  class_addsymbol(path_class,path_symbol);
  
  //class_addmethod(path_class, 
   //     (t_method)path_append, gensym("append"),
    //    A_GIMME, 0);
  
  //class_addfloat (path_class, path_float);
  /*class_addmethod(path_class, 
        (t_method)path_set, gensym("set"),
        A_DEFFLOAT, 0);
*/
  //class_addlist (path_class, path_list);
  
}
