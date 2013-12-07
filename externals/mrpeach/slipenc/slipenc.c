/* slipenc.c 20100513 Martin Peach */
/* encode a list of bytes as SLIP */
/*
*   From RFC 1055:
*   PROTOCOL
*
*   The SLIP protocol defines two special characters: SLIP_END and SLIP_ESC. SLIP_END is
*   octal 300 (decimal 192) and SLIP_ESC is octal 333 (decimal 219) not to be
*   confused with the ASCII ESCape character; for the purposes of this
*   discussion, SLIP_ESC will indicate the SLIP SLIP_ESC character.  To send a
*   packet, a SLIP host simply starts sending the data in the packet.  If
*   a data byte is the same code as SLIP_END character, a two byte sequence of
*   SLIP_ESC and octal 334 (decimal 220) is sent instead.  If it the same as
*   an SLIP_ESC character, an two byte sequence of SLIP_ESC and octal 335 (decimal
*   221) is sent instead.  When the last byte in the packet has been
*   sent, an SLIP_END character is then transmitted.
*
*   Phil Karn suggests a simple change to the algorithm, which is to
*   begin as well as end packets with an SLIP_END character.  This will flush
*   any erroneous bytes which have been caused by line noise.  In the
*   normal case, the receiver will simply see two back-to-back SLIP_END
*   characters, which will generate a bad IP packet.  If the SLIP
*   implementation does not throw away the zero-length IP packet, the IP
*   implementation certainly will.  If there was line noise, the data
*   received due to it will be discarded without affecting the following
*   packet.
*
*   Because there is no 'standard' SLIP specification, there is no real
*   defined maximum packet size for SLIP.  It is probably best to accept
*   the maximum packet size used by the Berkeley UNIX SLIP drivers: 1006
*   bytes including the IP and transport protocol headers (not including
*   the framing characters).  Therefore any new SLIP implementations
*   should be prepared to accept 1006 byte datagrams and should not send
*   more than 1006 bytes in a datagram.
*/

#include "m_pd.h" 

/* -------------------------- slipenc -------------------------- */
#ifndef _SLIPCODES
/* SLIP special character codes */
#define SLIP_END        0300 /* indicates end of packet */
#define SLIP_ESC        0333 /* indicates byte stuffing */
#define SLIP_ESC_END    0334 /* SLIP_ESC SLIP_ESC_END means SLIP_END data byte */
#define SLIP_ESC_ESC    0335 /* SLIP_ESC SLIP_ESC_ESC means SLIP_ESC data byte */
#define MAX_SLIP        1006 /* maximum SLIP packet size */
#define _SLIPCODES
#endif // _SLIPCODES

static t_class *slipenc_class;

typedef struct _slipenc
{
    t_object    x_obj;
    t_outlet    *x_slipenc_out;
    t_atom      *x_slip_buf;
    t_int       x_slip_length;
    t_int       x_slip_max_length;
} t_slipenc;

static void *slipenc_new(t_symbol *s, int argc, t_atom *argv);
static void slipenc_list(t_slipenc *x, t_symbol *s, int ac, t_atom *av);
static void slipenc_free(t_slipenc *x);
void slipenc_setup(void);

static void *slipenc_new(t_symbol *s, int argc, t_atom *argv)
{
    int i, max_len;
    t_slipenc  *x = (t_slipenc *)pd_new(slipenc_class);

    if (x == NULL) return x;
    
    x->x_slip_max_length = MAX_SLIP; // default unless float argument given
    for (i = 0; i < argc; ++i)
    {
        if (argv[i].a_type == A_FLOAT)
        {
            max_len = atom_getfloat(&argv[i]);
            if (max_len > 3)
            {
                x->x_slip_max_length = max_len;
                post("slipenc: maximum packet length is %d", x->x_slip_max_length);
            }
            else
                post("slipenc: maximum packet length must be greater than 3, using %d", x->x_slip_max_length);
            break;
        }
    }
    x->x_slip_buf = (t_atom *)getbytes(sizeof(t_atom)*x->x_slip_max_length);
    if(x->x_slip_buf == NULL)
    {
        error("slipenc: unable to allocate %lu bytes for x_slip_buf", (long)sizeof(t_atom)*x->x_slip_max_length);
        return NULL;
    }
    /* Initialize all the slip buf atoms to float type */
    for (i = 0; i < x->x_slip_max_length; ++i) x->x_slip_buf[i].a_type = A_FLOAT;
    x->x_slipenc_out = outlet_new(&x->x_obj, &s_list);
    return (x);
}

static void slipenc_list(t_slipenc *x, t_symbol *s, int ac, t_atom *av)
{
    /* SLIP encode a list of bytes */
    float   f;
    int     i, c;

    i = 0;    
    /* for each byte in the packet, send the appropriate character sequence */
    while (i < ac)
    {
        x->x_slip_length = 0;
        /* send an initial SLIP_END character to flush out any data that may */
        /* have accumulated in the receiver due to line noise */
        x->x_slip_buf[x->x_slip_length++].a_w.w_float = SLIP_END;

        while((i < ac)&&(x->x_slip_length < (x->x_slip_max_length-2)))
        {
            /* check each atom for byteness */
            f = atom_getfloat(&av[i++]);
            c = (((int)f) & 0x0FF);
            if (c != f)
            {
                /* abort, bad input character */
                pd_error (x, "slipenc: input %f out of range [0..255]", f);
                return;
            }
            if(SLIP_END == c)
            {
                /* If it's the same code as a SLIP_END character, replace it with a */
                /* special two-character code so as not to make the receiver think we sent SLIP_END */
                x->x_slip_buf[x->x_slip_length++].a_w.w_float = SLIP_ESC;
                x->x_slip_buf[x->x_slip_length++].a_w.w_float = SLIP_ESC_END;
            }
            else if (SLIP_ESC == c)
            {
                /* If it's the same code as a SLIP_ESC character, replace it with a special two-character code */
                /* so as not to make the receiver think we sent SLIP_ESC */
                x->x_slip_buf[x->x_slip_length++].a_w.w_float = SLIP_ESC;
                x->x_slip_buf[x->x_slip_length++].a_w.w_float = SLIP_ESC_ESC;
            }
            else
            {
                /* Otherwise, pass the character */
                x->x_slip_buf[x->x_slip_length++].a_w.w_float = c;
            }
        }
        /* Add the SLIP_END code to tell the receiver that the packet is complete */
        x->x_slip_buf[x->x_slip_length++].a_w.w_float = SLIP_END;
        outlet_list(x->x_slipenc_out, &s_list, x->x_slip_length, x->x_slip_buf);
    }
}

static void slipenc_free(t_slipenc *x)
{
    if (x->x_slip_buf != NULL) freebytes((void *)x->x_slip_buf, sizeof(t_atom)*x->x_slip_max_length);
}

void slipenc_setup(void)
{
    slipenc_class = class_new(gensym("slipenc"), 
        (t_newmethod)slipenc_new, (t_method)slipenc_free,
        sizeof(t_slipenc), 0, A_GIMME, 0);
    class_addlist(slipenc_class, slipenc_list);
}

/* fin slipenc.c*/
