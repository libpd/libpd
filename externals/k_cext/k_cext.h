/*
 *   k_cext.h copyright 2002 Kjetil S. Matheussen.
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <stdbool.h>
#ifdef _MSC_VER
typedef int bool;
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif

#include <stdarg.h>

#ifdef MSW
#define K_EXTERN __declspec(dllexport) extern
#else
#define K_EXTERN extern
#endif

typedef struct k_cext
{
  t_object x_ob;
  t_float *values;

  int num_ins;
  int num_outs;

  t_inlet **inlets;
  t_outlet **outlets;

  void (*k_cext_process)(struct k_cext *x);
  void *handle;

  char filename[40];

  bool iscext;

  void *userdata; // This attribute /can/ be used by a patch, but using "static" is much cleaner, so please don't.
} t_k_cext;


/* The following functions are used by intsort and floatsort */
K_EXTERN int k_cext_intcompare(const void *p1, const void *p2);
K_EXTERN int k_cext_floatcompare(const void *p1, const void *p2);


/* The following functions are system dependant, and called internally from k_cext only.
   All ports must implement these functions.
 */

K_EXTERN int k_sys_getprocessfunction(t_k_cext *x,char *funcname,char *name);
K_EXTERN void k_sys_freehandle(t_k_cext *x);
K_EXTERN void k_sys_mktempfilename(char *to);
K_EXTERN void k_sys_writeincludes(FILE *file);
K_EXTERN void k_sys_makecompilestring(char *to,char *name,char *funcname);
K_EXTERN void k_sys_deletefile(char *name);
K_EXTERN void k_sys_init(void);

/* TB value accessing functions*/
K_EXTERN t_float k_cext_getvalue(char c[]);
K_EXTERN int k_cext_setvalue(char c[],float f);


#define V(a) (x->values[a])
#define I(a) ((int)(x->values[a]))


#define O(a,b) outlet_float(x->outlets[a],b)

#define O0(b) O(0,b)
#define O1(b) O(1,b)
#define O2(b) O(2,b)
#define O3(b) O(3,b)
#define O4(b) O(4,b)
#define O5(b) O(5,b)
#define O6(b) O(6,b)

#define BETWEEN(dasmin,dasmax) ((dasmin) + (((float)(dasmax-(dasmin)))*rand())/(RAND_MAX+1.0))
#define RANDOM(dasmax) BETWEEN(0,dasmax)

#define SEND(symname,val) \
do{ \
  static t_symbol *k_cext_internal_symbol=NULL; \
  if(k_cext_internal_symbol==NULL) k_cext_internal_symbol=gensym(symname); \
  if(k_cext_internal_symbol->s_thing) pd_float(k_cext_internal_symbol->s_thing, val);  \
}while(0)

#define SEN(symname,val) SEND(symname,val)

#define INTARRAY(name,len) int name[len]={0}
#define FLOATARRAY(name,len) t_float name[len]={0.0f}

#define INTSORT(a,b) qsort((void *)(a),b, sizeof (int), k_cext_intcompare);
#define FLOATSORT(a,b) qsort((void *)(a),b, sizeof (float), k_cext_floatcompare);

/* TB: values and bang outlets */
#define Ob(a) outlet_bang(x->outlets[a]);
float k_cext_getvalue(char c[]);
int k_cext_setvalue(char c[],float f);
#define VALUE(char) k_cext_getvalue(char)
#define SETVALUE(char,float) k_cext_setvalue(char,float)


#define IF if(
#define FOR for(
#define RANGE(a,b,c) for(a=b;a<c;a++)
#define WHILE while(
#define SWITCH switch(

#define THEN )
#define BEGIN {

#define LOOP for(;;){

#define ELIF }else if(

/* If you think "END ELSE BEGIN" is more natural, just write "END else BEGIN". */
#define ELSE }else{

#define END }
#define ENDFOR END
#define ENDRANGE END
#define ENDIF END
#define ENDWHILE END
#define ENDLOOP END
#define ENDSWITCH END

#define SC ;

#define NL "\n"
  
#define SP " "
  //#define STRING(a) " " ## a ## " "

  //#define gakk system("echo");

typedef int (*k_cext_f_int_callback)(t_k_cext *x,...);
typedef float (*k_cext_f_float_callback)(t_k_cext *x,...);

#ifndef _MSC_VER
static k_cext_f_int_callback *k_cext_int_funcs[];
static k_cext_f_float_callback *k_cext_float_funcs[];
static t_k_cext **k_cext_int_x[];
static t_k_cext **k_cext_float_x[];
#else
k_cext_f_int_callback *k_cext_int_funcs[];
k_cext_f_float_callback *k_cext_float_funcs[];
t_k_cext **k_cext_int_x[];
t_k_cext **k_cext_float_x[];
#endif

bool k_cext_get_int_funcs(k_cext_f_int_callback **funcs,t_k_cext **xs[],int length,...);
bool k_cext_get_float_funcs(k_cext_f_float_callback **funcs,t_k_cext **xs[],int length,...);


#define INT_(b,a) (*k_cext_int_funcs[a])
#define I_(a) *k_cext_int_x[a]


/* 
   The alternative suggested by Thomas Grill might be better:

#define INT_0(b,a) (*k_cext_int_funcs[a])(*k_cext_int_x[a])
#define INT_1(b,a,c) (*k_cext_int_funcs[a])(*k_cext_int_x[a],c)
#define INT_2(b,a,c,d) (*k_cext_int_funcs[a])(*k_cext_int_x[a],c,d)
#define INT_3(b,a,c,e) (*k_cext_int_funcs[a])(*k_cext_int_x[a],c,d,e)
#define INT_4(b,a,c,e,f) (*k_cext_int_funcs[a])(*k_cext_int_x[a],c,d,e,f)
...
 */


/* VC is reported not to be an iso99 c compiler, so the following very nice macro can't be used. #$¥5¥@{@{@$¥@ !!! */
/* #define F_INT(a,...) (*k_cext_int_funcs[a])(*k_cext_int_x[a],__VA_ARGS__) */


#define FLOAT_(b,a) (*k_cext_float_funcs[a])
#define F_(a) *k_cext_float_x[a]

/* Same here. */
/* #define F_FLOAT(a,...) (*k_cext_float_funcs[a])(*k_cext_float_x[a],__VA_ARGS__) */

