/* k_cext_win.c. Windows part of k_cext. Made by Kjetil Matheussen and Olaf Matthes. */
//
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include <io.h>
#include <windows.h>

#include "m_pd.h"
#include "k_cext.h"

#define MAX_INSTANCES 256

char *nametemplate = "pd-XXXXXX";
extern int instancenumber;	/* number of instance of this object */
char names[MAX_INSTANCES][9];


int k_sys_getprocessfunction(t_k_cext *x, char *funcname, char *name){
  char dllname[50];
  strcpy(dllname, name);
  sys_bashfilename(dllname, dllname);
  dllname[strlen(dllname)-4]=0;
  strcat(dllname, ".dll");
  post("going to load %s...", dllname);
  x->handle = LoadLibrary(dllname);

  if (!x->handle){
    x->handle=NULL;
    post("Error (%d). \n", GetLastError());
    return 0;
  }
  // x->k_cext_process = (t_xxx)GetProcAddress(x->handle, "k_cext_process");  
  x->k_cext_process=(void (*)(struct k_cext *))GetProcAddress(x->handle, funcname);
  return 1;
}

void k_sys_freehandle(t_k_cext *x){
  char temp[500];

  FreeLibrary((HINSTANCE)x->handle);

  sprintf(temp,"%s.dll",x->filename);
  unlink(temp);
  sprintf(temp,"%s.obj",x->filename);
  unlink(temp);
  sprintf(temp,"%s.exp",x->filename);
  unlink(temp);
  sprintf(temp,"%s.lib",x->filename);
  unlink(temp);

  unlink(x->filename);
}

void k_sys_mktempfilename(char *to){
  char *temp;
  FILE *file;
  strcpy(names[instancenumber], nametemplate);
  temp = _mktemp(names[instancenumber]);
  if(temp == NULL)
  {
    error("could not create unique filename");
	return;
  }
  sprintf(to,"%s",temp);
  file=fopen(temp,"w");	/* we have to cheat ;-( */
  fclose(file);
}

void k_sys_writeincludes(FILE *file){
  fprintf(file,"#include \"" INCLUDEPATH "\\src\\m_pd.h\"\n");
  fprintf(file,"#include \"" INCLUDEPATH "\\src\\k_cext.h\"\n");	/* needs to be in pd/src, sorry.. */
}

void k_sys_makecompilestring(char *to,char *name,char *funcname){
  sprintf(to,"cl %s " INCLUDEPATH "\\bin\\pd.lib " INCLUDEPATH "\\extra\\k_cext.lib /LD /Gd /GD /Ox /DNT /link /export:%s",
	  name, funcname);
}

void k_sys_deletefile(char *name){
  char delname[16];
	post("del %s", name);
  _unlink(name);

}

void k_sys_init(void){
}
