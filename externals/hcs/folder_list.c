/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting file listings using wildcard patterns                  */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2006 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#else
#include <stdlib.h>
#include <glob.h>
#endif

#include <stdio.h>
#include <string.h>

static char *version = "$Revision: 1.12 $";

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *folder_list_class;

typedef struct _folder_list {
	t_object            x_obj;
	t_symbol*           x_pattern;
    t_canvas*           x_canvas;    
} t_folder_list;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

// TODO: make FindFirstFile display when its just a dir

static void normalize_path(t_folder_list* x, char *normalized, const char *original)
{
    char buf[FILENAME_MAX];
    t_symbol *cwd = canvas_getdir(x->x_canvas);
#ifdef _WIN32
    sys_unbashfilename(original, buf);
#else
    strncpy(buf, original, FILENAME_MAX);
#endif
    if(sys_isabsolutepath(buf)) {
        strncpy(normalized, buf, FILENAME_MAX);
        return;
    }
    strncpy(normalized, cwd->s_name, FILENAME_MAX);
    if(normalized[(strlen(normalized)-1)] != '/') {
        strncat(normalized, "/", 1);
    }
    if(buf[0] == '.') {
        if(buf[1] == '/') {
            strncat(normalized, buf + 2, 
                    FILENAME_MAX - strlen(normalized));
        } else if(buf[1] == '.' && buf[2] == '/') {
            strncat(normalized, buf, 
                    FILENAME_MAX - strlen(normalized));
        }
    } else if(buf[0] != '/') {
        strncat(normalized, buf, 
                FILENAME_MAX - strlen(normalized));
    } else {
        strncpy(normalized, buf, FILENAME_MAX);
    }
}

static void folder_list_output(t_folder_list* x)
{
	DEBUG(post("folder_list_output"););
    char normalized_path[FILENAME_MAX] = "";

    normalize_path(x, normalized_path, x->x_pattern->s_name);
#ifdef _WIN32
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	DWORD errorNumber;
	LPVOID lpErrorMessage;
	char fullPathNameBuffer[FILENAME_MAX] = "";
	char unbashBuffer[FILENAME_MAX] = "";
	char outputBuffer[FILENAME_MAX] = "";
	char *pathBuffer;

// arg, looks perfect, but only in Windows Vista
//	GetFinalPathNameByHandle(hFind,fullPathNameBuffer,FILENAME_MAX,FILE_NAME_NORMALIZED);
    GetFullPathName(normalized_path, FILENAME_MAX, fullPathNameBuffer, NULL);
    sys_unbashfilename(fullPathNameBuffer,unbashBuffer);
	
	hFind = FindFirstFile(fullPathNameBuffer, &findData);
	if (hFind == INVALID_HANDLE_VALUE) 
	{
	   errorNumber = GetLastError();
	   switch (errorNumber)
	   {
       case ERROR_FILE_NOT_FOUND:
       case ERROR_PATH_NOT_FOUND:
           pd_error(x,"[folder_list] nothing found for \"%s\"",x->x_pattern->s_name);
           break;
       default:
           FormatMessage(
               FORMAT_MESSAGE_ALLOCATE_BUFFER | 
               FORMAT_MESSAGE_FROM_SYSTEM,
               NULL,
               errorNumber,
               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
               (LPTSTR) &lpErrorMessage,
               0, NULL );
           pd_error(x,"[folder_list] %s", (char *)lpErrorMessage);
	   }
	   return;
	} 
    char* unbashBuffer_position = strrchr(unbashBuffer, '/');
    if(unbashBuffer_position)
    {
        pathBuffer = getbytes(FILENAME_MAX+1);
        strncpy(pathBuffer, unbashBuffer, unbashBuffer_position - unbashBuffer);
    }
	do {
        // skip "." and ".."
        if( strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, "..") ) 
		{
            strncpy(outputBuffer, pathBuffer, FILENAME_MAX);
			strcat(outputBuffer,"/");
			strcat(outputBuffer,findData.cFileName);
			outlet_symbol( x->x_obj.ob_outlet, gensym(outputBuffer) );
		}
	} while (FindNextFile(hFind, &findData) != 0);
	FindClose(hFind);
#else
	unsigned int i;
	glob_t glob_buffer;
	
	DEBUG(post("globbing %s",normalized_path););
	switch( glob( normalized_path, GLOB_TILDE, NULL, &glob_buffer ) )
	{
    case GLOB_NOSPACE: 
        pd_error(x,"[folder_list] out of memory for \"%s\"",normalized_path); 
        break;
# ifdef GLOB_ABORTED
    case GLOB_ABORTED: 
        pd_error(x,"[folder_list] aborted \"%s\"",normalized_path); 
        break;
# endif
# ifdef GLOB_NOMATCH
    case GLOB_NOMATCH: 
        pd_error(x,"[folder_list] nothing found for \"%s\"",normalized_path); 
        break;
# endif
	}
	for(i = 0; i < glob_buffer.gl_pathc; i++)
		outlet_symbol( x->x_obj.ob_outlet, gensym(glob_buffer.gl_pathv[i]) );
	globfree( &(glob_buffer) );
#endif
}


static void folder_list_set(t_folder_list* x, t_symbol *s) 
{
	DEBUG(post("folder_list_set"););
#ifdef _WIN32
    char *patternBuffer;
    char envVarBuffer[FILENAME_MAX];
    if( (s->s_name[0] == '~') && (s->s_name[1] == '/'))
    {
        // TODO this is probably never freed!
        patternBuffer = getbytes(FILENAME_MAX);
        strcpy(patternBuffer,"%USERPROFILE%");
        strncat(patternBuffer, s->s_name + 1, FILENAME_MAX - 1);
        verbose(-1, "set: %s", patternBuffer);
    }
    else
    {
        patternBuffer = s->s_name;
    }
	ExpandEnvironmentStrings(patternBuffer, envVarBuffer, FILENAME_MAX - 2);
	x->x_pattern = gensym(envVarBuffer);
#else  // UNIX
    // TODO translate env vars to a full path
	x->x_pattern = s;
#endif /* _WIN32 */
}


static void folder_list_symbol(t_folder_list *x, t_symbol *s) 
{
   folder_list_set(x,s);
   folder_list_output(x);
}


static void *folder_list_new(t_symbol *s) 
{
	DEBUG(post("folder_list_new"););

	t_folder_list *x = (t_folder_list *)pd_new(folder_list_class);
	t_symbol *currentdir;
	char buffer[MAXPDSTRING];

    x->x_canvas =  canvas_getcurrent();

    symbolinlet_new(&x->x_obj, &x->x_pattern);
	outlet_new(&x->x_obj, &s_symbol);
	
	/* set to the value from the object argument, if that exists */
	if (s != &s_)
	{
		x->x_pattern = s;
	}
	else
	{
		currentdir = canvas_getcurrentdir();
		strncpy(buffer,currentdir->s_name,MAXPDSTRING);
		strncat(buffer,"/*",MAXPDSTRING);
		x->x_pattern = gensym(buffer);
		logpost(x, 3, "setting pattern to default: %s",x->x_pattern->s_name);
	}

	return (x);
}

void folder_list_setup(void) 
{
	DEBUG(post("folder_list_setup"););
	folder_list_class = class_new(gensym("folder_list"), 
								  (t_newmethod)folder_list_new, 
								  0,
								  sizeof(t_folder_list), 
								  0, 
								  A_DEFSYMBOL, 
								  0);
	/* add inlet datatype methods */
	class_addbang(folder_list_class,(t_method) folder_list_output);
	class_addsymbol(folder_list_class,(t_method) folder_list_symbol);
	
	/* add inlet message methods */
	class_addmethod(folder_list_class,(t_method) folder_list_set,gensym("set"), 
					A_DEFSYMBOL, 0);
}

