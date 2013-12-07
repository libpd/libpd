/* Copyright (c) 1997-2005 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER verify endianness transparency */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define BINPORT_MAXSTRING  1000
#define BINPORT_SYMGROW      64

#ifndef MIXED_STANDALONE
/* load a max binary file into a Pd binbuf */

#include "m_pd.h"

#else
/* make a max-textual listing from a max binary file */

/* This is a standalone version of a ``max binary to binbuf'' module.
   It uses certain Pd calls and structs, which are duplicated in the
   "standalone" module defined in shared/unstable.
   LATER standalone binport should be linked to the Pd API library. */

#include "unstable/standalone.h"

#ifdef KRZYSZCZ
//#define BINPORT_DEBUG
#endif
#define BINPORT_VERBOSE
#endif

#include "common/lex.h"
#include "binport.h"

static void binport_error(char *fmt, ...)
{
    char buf[BINPORT_MAXSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
#ifdef MIXED_STANDALONE
    fprintf(stderr, "ERROR (binport): %s\n", buf);
#else
    post("ERROR (binport): %s", buf);
#endif
    va_end(ap);
}

static void binport_warning(char *fmt, ...)
{
#if defined (MIXED_STANDALONE) || defined(BINPORT_VERBOSE)
    char buf[BINPORT_MAXSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
#ifdef MIXED_STANDALONE
    fprintf(stderr, "warning (binport): %s\n", buf);
#else
    post("warning (binport): %s", buf);
#endif
    va_end(ap);
#endif
}

static void binport_bug(char *fmt, ...)
{
    char buf[BINPORT_MAXSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
#ifdef MIXED_STANDALONE
    fprintf(stderr, "BUG (binport): %s\n", buf);
#else
    bug("(binport) %s", buf);
#endif
    va_end(ap);
}

static void binport_failure(char *filename)
{
    binport_error("\"%s\" doesn't look like a patch file", filename);
}

static void binpold_failure(char *filename)
{
    binport_error("tried reading \"%s\" as an old format file, but failed",
		  filename);
}

enum {
    BINPORT_NULLTYPE,
    BINPORT_INTTYPE = 1, BINPORT_FLOATTYPE, BINPORT_SYMTYPE,
    BINPORT_DEFINTTYPE = 5, BINPORT_DEFFLOATTYPE, BINPORT_DEFSYMTYPE,
    BINPORT_DEFDOLLSYMTYPE = 9,
    BINPORT_SEMITYPE = 10, BINPORT_COMMATYPE,
    BINPORT_DOLLARTYPE, BINPORT_DOLLSYMTYPE
};

/* We use A_INT atom type not only for listing, but for import too --
   the parser passes ints to individual token handlers, so that any
   required conversion has to be done during Pd message generation. */
#define A_INT  A_DEFFLOAT

static int binport_readbuf(FILE *fp, char *buf, size_t sz)
{
    return (fread(buf, 1, sz, fp) == sz ? sz : 0);
}

static int binport_readbyte(FILE *fp, unsigned char *buf)
{
    int c;
    if ((c = fgetc(fp)) == EOF)
	return (0);
    *buf = (unsigned char)c;
    return (1);
}

static int binport_readint(FILE *fp, int *iptr)
{
    unsigned char word[4];
    if (fread(word, 1, 4, fp) == 4)
    {
	*iptr = ((word[0] << 24) | (word[1] << 16) | (word[2] << 8) | word[3]);
	return (4);
    }
    else return (0);
}

/* LATER more testing */
/* make it binpold_readfloat() */
static int binport_readfloat(FILE *fp, float *fptr)
{
    unsigned char word[10];
    if (fread(word, 1, 10, fp) == 10)
    {
	int ex;
	unsigned hi, lo;
	ex = ((word[0] & 0x7F) << 8) | word[1];
	hi = ((unsigned)word[2] << 24) | ((unsigned)word[3] << 16) |
	    ((unsigned)word[4] << 8) | (unsigned)word[5];
	lo = ((unsigned)word[6] << 24) | ((unsigned)word[7] << 16) |
	    ((unsigned)word[8] << 8) | (unsigned)word[9];
	if (ex == 0x7FFF)
	{
	    binport_warning("NaN atom bashed to zero");
	    *fptr = 0.;
	}
	else if (ex || hi || lo)
	{
	    double dhi, dlo, dabs;
	    ex -= 0x401e;
	    dhi = (double)((hi - 0x7fffffff) - 1) + ((float)0x7fffffff + 1.);
	    dlo = (double)((lo - 0x7fffffff) - 1) + ((float)0x7fffffff + 1.);
	    dabs  = ldexp(dhi, ex) + ldexp(dlo, ex - 32);
	    *fptr = ((word[0] & 0x80) ? -(float)dabs : (float)dabs);
	}
	else *fptr = 0.;
#ifdef BINPORT_DEBUG
	fprintf(stderr, "%02x%02x", (int)word[0], (int)word[1]);
	fprintf(stderr, " %02x%02x%02x%02x",
		(int)word[2], (int)word[3], (int)word[4], (int)word[5]);
	fprintf(stderr, " %02x%02x%02x%02x",
		(int)word[6], (int)word[7], (int)word[8], (int)word[9]);
	fprintf(stderr, " == %g\n", *fptr);
#endif
	return (10);
    }
    else return (0);
}

static int binport_readstring(FILE *fp, char *buf)
{
    int c, i = 1;
    while (c = fgetc(fp))
    {
	if (c == EOF)
	    return (0);
	if (++i < BINPORT_MAXSTRING)
	    *buf++ = (unsigned char)c;
    }
    *buf = '\0';
    if (i >= BINPORT_MAXSTRING)
	binport_warning("symbol string too long, skipped");
    return (i);
}

typedef struct _binpold
{
    FILE    *o_fp;
    int      o_natoms;
    int      o_bodysize;
    int      o_nsymbols;
    int      o_symbolid;
    int      o_ndx;
    t_atom  *o_atombuf;
} t_binpold;

#define BINPOLD_NATOMTYPES     16
#define BINPOLD_MAXATOMS  1000000  /* LATER rethink */

static t_atomtype binpold_atomtypes[BINPOLD_NATOMTYPES] = {
    A_NULL, A_INT, A_FLOAT, A_SYMBOL,
    A_CANT, A_CANT, A_CANT, A_CANT, A_CANT, A_CANT,
    A_SEMI, A_COMMA, A_DOLLAR, A_CANT, A_CANT, A_CANT
};

static int binpold_gettype(t_binpold *old, t_atom *ap)
{
    int typecode;
    if ((typecode = fgetc(old->o_fp)) != EOF)
    {
	if (typecode > 0 && typecode < BINPOLD_NATOMTYPES)
	{
	    ap->a_type = binpold_atomtypes[typecode];
	    if (ap->a_type != A_CANT)
		return (1);
	    else binport_warning("unsupported type of atom %d: %d",
				 old->o_ndx, typecode);
	}
	else binport_warning("bad type of atom %d: %d", old->o_ndx, typecode);
    }
    else binport_warning("failed reading type of atom %d", old->o_ndx);
    return (0);
}

static int binpold_getvalue(t_binpold *old, t_atom *ap, int *countp)
{
    int ival;
    float fval;
    *countp = 0;
    switch (ap->a_type)
    {
    case A_INT:
    case A_SYMBOL:
	if (*countp = binport_readint(old->o_fp, &ival))
	    ap->a_w.w_index = ival;
	else
	    goto valuefailed;
	if (ap->a_type == A_SYMBOL)
	{
	    if (ival >= old->o_nsymbols)
		old->o_nsymbols = ival + 1;
	    ap->a_type = A_DEFSYM;  /* invalidate, until w_symbol is known */
	}
	break;
    case A_FLOAT:
	if (*countp = binport_readfloat(old->o_fp, &fval))
	    ap->a_w.w_float = fval;
	else
	    goto valuefailed;
	break;
    case A_SEMI:
    case A_COMMA:
	break;
    case A_DOLLAR:
	if (*countp = binport_readint(old->o_fp, &ival))
	    ap->a_w.w_index = ival;
	else
	    goto valuefailed;
	break;
    default:
	goto valuefailed;
    }
    return (1);
valuefailed:
    binport_warning("failed reading value of atom %d (type %d)",
		    old->o_ndx, ap->a_type);
    return (0);
}

static int binpold_load(t_binpold *old)
{
    char buf[BINPORT_MAXSTRING];
    t_atom *ap;
    int total;
#ifdef BINPORT_DEBUG
    fprintf(stderr, "old format: %d atoms, %d-byte chunk of atom values\n",
	    old->o_natoms, old->o_bodysize);
#endif
    for (old->o_ndx = 0, ap = old->o_atombuf;
	 old->o_ndx < old->o_natoms; old->o_ndx++, ap++)
	if (!binpold_gettype(old, ap))
	    return (0);
    old->o_nsymbols = 0;
    total = 0;
    for (old->o_ndx = 0, ap = old->o_atombuf;
	 old->o_ndx < old->o_natoms; old->o_ndx++, ap++)
    {
	int count;
	if (!binpold_getvalue(old, ap, &count))
	    return (0);
	total += count;
    }
    if (total != old->o_bodysize)
    {
	binport_warning("actual chunk size %d inconsistent with declared %d",
			total, old->o_bodysize);
	return (0);
    }
    for (old->o_symbolid = 0;
	 old->o_symbolid < old->o_nsymbols; old->o_symbolid++)
    {
	if (binport_readstring(old->o_fp, buf))
	{
	    t_symbol *s = gensym(buf);
	    for (old->o_ndx = 0, ap = old->o_atombuf;
		 old->o_ndx < old->o_natoms; old->o_ndx++, ap++)
	    {
		if (ap->a_type == A_DEFSYM &&
		    ap->a_w.w_index == old->o_symbolid)
		{
		    ap->a_type = A_SYMBOL;
		    ap->a_w.w_symbol = s;
		}
	    }
	}
	else
	{
	    binport_warning("failed reading string for symbol %d",
			    old->o_symbolid);
	    return (0);
	}
    }
    for (old->o_ndx = 0, ap = old->o_atombuf;
	 old->o_ndx < old->o_natoms; old->o_ndx++, ap++)
    {
	if (ap->a_type == A_DEFSYM)
	{
	    binport_warning("unknown string for symbol %d", ap->a_w.w_index);
	    return (0);
	}
	else if (ap->a_type == A_DOLLAR)
	{
	    sprintf(buf, "#%d", ap->a_w.w_index);
	    ap->a_type = A_SYMBOL;
	    ap->a_w.w_symbol = gensym(buf);
	}
	/* CHECKME A_DOLLSYM */
    }
    return (1);
}

static int binpold_nextatom(t_binpold *old, t_atom *ap)
{
    if (old->o_ndx < old->o_natoms)
    {
	*ap = old->o_atombuf[old->o_ndx++];
	return (1);
    }
    else return (0);
}

static void binpold_free(t_binpold *old)
{
    if (old->o_fp)
	fclose(old->o_fp);
    if (old->o_atombuf)
	freebytes(old->o_atombuf, old->o_natoms * sizeof(*old->o_atombuf));
    freebytes(old, sizeof(*old));
}

static t_binpold *binpold_new(FILE *fp)
{
    int natoms, bodysize;
    if (binport_readint(fp, &natoms))
    {
	if (natoms < 0 || natoms > BINPOLD_MAXATOMS)
	    binport_warning("bad number of atoms: %d", natoms);
	else if (binport_readint(fp, &bodysize))
	{
	    if (bodysize < 0)
		binport_warning("negative chunk size: %d", bodysize);
	    else
	    {
		t_binpold *old = getbytes(sizeof(*old));
		old->o_fp = fp;
		old->o_natoms = natoms;
		old->o_bodysize = bodysize;
		if (!(old->o_atombuf =
		      getbytes(old->o_natoms * sizeof(*old->o_atombuf))))
		{
		    binport_error("could not allocate %d atoms", old->o_natoms);
		    freebytes(old, sizeof(*old));
		    fclose(fp);
		    return (0);
		}
		return (old);
	    }
	}
    }
    else binport_warning("file too short");
    fclose(fp);
    return (0);
}

typedef struct _binport
{
    FILE       *b_fp;
    int         b_ftype;
    int         b_nsymbols;
    int         b_symsize;
    t_symbol  **b_symtable;
    t_binpold  *b_old;
    t_lex      *b_lex;
} t_binport;

static void binport_setint(t_atom *ap, int i)
{
    ap->a_type = A_INT;
    ap->a_w.w_index = i;
}

static void binport_setfloat(t_atom *ap, float f)
{
    ap->a_type = A_FLOAT;
    ap->a_w.w_float = f;
}

static void binport_setsymbol(t_atom *ap, t_symbol *s)
{
    ap->a_type = A_SYMBOL;
    ap->a_w.w_symbol = s;
}

static t_symbol *binport_makesymbol(t_binport *bp, int id)
{
    char s[BINPORT_MAXSTRING];
    if (id < bp->b_nsymbols)
	binport_bug("symbol id mismatch");
    else if (id > bp->b_nsymbols)
	binport_error("unexpected symbol id");
    else if (binport_readstring(bp->b_fp, s))
    {
	int reqsize = ++bp->b_nsymbols;
	if (reqsize > bp->b_symsize)
	{
	    reqsize += (BINPORT_SYMGROW - 1);
#ifdef BINPORT_DEBUG
	    binport_warning("resizing symbol table to %d elements", reqsize);
#endif
	    if (bp->b_symtable =
		resizebytes(bp->b_symtable,
			    bp->b_symsize * sizeof(*bp->b_symtable),
			    reqsize * sizeof(*bp->b_symtable)))
		bp->b_symsize = reqsize;
	    else
	    {
		bp->b_nsymbols = bp->b_symsize = 0;
		return (0);
	    }
	}
	return (bp->b_symtable[id] = gensym(s));
    }
    return (0);
}

static int binport_setbysymtable(t_binport *bp, t_atom *ap, int id)
{
    t_symbol *s;
    if (id < bp->b_nsymbols)
	s = bp->b_symtable[id];
    else
	s = binport_makesymbol(bp, id);
    if (s)
    {
	ap->a_type = A_SYMBOL;
	ap->a_w.w_symbol = s;
    }
    return (s != 0);
}

static int binport_nextatom(t_binport *bp, t_atom *ap)
{
    unsigned char opcode;
    int opval;
    char buf[64];

    if (bp->b_ftype == BINPORT_MAXTEXT && bp->b_lex)
	return (lex_nextatom(bp->b_lex, ap));
    else if (bp->b_ftype == BINPORT_MAXOLD && bp->b_old)
	return (binpold_nextatom(bp->b_old, ap));

    if (!binport_readbyte(bp->b_fp, &opcode))
	goto badbin;
    opval = opcode & 0x0f;
    switch (opcode >> 4)
    {
    case BINPORT_INTTYPE:  /* variable length int,
			      opval: length (number of bytes that follow) */
	if (!binport_readbuf(bp->b_fp, buf, opval))
	    goto badbin;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    if (opcode == 0x12)  /* FIXME */
		i = (short)i;
	    binport_setint(ap, i);
	}
	break;
    case BINPORT_FLOATTYPE:  /* variable length float,
				opval: length (number of bytes that follow) */
	if (!binport_readbuf(bp->b_fp, buf, opval))
	    goto badbin;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    binport_setfloat(ap, *(t_float *)&i);
	}
	break;
    case BINPORT_SYMTYPE:  /* variable length symbol id,
			      opval: length (number of bytes that follow) */
	if (!binport_readbuf(bp->b_fp, buf, opval))
	    goto badbin;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    if (!binport_setbysymtable(bp, ap, i))
		goto badbin;
	}
	break;
    case BINPORT_DEFINTTYPE:  /* half-byte int */
	binport_setint(ap, opval);
	break;
    case BINPORT_DEFSYMTYPE:  /* half-byte symbol id */
	if (!binport_setbysymtable(bp, ap, opval))
	    goto badbin;
	break;
    case BINPORT_DEFDOLLSYMTYPE:  /* half-byte #symbol id */
	if (!binport_setbysymtable(bp, ap, opval))
	    goto badbin;
	sprintf(buf, "#%s", ap->a_w.w_symbol->s_name);
#ifdef BINPORT_DEBUG
	binport_warning(buf);
#endif
	ap->a_w.w_symbol = gensym(buf);
	break;
    case BINPORT_SEMITYPE:
	/* LATER warn about nonzero opval */
	ap->a_type = A_SEMI;
	break;
    case BINPORT_COMMATYPE:
	/* CHECKME apparently never used? */
	binport_warning("found the comma type in max binary...");
	/* LATER warn about nonzero opval */
	ap->a_type = A_COMMA;
	break;
    case BINPORT_DOLLARTYPE:  /* #number */
	sprintf(buf, "#%d", opval);
	ap->a_type = A_SYMBOL;
	ap->a_w.w_symbol = gensym(buf);
	break;
    case BINPORT_DOLLSYMTYPE:  /* #symbol id,
				  opval: length (number of bytes that follow) */
	if (!binport_readbuf(bp->b_fp, buf, opval))
	    goto badbin;
	else
	{
	    unsigned char *p = (unsigned char *)buf + opval;
	    int i = 0;
	    while (opval--) i = (i << 8) | *--p;
	    if (!binport_setbysymtable(bp, ap, i))
		goto badbin;
	}
	sprintf(buf, "#%s", ap->a_w.w_symbol->s_name);
#ifdef BINPORT_DEBUG
	binport_warning(buf);
#endif
	ap->a_w.w_symbol = gensym(buf);
	break;
    default:
	binport_error("unknown opcode %x", (int)opcode);
	goto badbin;
    }
    return (1);
badbin:
    return (0);
}

static int binport_alike(char *header, int *ftypep)
{
    static char bin_header[4] = { 2, 0, 0, 0 };  /* CHECKME any others? */
    static char old_header[4] = { 0, 0, 0, 1 };  /* CHECKME any others? */
    static char text_header[4] = { 'm', 'a', 'x', ' ' };
    static char pd_header[3] = { '#', 'N', ' ' };  /* canvas or struct */
    if (memcmp(header, bin_header, 4) == 0)
	*ftypep = BINPORT_MAXBINARY;
    else if (memcmp(header, text_header, 4) == 0)
	*ftypep = BINPORT_MAXTEXT;
    else if (memcmp(header, old_header, 4) == 0)
	*ftypep = BINPORT_MAXOLD;
    else
    {
	if (memcmp(header, pd_header, 3) == 0)
	    *ftypep = BINPORT_PDFILE;
	else
	    *ftypep = BINPORT_INVALID;
	return (0);
    }
    return (1);
}

static void binport_free(t_binport *bp)
{
    fclose(bp->b_fp);
    if (bp->b_symtable)
	freebytes(bp->b_symtable, bp->b_symsize * sizeof(*bp->b_symtable));
    if (bp->b_old)
    {
	bp->b_old->o_fp = 0;
	binpold_free(bp->b_old);
    }
    if (bp->b_lex)
    {
	bp->b_lex->l_fp = 0;
	lex_free(bp->b_lex);
    }
    freebytes(bp, sizeof(*bp));
}

static t_binport *binport_new(FILE *fp, int *ftypep)
{
    t_binport *bp = 0;
    char header[4];
    if (fread(header, 1, 4, fp) == 4)
    {
	int alike = binport_alike(header, ftypep);
	if (alike)
	{
	    bp = getbytes(sizeof(*bp));
	    bp->b_fp = fp;
	    bp->b_ftype = *ftypep;
	    bp->b_nsymbols = 0;
	    if (*ftypep == BINPORT_MAXBINARY)
	    {
		bp->b_symsize = BINPORT_SYMGROW;
		bp->b_symtable =
		    getbytes(bp->b_symsize * sizeof(*bp->b_symtable));
	    }
	    else
	    {
		bp->b_symsize = 0;
		bp->b_symtable = 0;
	    }
	    bp->b_old = 0;
	    bp->b_lex = 0;
	}
	else if (*ftypep != BINPORT_PDFILE)
	    binport_warning("unknown header: %02x%02x%02x%02x",
			    (int)header[0], (int)header[1],
			    (int)header[2], (int)header[3]);
    }
    else
    {
	binport_warning("file too short");
	*ftypep = BINPORT_INVALID;
    }
    if (!bp) fclose(fp);
    return (bp);
}

static void binport_print(t_binport *bp, FILE *fp)
{
    char buf[BINPORT_MAXSTRING];
    t_atom at;
    int cnt = 0;
    if (bp->b_old)
	bp->b_old->o_ndx = 0;
    while (binport_nextatom(bp, &at))
    {
	if (at.a_type == A_SEMI)
	{
	    fputs(";\n", fp);
	    cnt = 0;
	}
	else if (at.a_type != A_NULL)
	{
	    if (cnt++) fputc(' ', fp);
	    lex_atomstring(&at, buf, BINPORT_MAXSTRING, A_INT);
	    fputs(buf, fp);
	}
    }
}

#ifndef MIXED_STANDALONE

static int binport_tobinbuf(t_binport *bp, t_binbuf *bb)
{
    t_atom at;
    if (bp->b_old)
	bp->b_old->o_ndx = 0;
    while (binport_nextatom(bp, &at))
	if (at.a_type != A_NULL)
	    binbuf_add(bb, 1, &at);
    return (1);
}

/* LATER deal with corrupt binary files? */
int binport_read(t_binbuf *bb, char *filename, char *dirname)
{
    int result;
    FILE *fp;
    char namebuf[MAXPDSTRING];
    namebuf[0] = 0;
    if (*dirname)
    	strcat(namebuf, dirname), strcat(namebuf, "/");
    strcat(namebuf, filename);
    sys_bashfilename(namebuf, namebuf);
    if (fp = fopen(namebuf, "rb"))
    {
	int ftype;
	t_binport *bp = binport_new(fp, &ftype);
	if (bp)
	{
	    if (ftype == BINPORT_MAXBINARY)
		result = (binport_tobinbuf(bp, bb)
			  ? BINPORT_MAXBINARY : BINPORT_CORRUPT);
	    else if (ftype == BINPORT_MAXTEXT)
	    {
		t_atom at;
		if (bp->b_lex = lex_new(fp, A_INT))
		{
		    while (binport_nextatom(bp, &at))
			if (at.a_type == A_SEMI)
			    break;
		    binbuf_addv(bb, "ss;", gensym("max"), gensym("v2"));
		    result = (binport_tobinbuf(bp, bb)
			      ? BINPORT_MAXTEXT : BINPORT_CORRUPT);
		}
		else result = BINPORT_FAILED;
	    }
	    else if (ftype == BINPORT_MAXOLD)
	    {
		t_binpold *old = binpold_new(fp);
		result = BINPORT_FAILED;
		if (old)
		{
		    bp->b_old = old;
		    if (binpold_load(old) && binport_tobinbuf(bp, bb))
			result = BINPORT_MAXOLD;
		}
		else binpold_failure(filename);
	    }
	    else result = BINPORT_FAILED;
	    binport_free(bp);
	}
	else if (ftype == BINPORT_PDFILE)
	    result = (binbuf_read(bb, filename, dirname, 0)
		      ? BINPORT_FAILED : BINPORT_PDFILE);
	else
	{
	    binport_failure(filename);
	    result = BINPORT_INVALID;
	}
    }
    else
    {
	binport_bug("cannot open file");
	result = BINPORT_FAILED;
    }
    return (result);
}

/* save as MAXTEXT */
void binport_write(t_binbuf *bb, char *filename, char *dirname)
{
    int result;
    FILE *fp;
    char namebuf[MAXPDSTRING];
    namebuf[0] = 0;
    if (*dirname)
    	strcat(namebuf, dirname), strcat(namebuf, "/");
    strcat(namebuf, filename);
    sys_bashfilename(namebuf, namebuf);
    if (fp = fopen(namebuf, "w"))
    {
	char buf[BINPORT_MAXSTRING];
	t_atom *ap = binbuf_getvec(bb);
	int cnt = 0, ac = binbuf_getnatom(bb);
	while (ac--)
	{
	    if (ap->a_type == A_SEMI)
	    {
		fputs(";\n", fp);
		cnt = 0;
	    }
	    else if (ap->a_type != A_NULL)
	    {
		if (cnt++) fputc(' ', fp);
		lex_atomstring(ap, buf, BINPORT_MAXSTRING, A_INT);
		fputs(buf, fp);
	    }
	    ap++;
	}
	fclose(fp);
    }
}

#else

int main(int ac, char **av)
{
    if (ac > 1)
    {
	FILE *fp = fopen(av[1], "rb");
	if (fp)
	{
	    int ftype;
	    t_binport *bp = binport_new(fp, &ftype);
	    if (bp)
	    {
		if (ftype == BINPORT_MAXBINARY)
		    binport_print(bp, stdout);
		else if (ftype == BINPORT_MAXTEXT)
		    binport_warning("\"%s\" looks like a Max text file", av[1]);
		else if (ftype == BINPORT_MAXOLD)
		{
		    t_binpold *old = binpold_new(fp);
		    if (old)
		    {
			bp->b_old = old;
			if (binpold_load(old))
			    binport_print(bp, stdout);
			else
			    ftype = BINPORT_FAILED;
		    }
		    else ftype = BINPORT_FAILED;
		    if (ftype == BINPORT_FAILED) binpold_failure(av[1]);
		}
		binport_free(bp);
	    }
	    else if (ftype == BINPORT_PDFILE)
		binport_warning("\"%s\" looks like a Pd patch file", av[1]);
	    else
		binport_failure(av[1]);
	}
	else binport_error("cannot open file \"%s\"", av[1]);
    }
    else binport_error("what file?");
    return (0);
}

#endif
