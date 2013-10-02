/* k_cext_macosx.c. MacosX part of k_cext. Made by Kjetil Matheussen. Never tested. */
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

#include "m_pd.h"
#include "k_cext.h"

#include <unistd.h>

#include <mach-o/dyld.h> 

int k_sys_getprocessfunction(t_k_cext *x,char *funcname,char *name){
  NSObjectFileImage image; 
  void *ret;
  NSSymbol s; 

  x->k_cext_process=NULL;
  x->handle=NULL;

  if ( NSCreateObjectFileImageFromFile( name, &image) != NSObjectFileImageSuccess ){
    post("Error. \n");
    return 0;
  }
  ret=NSLinkModule( image, name, NSLINKMODULE_OPTION_BINDNOW); 
  s = NSLookupSymbolInModule(ret,funcname); 
  
  if(s){
    x->k_cext_process = (t_xxx)NSAddressOfSymbol( s);
  }else{
    return 0;
  }

  return 1;
}

void k_sys_freehandle(t_k_cext *x){
  post("k_cext_macosx.c/k_sys_freehandle: FIX ME.\n");
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
  sprintf(to,"gcc -Wall -O2 %s -o %s.o -shared",name,name);
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
