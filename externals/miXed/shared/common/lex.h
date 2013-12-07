/* Copyright (c) 2003-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __LEX_H__
#define __LEX_H__

typedef struct _lex
{
    FILE           *l_fp;
    unsigned char  *l_buf;
    int             l_bufsize;
    int             l_bufndx;
    t_atomtype      l_inttype;
    t_atomtype      l_lasttype;
    int             l_errbinary;
} t_lex;

int lex_nextatom(t_lex *lx, t_atom *ap);
void lex_atomstring(t_atom *ap, char *buf, int bufsize, t_atomtype inttype);
int lex_isbinary(t_lex *lx);
void lex_free(t_lex *lx);
t_lex *lex_new(FILE *fp, t_atomtype inttype);

#endif
