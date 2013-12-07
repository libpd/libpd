/* packxbee outputs a list of floats which are the bytes making up an xbee api packet. */
/* The packet can then be sent through [comport]. */
/* Started by Martin Peach 20110731 */
/* Information taken from "XBee®/XBee-PRO® ZB RF Modules" (document 90000976_G, 11/15/2010)*/
/* by Digi International Inc. http://www.digi.com */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "pdxbee.h"

static t_class *packxbee_class;


typedef struct _packxbee
{
    t_object        x_obj;
    t_outlet        *x_listout;
    int             x_api_mode;
    unsigned char   x_frameID;
    unsigned char   x_frameType;
    int             x_verbosity;
    t_atom          x_outbuf[MAX_XBEE_PACKET_LENGTH];
} t_packxbee;

static void *packxbee_new(t_floatarg f);
static int packxbee_outbuf_add(t_packxbee *x, int index, unsigned char val);
static void packxbee_AT(t_packxbee *x, t_symbol *s, int argc, t_atom *argv);
static void packxbee_RAT(t_packxbee *x, t_symbol *s, int argc, t_atom *argv);
static void packxbee_ATQ(t_packxbee *x, t_symbol *s, int argc, t_atom *argv);
static void packxbee_TX(t_packxbee *x, t_symbol *s, int argc, t_atom *argv);
static void packxbee_pack_remote_frame(t_packxbee *x, t_symbol *s, int argc, t_atom *argv);
static void packxbee_pack_frame(t_packxbee *x, t_symbol *s, int argc, t_atom *argv);
static void packxbee_API(t_packxbee *x, t_float api);
static void packxbee_verbosity(t_packxbee *x, t_float verbosity_level);
static void packxbee_free(t_packxbee *x);
void packxbee_setup(void);

static void *packxbee_new(t_floatarg f)
{
    int i;

    t_packxbee *x = (t_packxbee *)pd_new(packxbee_class);
    if (x)
    {
        x->x_listout = outlet_new(&x->x_obj, &s_list);
        if (1 == f) x->x_api_mode = 1;
        else x->x_api_mode = 2; /* default to escaped mode */
        x->x_verbosity = 0; /* debug level */
        for(i = 0; i < MAX_XBEE_PACKET_LENGTH; ++i) x->x_outbuf[i].a_type = A_FLOAT; /* init output atoms as floats */
    }
    return (x);
}

static void packxbee_API(t_packxbee *x, t_float api)
{
    if ((api == 1) || (api ==2)) x->x_api_mode = api;
    else error ("packxbee: api mode must be 1 or 2");
}

static void packxbee_verbosity(t_packxbee *x, t_float verbosity_level)
{
    if (verbosity_level >= 0) x->x_verbosity = verbosity_level;
    else error ("packxbee: verbosity_level must be positive");
}

static int packxbee_outbuf_add(t_packxbee *x, int index, unsigned char val)
{
    int i = index;

/* if API mode is 2 all characters after the first are escaped if they are one of the sacred texts */
/* to escape the character prefix it with XSCAPE and XOR it with 0x20 */
    if
    (
        (2 == x->x_api_mode)
        &&
        (
            ((0 < index)&&(XFRAME == val))
            ||(XSCAPE == val)
            ||(XON == val)
            ||(XOFF == val)
        )
    )
    { /* escape the character */
        x->x_outbuf[i].a_w.w_float = XSCAPE;
        ++i;
        x->x_outbuf[i].a_w.w_float = val ^ 0x20;
        ++i;
    }
    else
    { /* otherwise just put it in the buffer */
        x->x_outbuf[i++].a_w.w_float = val;
    }
    return i;
}

/* send a packet given a 64-bit address, a 16-bit address, broadcast radius, options, followed by raw data */
static void packxbee_TX(t_packxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    unsigned char       floatstring[256]; /* longer than the longest hex number with each character escaped plus the header and checksum overhead */
    unsigned long long  dest64;
    unsigned int        dest16;
    int                 result;
    char                checksum = 0xFF;
    unsigned char       broadcast_radius, options;
    t_float             f;
    int                 d, i, j, k;
    int                 length = 0;
    unsigned char       c;

    if (argc < 5)
    {
        error("packxbee_TX: not enough parameters");
        return;
    }
    /* first arg is dest64, a symbol starting with "0x" */
    if (argv[0].a_type != A_SYMBOL)
    {
        error("packxbee_TX: first argument is not a symbol");
        return;
    }
    if ((argv[0].a_w.w_symbol->s_name[0] != '0')||(argv[0].a_w.w_symbol->s_name[1] != 'x'))
    {
        error("packxbee_TX: first argument is not a hex string beginning with \"0x\"");
        return;
    }
#ifdef _MSC_VER
    result = sscanf(argv[0].a_w.w_symbol->s_name, "0x%I64X", &dest64); 
#else
    result = sscanf(argv[0].a_w.w_symbol->s_name, "0x%LX", &dest64);
#endif
    if (result == 0)
    {
        error("packxbee_TX: first argument is not a hex string");
        return;
    }
#ifdef _MSC_VER
    if (x->x_verbosity > 1) post ("packxbee_TX: dest64:0x%016I64X", dest64);
#else
    if (x->x_verbosity > 1) post ("packxbee_TX: dest64:0x%016LX", dest64);
#endif
    /* second arg is dest16 also a symbol starting with "0x" */
    if (argv[1].a_type != A_SYMBOL)
    {
        error("packxbee_TX: second argument is not a symbol");
        return;
    }
    if ((argv[1].a_w.w_symbol->s_name[0] != '0')||(argv[1].a_w.w_symbol->s_name[1] != 'x'))
    {
        error("packxbee_TX: second argument is not a hex string beginning with \"0x\"");
        return;
    }
    result = sscanf(argv[1].a_w.w_symbol->s_name, "0x%X", &dest16);
    if (result == 0)
    {
        error("packxbee_TX: second argument is not a hex string");
        return;
    }
    if (x->x_verbosity > 1) post ("packxbee_TX: dest16: 0x%X", dest16);
    /* broadcast radius is a single byte as a float */
    if (argv[2].a_type != A_FLOAT)
    {
        error("packxbee_TX: third argument is not a float");
        return;
    }
    f = argv[2].a_w.w_float;
    if (x->x_verbosity > 1)  post("packxbee_TX float parameter %f", f);
    d = ((unsigned int)f)&0x0FF;
    if (f != d)
    {
        post ("packxbee_TX third argument not a positive integer from 0 to 255");
        return;
    }
    else broadcast_radius = d;
    if (x->x_verbosity > 1)  post("packxbee_TX: broadcast_radius: %d", d);

    /* options is a single byte as a float */
    if (argv[3].a_type != A_FLOAT)
    {
        error("packxbee_TX: fourth argument is not a float");
        return;
    }
    f = argv[3].a_w.w_float;
    if (x->x_verbosity > 1)  post("packxbee_TX float parameter %f", f);
    d = ((unsigned int)f)&0x0FF;
    if (f != d)
    {
        post ("packxbee_TX fourth argument not a positive integer from 0 to 255");
        return;
    }
    else options = d;
    if (x->x_verbosity > 1)  post("packxbee_TX: options: %d", d);

    x->x_frameType = ZigBee_Transmit_Request;/* because we're building a queued AT frame */
    floatstring[0] = XFRAME; /* as usual */
    floatstring[1] = 0; /* length MSB */
    floatstring[2] = 0;/* length LSB */
    floatstring[3] = x->x_frameType;
    checksum -= x->x_frameType;
    if (0 == x->x_frameID) x->x_frameID++;
    checksum -= x->x_frameID; /* frame ID */
    floatstring[4] = x->x_frameID++;
    /* raw 8 byte address in big-endian order: */
    floatstring[5] = (dest64>>56)&0x0FF;
    checksum -= floatstring[5];
    floatstring[6] = (dest64>>48)&0x0FF;
    checksum -= floatstring[6];
    floatstring[7] = (dest64>>40)&0x0FF;
    checksum -= floatstring[7];
    floatstring[8] = (dest64>>32)&0x0FF;
    checksum -= floatstring[8];
    floatstring[9] = (dest64>>24)&0x0FF;
    checksum -= floatstring[9];
    floatstring[10] = (dest64>>16)&0x0FF;
    checksum -= floatstring[10];
    floatstring[11] = (dest64>>8)&0x0FF;
    checksum -= floatstring[11];
    floatstring[12] = (dest64)&0x0FF;
    checksum -= floatstring[12];

    floatstring[13] = (dest16>>8)&0x0FF;
    checksum -= floatstring[13];
    floatstring[14] = (dest16)&0x0FF;
    checksum -= floatstring[14];

    floatstring[15] = broadcast_radius;
    checksum -= floatstring[15];
    floatstring[16] = options;
    checksum -= floatstring[16];
    /* the rest is payload */
    i = 17;
    for (k = 4; k < argc; ++k)
    {
        if (A_FLOAT == argv[k].a_type)
        {
            f = argv[k].a_w.w_float;
            if (x->x_verbosity > 1)  post("packxbee_TX float parameter %f", f);
            d = ((unsigned int)f)&0x0FF;
            if (f != d)
            {
                post ("packxbee_TX %dth argument not a positive integer from 0 to 255", k+1);
                return;
            }
            floatstring[i++] = d;
            checksum -= d;
        }
        else if (A_SYMBOL == argv[k].a_type)
        {
            if (x->x_verbosity > 1)  post("packxbee_TX symbol parameter %s", argv[k].a_w.w_symbol->s_name);
            j = i;
            i += sprintf((char *)&floatstring[i], "%s", argv[k].a_w.w_symbol->s_name);
            for (;j < i; ++j) checksum -= floatstring[j];
        }
        else
        {
            error("packxbee_TX %dth argument neither a float nor a symbol", k);
            return;
        }
    }
    length = i-3;
    floatstring[LENGTH_LSB_INDEX] = length & 0x0FF;
    floatstring[LENGTH_MSB_INDEX] = length >> 8;
    floatstring[i++] = checksum;
    k = j = 0; /* j indexes the outbuf, k indexes the floatbuf, i is the length of floatbuf */
    for (k = 0; k < i; ++k) j = packxbee_outbuf_add(x, j, floatstring[k]);
    outlet_list(x->x_listout, &s_list, j, x->x_outbuf);
    if(x->x_verbosity > 1)
    {
        for (k = 0; k < j; ++k)
        {
            c = (unsigned char)atom_getfloat(&x->x_outbuf[k]);
            post("buf[%d]: %d [0x%02X]", k, c, c);
        }
    }
}

/* format a queued AT packet and send it through x_listout as a list of floats */
static void packxbee_ATQ(t_packxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_frameType = AT_Command_Queue_Parameter_Value;/* we're building a queued AT frame */
    packxbee_pack_frame(x, s, argc, argv);
}

/* format an AT packet and send it through x_listout as a list of floats */
static void packxbee_AT(t_packxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_frameType = AT_Command;/* we're building an AT frame */
    packxbee_pack_frame(x, s, argc, argv);
}

/* format a remote AT packet and send it through x_listout as a list of floats */
static void packxbee_RAT(t_packxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_frameType = Remote_Command_Request;/* we're building a remote AT frame */
    if (argc < 3)
    {
        error("packxbee_RAT: not enough parameters");
        return;
    }
    packxbee_pack_remote_frame(x, s, argc, argv);
}

static void packxbee_pack_remote_frame(t_packxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    int                 i, j, k, maxdigits, d;
    char                checksum = 0xFF;
    unsigned char       floatstring[256]; /* longer than the longest hex number with each character escaped plus the header and checksum overhead */
    int                 length = 0;
    unsigned char       c, digits;
    t_float             f;

    unsigned long long  dest64;
    unsigned int        dest16;
    int                 result;
    unsigned char       options;


    if (x->x_verbosity > 0) post("packxbee_AT s is %s, argc is %d", s->s_name, argc);
    if (argc >= 4) /* we must have addr64, addr16, option byte, and an AT command */
    {
        /* first arg is dest64, a symbol starting with "0x" */
        if (argv[0].a_type != A_SYMBOL)
        {
            error("packxbee_pack_remote_frame: first argument is not a symbol");
            return;
        }
        if ((argv[0].a_w.w_symbol->s_name[0] != '0')||(argv[0].a_w.w_symbol->s_name[1] != 'x'))
        {
            error("packxbee_pack_remote_frame: first argument is not a hex string beginning with \"0x\"");
            return;
        }
#ifdef _MSC_VER
        result = sscanf(argv[0].a_w.w_symbol->s_name, "0x%I64X", &dest64);
#else
        result = sscanf(argv[0].a_w.w_symbol->s_name, "0x%LX", &dest64);
#endif
        if (result == 0)
        {
            error("packxbee_pack_remote_frame: first argument is not a hex string");
            return;
        }
#ifdef _MSC_VER
        if (x->x_verbosity > 0) post ("packxbee_pack_remote_frame: dest64:0x%016I64X", dest64);
#else
        if (x->x_verbosity > 0) post ("packxbee_pack_remote_frame: dest64:0x%016LX", dest64);
#endif
        /* second arg is dest16 also a symbol starting with "0x" */
        if (argv[1].a_type != A_SYMBOL)
        {
            error("packxbee_pack_remote_frame: second argument is not a symbol");
            return;
        }
        if ((argv[1].a_w.w_symbol->s_name[0] != '0')||(argv[1].a_w.w_symbol->s_name[1] != 'x'))
        {
            error("packxbee_pack_remote_frame: second argument is not a hex string beginning with \"0x\"");
            return;
        }
        result = sscanf(argv[1].a_w.w_symbol->s_name, "0x%X", &dest16);
        if (result == 0)
        {
            error("packxbee_pack_remote_frame: second argument is not a hex string");
            return;
        }
        if (x->x_verbosity > 0) post ("packxbee_pack_remote_frame: dest16: 0x%X", dest16);
        /* options is a single byte as a float */
        if (argv[2].a_type != A_FLOAT)
        {
            error("packxbee_pack_remote_frame: third argument is not a float");
            return;
        }
        f = argv[2].a_w.w_float;
        if (x->x_verbosity > 0)  post("packxbee_pack_remote_frame float parameter %f", f);
        d = ((unsigned int)f)&0x0FF;
        if (f != d)
        {
            post ("packxbee_pack_remote_frame third argument not a positive integer from 0 to 255");
            return;
        }
        else options = d;
        if (x->x_verbosity > 0)  post("packxbee_pack_remote_frame: options: %d", d);
        if (argv[3].a_type != A_SYMBOL)
        {
            post ("packxbee_pack_remote_frame: argument 4 must be an AT command");
            return;
        }
        if (x->x_verbosity > 0) post ("packxbee_pack_remote_frame: argument 4 is %s", argv[3].a_w.w_symbol->s_name);
        if (2 != strlen(argv[3].a_w.w_symbol->s_name))
        {
            post ("packxbee_pack_remote_frame: argument 4 must be a two-character AT command");
            return;
        }
        if (((argv[3].a_w.w_symbol->s_name[0] < 0x20) || (argv[3].a_w.w_symbol->s_name[0] & 0x80))
            || ((argv[3].a_w.w_symbol->s_name[1] < 0x20) || (argv[3].a_w.w_symbol->s_name[1] & 0x80)))
        {
            post ("packxbee_pack_remote_frame: argument 4 must be printable ascii");
            return;
        }
        /* parameters seem valid, now build the frame */
        i = 0;
        floatstring[i++] = XFRAME; /* as usual */
        floatstring[i++] = 0; /* length MSB */
        floatstring[i++] = 0;/* length LSB */
        floatstring[i++] = x->x_frameType;
        checksum -= x->x_frameType;
        if (0 == x->x_frameID) x->x_frameID++; /* never use zero as frame ID */
        floatstring[i] = x->x_frameID++;
        checksum -= floatstring[i++];

        /* raw 8 byte address in big-endian order: */
        floatstring[i] = (dest64>>56)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64>>48)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64>>40)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64>>32)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64>>24)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64>>16)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64>>8)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest64)&0x0FF;
        checksum -= floatstring[i++];

        floatstring[i] = (dest16>>8)&0x0FF;
        checksum -= floatstring[i++];
        floatstring[i] = (dest16)&0x0FF;
        checksum -= floatstring[i++];

        floatstring[i] = options;
        checksum -= floatstring[i++];

        c = argv[3].a_w.w_symbol->s_name[0];/* the first character of the AT command */
        floatstring[i] = c;
        checksum -= floatstring[i++];
        c = argv[3].a_w.w_symbol->s_name[1];/* the second character of the AT command */
        floatstring[i] = c;
        checksum -= floatstring[i++];
        j = i; /* save i in j so we can calculate checksum on any further parameters */
        /* parameters if any */
        if (argc >= 5)
        { /* some parameters */
            if (argv[4].a_type == A_SYMBOL)
            {
                if (x->x_verbosity > 0)  post("packxbee_pack_remote_frame symbol parameter %s", argv[4].a_w.w_symbol->s_name);
                if (('0' == argv[4].a_w.w_symbol->s_name[0])&&(('x' == argv[4].a_w.w_symbol->s_name[1])))
                { /* this is a hexadecimal number: strip the "0x" and copy the rest to the buffer as ascii digits */
                    i += sprintf((char *)&floatstring[i], "%s", &argv[4].a_w.w_symbol->s_name[2]);
                }
                else // if ((0 == strncmp("NI", argv[0].a_w.w_symbol->s_name, 2))||(0 == strncmp("DN", argv[0].a_w.w_symbol->s_name, 2)))
                { /* we hope it's just an ascii string for the NI command */
                    for (k = 0; (k < 20); ++k) /* no more than 20 characters in a node identifier */
                    {
                        c = argv[4].a_w.w_symbol->s_name[k];
                        if (0 == c) break;
                        //checksum -= c;
                        floatstring[i++] = c;
                    }
                }
            }
            else if (argv[4].a_type == A_FLOAT)
            {
                f = argv[4].a_w.w_float;
                if (x->x_verbosity > 0)  post("packxbee_pack_remote_frame float parameter %f", f);
                d = ((unsigned int)f)&0x0FF;
                if (f != d)
                {
                    post ("packxbee_pack_remote_frame parameter not a positive integer from 0 to 255");
                }
                else
                {
                    // put the significant part of the raw value into floatstring in big endian order
                    if (0 != ((d>>24) & 0x0FF)) digits = 4;
                    else if (0 != ((d>>16) & 0x0FF)) digits = 3;
                    else if (0 != ((d>>8) & 0x0FF)) digits = 2;
                    else digits = 1;
                    if (4 == digits) floatstring[i++] = (d>>24) & 0x0FF;
                    if (3 <= digits) floatstring[i++] = (d>>16) & 0x0FF;
                    if (2 <= digits) floatstring[i++] = (d>>8) & 0x0FF;
                    floatstring[i++] = d & 0x0FF;
                }
            }
            else
            {
                post("packxbee_pack_remote_frame parameter not symbol or float: ignoring");
            }
            maxdigits = 32; /* the longest possible hex string is for the encryption key */
            /* we leave it up to the user to send the correct values */
            if (j != i)
            { /* update the checksum */
                for (; ((j < maxdigits)&&(j < i)); ++j)
                {
                    c = floatstring[j];
                    if (0 == c) break;
                    checksum -= c;
                }
            }
        } /* argc >= 5 */
    } /* argc >= 4 */
    else
    {
        error("packxbee_pack_remote_frame: not enough parameters");
        return;
    }
    length = i-3;
    floatstring[LENGTH_LSB_INDEX] = length & 0x0FF;
    floatstring[LENGTH_MSB_INDEX] = length >> 8;
    floatstring[i++] = checksum;
    k = j = 0; /* j indexes the outbuf, k indexes the floatbuf, i is the length of floatbuf */
    for (k = 0; k < i; ++k) j = packxbee_outbuf_add(x, j, floatstring[k]);
    outlet_list(x->x_listout, &s_list, j, x->x_outbuf);
    if(x->x_verbosity > 1)
    {
        for (k = 0; k < j; ++k)
        {
            c = (unsigned char)atom_getfloat(&x->x_outbuf[k]);
            post("buf[%d]: %d [0x%02X]", k, c, c);
        }
    }
}

static void packxbee_pack_frame(t_packxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    int             i, j, k, maxdigits, d;
    char            checksum = 0xFF;
    unsigned char   floatstring[256]; /* longer than the longest hex number with each character escaped plus the header and checksum overhead */
    int             length = 0;
    unsigned char   c, digits;
    t_float         f;

    if (x->x_verbosity > 0) post("packxbee_AT s is %s, argc is %d", s->s_name, argc);
    if (argc >= 1)
    {
        if (argv[0].a_type != A_SYMBOL)
        {
            post ("packxbee_AT: argument 1 must be an AT command");
            return;
        }
        if (x->x_verbosity > 0) post ("packxbee_AT: argument 1 is %s", argv[0].a_w.w_symbol->s_name);
        if (2 != strlen(argv[0].a_w.w_symbol->s_name))
        {
            post ("packxbee_AT: argument 1 must be a two-character AT command");
            return;
        }
        if (((argv[0].a_w.w_symbol->s_name[0] < 0x20) || (argv[0].a_w.w_symbol->s_name[0] & 0x80))
            || ((argv[0].a_w.w_symbol->s_name[1] < 0x20) || (argv[0].a_w.w_symbol->s_name[1] & 0x80)))
        {
            post ("packxbee_AT: argument 1 must be printable ascii");
            return;
        }
        i = 0;
        floatstring[i++] = XFRAME; /* as usual */
        floatstring[i++] = 0; /* length MSB */
        floatstring[i++] = 0;/* length LSB */
        floatstring[i] = x->x_frameType;
        checksum -= floatstring[i++];
        if (0 == x->x_frameID) x->x_frameID++;
        floatstring[i] = x->x_frameID++;
        checksum -= floatstring[i++];
        c = argv[0].a_w.w_symbol->s_name[0];/* the first character of the AT command */
        floatstring[i] = c;
        checksum -= floatstring[i++];
        c = argv[0].a_w.w_symbol->s_name[1];/* the second character of the AT command */
        floatstring[i] = c;
        checksum -= floatstring[i++];
        j = i; /* store i in j to see if anything gets added later and we need to update the checksum */
        /* parameters if any */
        if (argc >= 2)
        { /* some parameters */
            if (argv[1].a_type == A_SYMBOL)
            {
                if (x->x_verbosity > 0)  post("packxbee_AT symbol parameter %s", argv[1].a_w.w_symbol->s_name);
                if (('0' == argv[1].a_w.w_symbol->s_name[0])&&(('x' == argv[1].a_w.w_symbol->s_name[1])))
                { /* this is a hexadecimal number: strip the "0x" and copy the rest to the buffer as ascii digits */
                    i += sprintf((char *)&floatstring[i], "%s", &argv[1].a_w.w_symbol->s_name[2]);
                }
                else // if ((0 == strncmp("NI", argv[0].a_w.w_symbol->s_name, 2))||(0 == strncmp("DN", argv[0].a_w.w_symbol->s_name, 2)))
                { /* we hope it's just an ascii string for the NI command */
                    for (k = 0; (k < 20); ++k) /* no more than 20 characters in a node identifier */
                    {
                        c = argv[1].a_w.w_symbol->s_name[k];
                        if (0 == c) break;
//                        checksum -= c;
                        floatstring[i++] = c;
                    }
                }
            }
            else if (argv[1].a_type == A_FLOAT)
            {
                f = argv[1].a_w.w_float;
                if (x->x_verbosity > 0)  post("packxbee_AT float parameter %f", f);
                d = ((unsigned int)f)&0x0FF;
                if (f != d)
                {
                    post ("packxbee_AT parameter not a positive integer from 0 to 255");
                }
                else
                {
                    // put the significant part of the raw value into floatstring in big endian order
                    if (0 != ((d>>24) & 0x0FF)) digits = 4;
                    else if (0 != ((d>>16) & 0x0FF)) digits = 3;
                    else if (0 != ((d>>8) & 0x0FF)) digits = 2;
                    else digits = 1;
                    if (4 == digits) floatstring[i++] = (d>>24) & 0x0FF;
                    if (3 <= digits) floatstring[i++] = (d>>16) & 0x0FF;
                    if (2 <= digits) floatstring[i++] = (d>>8) & 0x0FF;
                    floatstring[i++] = d & 0x0FF;
                }
            }
            else
            {
                post("packxbee_AT parameter not symbol or float: ignoring");
            }
            maxdigits = 32; /* the longest possible hex string is for the encryption key */
            /* we leave it up to the user to send the correct values */
            /* if anything was added to floatstring, calculate the checksum on it */
            /* update the checksum */
            if (j != i)
            {
                for (; ((j < maxdigits)&&(j < i)); ++j)
                {
                    c = floatstring[j];
                    if (0 == c) break;
                    checksum -= c;
                }
            }
        } /* argc >= 2 */
        length = i-3;
        floatstring[LENGTH_LSB_INDEX] = length & 0x0FF;
        floatstring[LENGTH_MSB_INDEX] = length >> 8;
        floatstring[i++] = checksum;
        k = j = 0; /* j indexes the outbuf, k indexes the floatbuf, i is the length of floatbuf */
        for (k = 0; k < i; ++k) j = packxbee_outbuf_add(x, j, floatstring[k]);
        outlet_list(x->x_listout, &s_list, j, x->x_outbuf);
        if(x->x_verbosity > 1)
        {
            for (k = 0; k < j; ++k)
            {
                c = (unsigned char)atom_getfloat(&x->x_outbuf[k]);
                post("buf[%d]: %d [0x%02X]", k, c, c);
            }
        }
    } /* argc >= 1 */
}

static void packxbee_free(t_packxbee *x)
{
    /* free any memory we allocated */
    /* stop any callbacks */
}

void packxbee_setup(void)
{ 
    packxbee_class = class_new(gensym("packxbee"), (t_newmethod)packxbee_new,
        (t_method)packxbee_free,
        sizeof(t_packxbee), 0, A_DEFFLOAT, 0);
    class_addmethod(packxbee_class, (t_method)packxbee_AT, gensym("AT"), A_GIMME, 0);
    class_addmethod(packxbee_class, (t_method)packxbee_ATQ, gensym("ATQ"), A_GIMME, 0);
    class_addmethod(packxbee_class, (t_method)packxbee_RAT, gensym("RAT"), A_GIMME, 0);
    class_addmethod(packxbee_class, (t_method)packxbee_TX, gensym("TX"), A_GIMME, 0);
    class_addmethod(packxbee_class, (t_method)packxbee_API, gensym("API"), A_DEFFLOAT, 0);
    class_addmethod(packxbee_class, (t_method)packxbee_verbosity, gensym("verbosity"), A_DEFFLOAT, 0);
}

/* fin packxbee.c*/
