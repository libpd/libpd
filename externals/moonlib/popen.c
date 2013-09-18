/*
Copyright (C) 2002 Antoine Rousseau

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#define BUFSIZE 4096

typedef struct popen
{
    t_object x_obj;
    FILE *x_file;
    char x_data[BUFSIZE];
    int x_count;
    int x_ropened;
    t_outlet *x_msgout;
} t_popen;

t_class *popen_class;
static t_binbuf *inbinbuf;


static void *popen_new(t_symbol *s, int argc, t_atom *argv)
{
    t_popen *x;

    x = (t_popen *)pd_new(popen_class);

    x->x_file = NULL;
    x->x_count=0;
    x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    x->x_ropened=0;
    return (x);
}

static void popen_close(t_popen *x)
{
    if(x->x_ropened)
    {
        sys_rmpollfn(fileno(x->x_file));
        //fflush(x->x_file);
    }
    if(x->x_file) pclose(x->x_file);
    x->x_file=0;
    x->x_ropened=0;
}

static void popen_ff(t_popen *x)
{

}

static void popen_open(t_popen *x, t_symbol *s,int argc, t_atom *argv)
{
    char cmd[512],*text;
    int cmd_len;

    t_binbuf *bb=binbuf_new();

    popen_close(x);

    //post("argc=%d",argc);
    //post("argv[0]=%s",atom_getsymbol(&argv[0])->s_name);

    binbuf_add(bb,argc,argv);
    binbuf_gettext(bb, &text,&cmd_len);
    binbuf_free(bb);

    strncpy(cmd,text,cmd_len);
    cmd[cmd_len]=0;
    //post("cmd=%s",cmd);

    x->x_file=popen(cmd,"w");

}

static void popen_list(t_popen *x, t_symbol *s,
                       int argc, t_atom *argv)
{
    t_binbuf *bb;
    char *buf;
    int l;

    if(!x->x_file) return;

    bb=binbuf_new();
    binbuf_add(bb,argc,argv);
    //binbuf_print(bb);
    binbuf_gettext(bb, &buf,&l);
    buf[l]=0;

    //printf("popen list: %s\n",buf);
    fprintf(x->x_file,"%s\n",buf);
    fflush(x->x_file);

    freebytes(buf,l);
    binbuf_free(bb);
}

static void popen_out(t_popen *x, t_binbuf *b)
{
    t_atom messbuf[1024];
    int msg, natom = binbuf_getnatom(b);
    t_atom *at = binbuf_getvec(b);
    for (msg = 0; msg < natom;)
    {
        int emsg;
        for (emsg = msg; emsg < natom && at[emsg].a_type != A_COMMA
                && at[emsg].a_type != A_SEMI; emsg++)
            ;
        if (emsg > msg)
        {
            int i;
            for (i = msg; i < emsg; i++)
                if (at[i].a_type == A_DOLLAR || at[i].a_type == A_DOLLSYM)
                {
                    pd_error(x, "popen: got dollar sign in message");
                    goto nodice;
                }
            if (at[msg].a_type == A_FLOAT)
            {
                if (emsg > msg + 1)
                    outlet_list(x->x_msgout, 0, emsg-msg, at + msg);
                else outlet_float(x->x_msgout, at[msg].a_w.w_float);
            }
            else if (at[msg].a_type == A_SYMBOL)
                outlet_anything(x->x_msgout, at[msg].a_w.w_symbol,
                                emsg-msg-1, at + msg + 1);
        }
nodice:
        msg = emsg + 1;
    }
}

static void popen_read(t_popen *x,int fd)
{
    int len,i=0;
    unsigned char b;
    unsigned char buffer[BUFSIZE];

    if((len=read(fd,buffer,BUFSIZE))> 0)
    {

        for(i=0; i<len; i++)
        {
            if(x->x_count>=BUFSIZE) x->x_count=0;
            x->x_data[x->x_count++]=buffer[i];
            if(buffer[i]==';')
            {
                binbuf_text(inbinbuf, x->x_data, x->x_count);
                x->x_count=0;
                popen_out(x,inbinbuf);
            }
        }
    }
}

static void popen_ropen(t_popen *x, t_symbol *s,int argc, t_atom *argv)
{
    char cmd[512],*text;
    int cmd_len;

    t_binbuf *bb=binbuf_new();

    popen_close(x);

    //post("argc=%d",argc);
    //post("argv[0]=%s",atom_getsymbol(&argv[0])->s_name);

    binbuf_add(bb,argc,argv);
    binbuf_gettext(bb, &text,&cmd_len);
    binbuf_free(bb);

    strncpy(cmd,text,cmd_len);
    cmd[cmd_len]=0;
    //post("cmd=%s",cmd);

    x->x_file=popen(cmd,"r");
    sys_addpollfn(fileno(x->x_file),(t_fdpollfn)popen_read,(void *)x);
    x->x_ropened=1;
}

void popen_setup(void )
{
    inbinbuf = binbuf_new();
    popen_class = class_new(gensym("popen"), (t_newmethod)popen_new,
                            (t_method)popen_close,sizeof(t_popen), 0, A_GIMME, 0);
    class_addmethod(popen_class, (t_method)popen_close,
                    gensym("close"), 0);
    class_addmethod(popen_class, (t_method)popen_open,
                    gensym("open"), A_GIMME, 0);
    class_addmethod(popen_class, (t_method)popen_ropen,
                    gensym("ropen"), A_GIMME, 0);
    class_addlist(popen_class, popen_list);

}

