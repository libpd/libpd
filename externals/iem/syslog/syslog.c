/******************************************************
 *
 * syslog - implementation file
 *
 * copyright (c) 2006 IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2 or later
 *
 ******************************************************/

/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License*                 */
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
/* Foundation, Inc.,                                                            */
/* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                  */
/*                                                                              */

/*
  syslog :  syslog-code for message-objects
*/


#include <syslog.h>
#include <string.h>

#include "m_pd.h"

/* ------------------------- syslog ------------------------------- */

/*
MESSAGE SYSLOG: simple and easy
*/

static t_class *syslog_class;

typedef struct _syslog
{
  t_object x_obj;
  t_float x_level;
} t_syslog;


static void syslog_anything(t_syslog *x, t_symbol*s, t_int argc, t_atom*argv)
{
  int level=x->x_level;
  char *result = 0;
  int pos=0, i=0;
  t_atom*ap;
  int length=0;

  /* 1st get the length of the symbol */
  if(s)length+=strlen(s->s_name);
  else length-=1;
  length+=argc;

  i=argc;
  ap=argv;
  while(i--){
    char buffer[MAXPDSTRING];
    int len=0;
    if(A_SYMBOL==ap->a_type){
      len=strlen(ap->a_w.w_symbol->s_name);
    } else {
      atom_string(ap, buffer, MAXPDSTRING);
      len=strlen(buffer);
    }
    length+=len;
    ap++;
  }

  if(length<=0)return;

  result = (char*)getbytes((length+1)*sizeof(char));

  if (s) {
    char *buf = s->s_name;
    strcpy(result+pos, buf);
    pos+=strlen(buf);
    if(i){
      strcpy(result+pos, " ");
      pos += 1;
    }
  }

  ap=argv;
  i=argc;
  while(i--){
    if(A_SYMBOL==ap->a_type){
      strcpy(result+pos, ap->a_w.w_symbol->s_name);
      pos+= strlen(ap->a_w.w_symbol->s_name);
    } else {
      char buffer[MAXPDSTRING];
      atom_string(ap, buffer, MAXPDSTRING);
      strcpy(result+pos, buffer);
      pos += strlen(buffer);
    }
    ap++;
    if(i){
      strcpy(result+pos, " ");
      pos++;
    }
  }

  result[length]=0;
  if(result){
    syslog(level, "%s", result);
    freebytes(result, length+1);
  }
    
}
static void syslog_free(t_syslog *x)
{
  closelog();
}

static void *syslog_new(t_symbol*s)
{
  t_syslog *x = (t_syslog *)pd_new(syslog_class);
  x->x_level=LOG_NOTICE;
  if(s&&s!=&s_){
    openlog(s->s_name, 0, LOG_USER);
  } else {
    openlog("puredata", 0, LOG_USER);
  }

  floatinlet_new(&x->x_obj, &x->x_level);
  return (x);
}


void syslog_setup(void)
{
  syslog_class = class_new(gensym("syslog"), (t_newmethod)syslog_new, 
                           (t_method)syslog_free, sizeof(t_syslog), 
                           0, 
                           A_DEFSYM, A_NULL);
  
  class_addanything(syslog_class, syslog_anything);
}
