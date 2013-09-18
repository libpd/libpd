/* Copyright (c) 1997-2004 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef MIXED_STANDALONE
#include "unstable/standalone.h"
#else
#include "m_pd.h"
#endif
#include "lex.h"

static int lex_nextbyte(t_lex *lx, unsigned char *buf)
{
    int ich;
    if (lx->l_fp)
    {
	if ((ich = fgetc(lx->l_fp)) == EOF)
	    return (0);
    }
    else if (lx->l_buf)
    {
	if (lx->l_bufndx < lx->l_bufsize)
	    ich = lx->l_buf[lx->l_bufndx++];
	else
	    return (0);
    }
    else return (0);
    if (ich)
    {
	*buf = (unsigned char)ich;
	return (1);
    }
    else
    {
	lx->l_errbinary = 1;
	return (0);
    }
}

static void lex_ungetbyte(t_lex *lx, unsigned char ch)
{
    if (lx->l_fp)
    {
	ungetc(ch, lx->l_fp);
    }
    else if (lx->l_buf)
    {
	if (lx->l_bufndx > 0)
	    lx->l_buf[--lx->l_bufndx] = ch;
    }
}

/* single pass of binbuf_text(), optionally int-preserving version */
int lex_nextatom(t_lex *lx, t_atom *ap)
{
    char buf[1001], *bufp, *ebuf = buf + 1000;
    int ready;
    unsigned char ch;
    ap->a_type = A_NULL;
    while ((ready = lex_nextbyte(lx, &ch)) &&
	   (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t'));
    if (!ready)
    {
	/* ??? */
	if (lx->l_lasttype == A_SEMI)
	    return (0);
	else
	    ap->a_type = A_SEMI;
    }
    else if (ch == ';')
	ap->a_type = A_SEMI;
    else if (ch == ',')
	ap->a_type = A_COMMA;
    else
    {
	int floatstate = 0, slash = 0, lastslash = 0, firstslash = (ch == '\\');
	bufp = buf;
	do
	{
	    *bufp = ch;
	    lastslash = slash;
	    slash = (ch == '\\');

	    if (floatstate >= 0)
	    {
		int digit = (ch >= '0' && ch <= '9'),
		    dot = (ch == '.'), minus = (ch == '-'),
		    plusminus = (minus || (ch == '+')),
		    expon = (ch == 'e' || ch == 'E');
		if (floatstate == 0)    /* beginning */
		{
		    if (minus) floatstate = 1;
		    else if (digit) floatstate = 2;
		    else if (dot) floatstate = 3;
		    else floatstate = -1;
		}
		else if (floatstate == 1)	/* got minus */
		{
		    if (digit) floatstate = 2;
		    else if (dot) floatstate = 3;
		    else floatstate = -1;
		}
		else if (floatstate == 2)	/* got digits */
		{
		    if (dot) floatstate = 4;
		    else if (expon) floatstate = 6;
		    else if (!digit) floatstate = -1;
		}
		else if (floatstate == 3)	/* got '.' without digits */
		{
		    if (digit) floatstate = 5;
		    else floatstate = -1;
		}
		else if (floatstate == 4)	/* got '.' after digits */
		{
		    if (digit) floatstate = 5;
		    else if (expon) floatstate = 6;
		    else floatstate = -1;
		}
		else if (floatstate == 5)	/* got digits after . */
		{
		    if (expon) floatstate = 6;
		    else if (!digit) floatstate = -1;
		}
		else if (floatstate == 6)	/* got 'e' */
		{
		    if (plusminus) floatstate = 7;
		    else if (digit) floatstate = 8;
		    else floatstate = -1;
		}
		else if (floatstate == 7)	/* got plus or minus */
		{
		    if (digit) floatstate = 8;
		    else floatstate = -1;
		}
		else if (floatstate == 8)	/* got digits */
		{
		    if (!digit) floatstate = -1;
		}
	    }
	    if (!slash) bufp++;
	}
	while ((ready = lex_nextbyte(lx, &ch)) && bufp != ebuf
	       && (slash || (ch != ' ' && ch != '\n' && ch != '\r'
			     && ch != '\t' && ch != ',' && ch != ';')));
	if (ready && (ch == ',' || ch == ';'))
	    lex_ungetbyte(lx, ch);
	*bufp = 0;
#if 0
	fprintf(stderr, "buf %s\n", buf);
#endif
	if (*buf == '$' && buf[1] >= '0' && buf[1] <= '9' && !firstslash)
	{
	    for (bufp = buf+2; *bufp; bufp++)
	    {
		if (*bufp < '0' || *bufp > '9')
		{
		    ap->a_type = A_DOLLSYM;
		    ap->a_w.w_symbol = gensym(buf+1);
		    break;
		}
	    }
	    if (ap->a_type == A_NULL)
	    {
		ap->a_type = A_DOLLAR;
		ap->a_w.w_index = atoi(buf+1);
	    }
	}
	else if (floatstate == 2)
	{
	    if (lx->l_inttype == A_FLOAT)
	    {
		ap->a_type = A_FLOAT;
		ap->a_w.w_float = (float)atof(buf);
	    }
	    else
	    {
		ap->a_type = lx->l_inttype;
		ap->a_w.w_index = atoi(buf);
	    }
	}
	else if (floatstate == 4 || floatstate == 5 || floatstate == 8)
	{
	    ap->a_type = A_FLOAT;
	    ap->a_w.w_float = (float)atof(buf);
	}
	else
	{
	    ap->a_type = A_SYMBOL;
	    ap->a_w.w_symbol = gensym(buf);
	}
    }
    lx->l_lasttype = ap->a_type;
    return (1);
}

void lex_atomstring(t_atom *ap, char *buf, int bufsize, t_atomtype inttype)
{
    char *sp, *bp, *ep;
    switch(ap->a_type)
    {
    case A_SEMI:
	strcpy(buf, ";"); break;
    case A_COMMA:
	strcpy(buf, ","); break;
    case A_FLOAT:
	sprintf(buf, "%#f", ap->a_w.w_float);
	ep = buf + strlen(buf) - 1;
	while (ep > buf && *ep == '0') *ep-- = 0;
	break;
    case A_SYMBOL:
    	sp = ap->a_w.w_symbol->s_name;
	bp = buf;
	ep = buf + (bufsize-5);
	while (bp < ep && *sp)
	{
	    if (*sp == ';' || *sp == ',' || *sp == '\\' ||
		(*sp == '$' && bp == buf && sp[1] >= '0' && sp[1] <= '9'))
		*bp++ = '\\';
	    if ((unsigned char)*sp < 127)
		*bp++ = *sp++;
	    else
		/* FIXME this is temporary -- codepage horror */
		sprintf(bp, "\\%.3o", (unsigned char)*sp++), bp += 4;
	}
	if (*sp) *bp++ = '*';
	*bp = 0;
	break;
    case A_DOLLAR:
    	sprintf(buf, "$%d", ap->a_w.w_index);
    	break;
    case A_DOLLSYM:
    	sprintf(buf, "$%s", ap->a_w.w_symbol->s_name);
    	break;
    default:
	if (ap->a_type == inttype)
	    sprintf(buf, "%d", ap->a_w.w_index);
	else
	{
#ifdef MIXED_STANDALONE
	    fprintf(stderr, "BUG (lex): bad atom type\n");
#else
	    bug("lex_atomstring (bad atom type)");
#endif
	    strcpy(buf, "???");
	}
    }
}

int lex_isbinary(t_lex *lx)
{
    return (lx->l_errbinary);
}

void lex_free(t_lex *lx)
{
    freebytes(lx, sizeof(*lx));
}

t_lex *lex_new(FILE *fp, t_atomtype inttype)
{
    t_lex *lx = (t_lex *)getbytes(sizeof(*lx));
    lx->l_fp = fp;
    lx->l_buf = 0;  /* FIXME */
    lx->l_inttype = inttype;
    lx->l_lasttype = A_SEMI;
    lx->l_errbinary = 0;
    return (lx);
}
