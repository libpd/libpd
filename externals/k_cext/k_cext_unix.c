/* k_cext_unix.c. unix part of k_cext. Made by Kjetil Matheussen. Never tested. */
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

#include <m_pd.h>
#include "k_cext.h"
#include <dlfcn.h>

#include <unistd.h>
#include <signal.h>


int k_sys_getprocessfunction(t_k_cext *x,char *funcname,char *name){

  x->handle=dlopen(name,RTLD_NOW|RTLD_GLOBAL);

  if(x->handle!=NULL){
    x->k_cext_process=(void (*)(struct k_cext *))dlsym(x->handle,funcname);
    return 1;
  }
  return 0;
}

void k_sys_freehandle(t_k_cext *x){
  char temp[500];
  dlclose(x->handle);

  sprintf(temp,"%s.c.o",x->filename);
  unlink(temp);
}

void k_sys_mktempfilename(char *to){
  sprintf(to,"/tmp/pd-k_cext-XXXXXX");
  mktemp(to);
}

void k_sys_writeincludes(FILE *file){
  fprintf(file,"#include \"" INCLUDEPATH "/" LINUXINCLUDE "/m_pd.h\"\n");
  fprintf(file,"#include \"" INCLUDEPATH  "/k_cext.h\"\n");
}

void k_sys_makecompilestring(char *to,char *name,char *funcname){
  sprintf(to,"gcc -Wall -O2 %s -o %s.o -shared -fPIC",name,name);
}

void k_sys_deletefile(char *name){
  unlink(name);
}

static void k_sys_cleanup(void){
  system("rm -fr /tmp/pd-k_cext-*");
}

static void finish(int sig){
  k_sys_cleanup();
  exit(0);
}


void k_sys_init(void){
  atexit(k_sys_cleanup);
  signal(SIGINT,finish);
}
