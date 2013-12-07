/* unpackxbee outputs a list of floats which are the bytes making up an xbee api packet. */
/* The packet can then be sent through [comport]. */
/* Started by Martin Peach 20110731 */
/* Information taken from "XBee®/XBee-PRO® ZB RF Modules" (document 90000976_G, 11/15/2010)*/
/* by Digi International Inc. http://www.digi.com */

#include <stdio.h>
//#include <string.h>
#include "m_pd.h"
#include "pdxbee.h"

static t_class *unpackxbee_class;


typedef struct _unpackxbee
{
    t_object        x_obj;
    t_outlet        *x_status_out;
    t_outlet        *x_payload_out;
    int             x_api_mode;
    unsigned char   x_frame_ID;
    unsigned char   x_frame_type;
    unsigned int    x_frame_length;
    int             x_verbosity;
    unsigned char   x_message[MAX_XBEE_PACKET_LENGTH];
    unsigned int    x_message_index;
    int             x_escaped;
    t_atom          x_outbuf[MAX_XBEE_PACKET_LENGTH];
    t_atom          x_statusbuf[32]; /* some number bigger than we will ever reach */
} t_unpackxbee;

static void *unpackxbee_new(t_floatarg f);
static void unpackxbee_input(t_unpackxbee *x, t_symbol *s, int argc, t_atom *argv);
static void unpackxbee_API(t_unpackxbee *x, t_float api);
static void unpackxbee_verbosity(t_unpackxbee *x, t_float verbosity_level);
static void unpackxbee_free(t_unpackxbee *x);
void unpackxbee_setup(void);

static void *unpackxbee_new(t_floatarg f)
{
    int i;

    t_unpackxbee *x = (t_unpackxbee *)pd_new(unpackxbee_class);
    if (x)
    {
        x->x_payload_out = outlet_new(&x->x_obj, &s_list); /* the first outlet on the left */
        x->x_status_out = outlet_new(&x->x_obj, &s_list);

        if (1 == f) x->x_api_mode = 1;
        else x->x_api_mode = 2; /* default to escaped mode */
        x->x_verbosity = 0; /* debug level */
        for(i = 0; i < MAX_XBEE_PACKET_LENGTH; ++i) x->x_outbuf[i].a_type = A_FLOAT; /* init output atoms as floats */
    }
    return (x);
}

static void unpackxbee_API(t_unpackxbee *x, t_float api)
{
    if ((api == 1) || (api ==2)) x->x_api_mode = api;
    else error ("unpackxbee: api mode must be 1 or 2");
}

static void unpackxbee_verbosity(t_unpackxbee *x, t_float verbosity_level)
{
    if (verbosity_level >= 0) x->x_verbosity = verbosity_level;
    else error ("packxbee: verbosity_level must be positive");
}

int unpackxbee_add(t_unpackxbee *x, unsigned char d)
{
    if (XFRAME == d)
    {
        if (x->x_verbosity > 0) post ("frame start");
        x->x_message_index = 0;
        x->x_frame_length = 0;
        x->x_frame_type = 0;
        x->x_escaped = 0;
    }
    if (2 == x->x_api_mode)
    {
        if (XSCAPE == d)
        {
            x->x_escaped = 1; /* we need to xor the next character with 0x20 */
            return 0; /* don't store the escape character */
        }
        else if (1 == x->x_escaped)
        {
            d ^= 0x20; /* xor with 0x20 to restore the original character */
            x->x_escaped = 0; /* don't do it again */
        }
    }
    if (LENGTH_LSB_INDEX == x->x_message_index) /* length is a bigendian pair */
    {
        x->x_frame_length = (x->x_message[LENGTH_MSB_INDEX]<<8) + d;
        if (x->x_verbosity > 0) post ("frame length %d", x->x_frame_length);
    }
    else if (FRAME_TYPE_INDEX == x->x_message_index)
    {
        x->x_frame_type = d;
        if (x->x_verbosity > 0) post ("frame type 0x%02X", x->x_frame_type);
    }
    else if (FRAME_ID_INDEX == x->x_message_index)
    { /* this is part of the payload and may not be valid in some frame types */
        x->x_frame_ID = d;
        if (x->x_verbosity > 0) post ("frame ID %d", x->x_frame_ID);
    }
    x->x_message[x->x_message_index++] = d; /* add the unescaped character to the output list */
    return 1;
}

static void unpackxbee_input(t_unpackxbee *x, t_symbol *s, int argc, t_atom *argv)
{
    int                 i , j, k;
    t_float             f;
    unsigned int        d, checksum = 0;
    unsigned char       c;
    t_symbol            *type_selector;
    int                 statuslength = 0, payloadstart = 0;
    char                atbuf[64];
    unsigned char       floatstring[256]; /* longer than the longest hex number with each character escaped plus the header and checksum overhead */
    unsigned long long  addr64;
    unsigned int        addr16;

    if (x->x_verbosity > 0) post("unpackxbee_input: s is %s, argc is %d", s->s_name, argc);
    for (i = 0; i < argc; ++i)
    {
        if (A_FLOAT == argv[i].a_type)
        {
            f = argv[i].a_w.w_float;
            d = ((unsigned int)f)&0x0FF;
            if (x->x_verbosity > 0) post("unpackxbee_input: argv[%d] is %f int is %d", i, f, d);
            if (f != d)
            {
                post ("unpackxbee_input not a positive integer from 0 to 255");
            }
            else unpackxbee_add(x, d);
        }
        else
            post("unpackxbee_input: item %d is not a float", i+1);
    }
    if ((x->x_frame_length > 0)&&(x->x_frame_length + 4 == x->x_message_index))
    { /* end of frame reached */
        k = x->x_frame_length+4; /* total packet length is payload + 1 start 2 length 1 checksum*/
        if(x->x_verbosity > 0)
        {
            post("frame end");
            for (j = 0; j < k; ++j)
            {
                c = x->x_message[j];
                post("unpackxbee buf[%d]: %d [0x%02X]", j, c, c);
            }
        }
        for (j = 3; j < k; ++j)
        {
            checksum += x->x_message[j];
        }
        checksum &= 0x0FF;
        if (checksum != 0xFF)
        {
            post("unpackxbee: wrong checksum; dropping packet");
            return;
        }
        if(x->x_verbosity > 0) post("unpackxbee checksum %d [0x%02X]", checksum, checksum);
        switch(x->x_frame_type)
        {
            case AT_Command:
                type_selector = gensym("AT_Command");
                break;
            case AT_Command_Queue_Parameter_Value:
                type_selector = gensym("AT_Command_Queue_Parameter_Value");
                break;
            case ZigBee_Transmit_Request:
                type_selector = gensym("ZigBee_Transmit_Request");
                break;
            case Explicit_Addressing_ZigBee_Command_Frame:
                type_selector = gensym("Explicit_Addressing_ZigBee_Command_Frame");
                break;
            case Remote_Command_Request:
                type_selector = gensym("Remote_Command_Request");
                break;
            case Create_Source_Route:
                type_selector = gensym("Create_Source_Route");
                break;
            case AT_Command_Response:
                type_selector = gensym("AT_Command_Response");
                break;
            case Modem_Status:
                type_selector = gensym("Modem_Status");
                break;
            case ZigBee_Transmit_Status:
                type_selector = gensym("ZigBee_Transmit_Status");
                break;
            case ZigBee_Receive_Packet:
                type_selector = gensym("ZigBee_Receive_Packet");
                break;
            case ZigBee_Explicit_Rx_Indicator:
                type_selector = gensym("ZigBee_Explicit_Rx_Indicator");
                break;
            case ZigBee_IO_Data_Sample_Rx_Indicator:
                type_selector = gensym("ZigBee_IO_Data_Sample_Rx_Indicator");
                break;
            case XBee_Sensor_Read_Indicator:
                type_selector = gensym("XBee_Sensor_Read_Indicator");
                break;
            case Node_Identification_Indicator:
                type_selector = gensym("Node_Identification_Indicator");
                break;
            case Remote_Command_Response:
                type_selector = gensym("Remote_Command_Response");
                break;
            case Over_the_Air_Firmware_Update_Status:
                type_selector = gensym("Over_the_Air_Firmware_Update_Status");
                break;
            case Route_Record_Indicator:
                type_selector = gensym("Route_Record_Indicator");
                break;
            case Many_to_One_Route_Request_Indicator:
                type_selector = gensym("Many_to_One_Route_Request_Indicator");
                break;
            default:
                type_selector = gensym("unknown");
        }
        statuslength = 0;
        SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_type);
        statuslength++;
        if
        (
            (AT_Command_Response == x->x_frame_type)
            ||(AT_Command == x->x_frame_type)
            ||(AT_Command_Queue_Parameter_Value == x->x_frame_type)
        )
        {
            if (x->x_verbosity > 0) 
                post("AT_Command_Response  AT_Command AT_Command_Queue_Parameter_Value statuslength %d", statuslength);
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_ID);
            statuslength++;
            /* data doesn't include 1byte frame type 1byte ID 2byte AT command 1byte AT command status = 5bytes */
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_length-5);
            statuslength++;
            atbuf[0] = x->x_message[5]; /* the AT command string */
            atbuf[1] = x->x_message[6];
            atbuf[2] = '\0';
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym(atbuf));
            statuslength++;
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[7]);/* AT command status */
            statuslength++;
            if ((0 == x->x_message[7]) && ('N' == x->x_message[5]) && ('D' == x->x_message[6]))
            { /* a succesful node discover response: output the addresses as symbols */
/*
buf[0]: 126 [0x7E] ND packet start
buf[1]: 0 [0x00] Length MSB
buf[2]: 25 [0x19] Length LSB
buf[3]: 136 [0x88] AT response
buf[4]: 5 [0x05] packet ID
buf[5]: 78 [0x4E] N
buf[6]: 68 [0x44] D
buf[7]: 0 [0x00] status
*/
                addr16 = (x->x_message[8]<<8) + x->x_message[9];
                sprintf((char *)floatstring, "0x%X", addr16);
                SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring));
                statuslength++;
/*
buf[8]: 121 [0x79] MY
buf[9]: 214 [0xD6] 
*/
/* now switch endianness */

                addr64 = x->x_message[10]; 
                addr64 <<= 8;
                addr64 |= x->x_message[11];
                addr64 <<= 8;
                addr64 |= x->x_message[12];
                addr64 <<= 8;
                addr64 |= x->x_message[13];
                addr64 <<= 8;
                addr64 |= x->x_message[14];
                addr64 <<= 8;
                addr64 |= x->x_message[15];
                addr64 <<= 8;
                addr64 |= x->x_message[16];
                addr64 <<= 8;
                addr64 |= x->x_message[17];
#ifdef _MSC_VER
                sprintf((char *)floatstring, "0x%016I64X", addr64);
#else
                sprintf((char *)floatstring, "0x%016LX", addr64);
#endif
                SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* addr64 */
                statuslength++;
/* 
buf[10]: 0 [0x00] SH
buf[11]: 19 [0x13]
buf[12]: 162 [0xA2]
buf[13]: 0 [0x00]
buf[14]: 64 [0x40] SL
buf[15]: 106 [0x6A]
buf[16]: 222 [0xDE]
buf[17]: 30 [0x1E]
*/
                for (j = 0, i = 18; i < k; ++i, ++j)
                {
                    floatstring[j] = x->x_message[i];
                    if (0 == floatstring[j])
                    {
                        i++;
                        break;/* Node Identifier should be a null-terminated ascii string */
                    }
                }
                SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* Node Identifier */
                statuslength++;
/*
buf[18]: 32 [0x20] NI
buf[19]: 0 [0x00]
*/
                addr16 = (x->x_message[i]<<8) + x->x_message[i+1];
                sprintf((char *)floatstring, "0x%X", addr16);
                i += 2;
                SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* parent addr16 */
                statuslength++;
/*
buf[20]: 255 [0xFF] parent
buf[21]: 254 [0xFE]
*/
                SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[i++]);/* Device Type */
                statuslength++;
/*
buf[22]: 1 [0x01] device type
*/
                SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[i++]);/* Source Event */
                statuslength++;
/*
buf[23]: 0 [0x00] source event
*/
                addr16 = x->x_message[i++]<<8;
                addr16 |= x->x_message[i++];
                sprintf((char *)floatstring, "0x%X", addr16);
                SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* Profile ID */
                statuslength++;
/*
buf[24]: 193 [0xC1] Profile ID
buf[25]: 5 [0x05]
*/
                addr16 = (x->x_message[i]<<8) + x->x_message[i+1];
                sprintf((char *)floatstring, "0x%X", addr16);
                i += 2;
                SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* Manufacturer ID */
                statuslength++;
/*
buf[26]: 16 [0x10] Manufacturer ID
buf[27]: 30 [0x1E]
*/

/*
buf[28]: 36 [0x24] checksum
*/
                payloadstart = 0;/* no payload */
            }
            else
            {
                payloadstart = 8;
            }
        }
/* RAT */
        if (Remote_Command_Response == x->x_frame_type)
        {
            if (x->x_verbosity > 0) 
                post("Remote_Command_Response statuslength %d", statuslength);
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_ID);
            statuslength++;
/*
buf[0]: 126 [0x7E] packet start
buf[1]: 0 [0x00] Length MSB

buf[2]: 25 [0x19] Length LSB
buf[3]: 151 [0x97] remote response frame type
buf[4]: 5 [0x05] packet ID
buf[5-12]: 0 [0x00] 64-bit source (remote) address
buf[13-14]: 68 [0x44] 16-bit source address
buf[15-16]: AT command
buf[17]: status
buf[18...] data
*/

            addr64 = x->x_message[5]; 
            addr64 <<= 8;
            addr64 |= x->x_message[6];
            addr64 <<= 8;
            addr64 |= x->x_message[7];
            addr64 <<= 8;
            addr64 |= x->x_message[8];
            addr64 <<= 8;
            addr64 |= x->x_message[9];
            addr64 <<= 8;
            addr64 |= x->x_message[10];
            addr64 <<= 8;
            addr64 |= x->x_message[11];
            addr64 <<= 8;
            addr64 |= x->x_message[12];
#ifdef _MSC_VER
            sprintf((char *)floatstring, "0x%016I64X", addr64);
#else
            sprintf((char *)floatstring, "0x%016LX", addr64);
#endif
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* addr64 */
            statuslength++;

            addr16 = (x->x_message[13]<<8) + x->x_message[14];
            sprintf((char *)floatstring, "0x%X", addr16);
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring));
            statuslength++;
            atbuf[0] = x->x_message[15]; /* the remote AT command string */
            atbuf[1] = x->x_message[16];
            atbuf[2] = '\0';
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym(atbuf));
            statuslength++;

            SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[17]);/* AT command status */
            statuslength++;
            /* data doesn't include 1byte frame type 1byte ID 8byte addr64 2byte addr16 2byte AT command 1byte status = 15bytes */
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_length-15);
            statuslength++;
            payloadstart = 18;
         }
/* RAT */
        else if (ZigBee_Transmit_Status == x->x_frame_type)
        {
            if (x->x_verbosity > 0) 
                post("ZigBee_Transmit_Status statuslength %d", statuslength);
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_ID);
            statuslength++;
            sprintf(atbuf, "0x%X", (x->x_message[5]<<8) + x->x_message[6]); /* the 16-bit address as a symbol */
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym(atbuf));
            statuslength++;
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[7]);/* Transmit Retry Count */
            statuslength++;
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[8]);/* Delivery Status */
            statuslength++;
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[9]);/* Discovery Status */
            statuslength++;
            payloadstart = 0; /* no payload */
        }
        else if (ZigBee_Receive_Packet == x->x_frame_type)
        {
            if (x->x_verbosity > 0) 
                post("ZigBee_Receive_Packet statuslength %d", statuslength);
            /* data doesn't include 1byte frametype, 8byte addr64, 2byte addr16, 1byte options = 12bytes*/
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_length-12);
            statuslength++;
            /* frame type */
            /* 64-bit source address */
            i = 4;
            addr64 = x->x_message[i++]; 
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
            addr64 <<= 8;
            addr64 |= x->x_message[i++];
#ifdef _MSC_VER
            sprintf((char *)floatstring, "0x%016I64X", addr64);
#else
            sprintf((char *)floatstring, "0x%016LX", addr64);
#endif
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* addr64 */
            statuslength++;
            /* 16-bit source address */
            addr16 = x->x_message[i++]<<8;
            addr16 |= x->x_message[i++];
            sprintf((char *)floatstring, "0x%X", addr16);
            SETSYMBOL(&x->x_statusbuf[statuslength], gensym((char *)floatstring)); /* addr16 */
            statuslength++;
            /* receive options byte */
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_message[i++]);/* 1 2 32 64 */
            statuslength++;
            /* data */
            payloadstart = i;
        }
        else
        {
            if (x->x_verbosity > 0) 
                post("some other packet statuslength %d", statuslength);
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_ID);/* may not be valid */
            statuslength++;
            SETFLOAT(&x->x_statusbuf[statuslength], x->x_frame_length-2);/* payload doesn't include frame type and ID */
            statuslength++;
            payloadstart = 5;
        }
        outlet_anything(x->x_status_out, type_selector, statuslength, x->x_statusbuf);
        if (payloadstart > 0)
        {
            for (j = 0, i = payloadstart; i < k-1; ++j, ++i)
                SETFLOAT(&x->x_outbuf[j], x->x_message[i]); /* the payload */
            if (j > 0)
                outlet_list(x->x_payload_out, &s_list, j, x->x_outbuf);
        }
    }
}

static void unpackxbee_free(t_unpackxbee *x)
{
}

void unpackxbee_setup(void)
{ 
    unpackxbee_class = class_new(gensym("unpackxbee"), (t_newmethod)unpackxbee_new,
        (t_method)unpackxbee_free,
        sizeof(t_unpackxbee), 0, A_DEFFLOAT, 0);
    class_addanything(unpackxbee_class, (t_method)unpackxbee_input);
    class_addmethod(unpackxbee_class, (t_method)unpackxbee_API, gensym("API"), A_DEFFLOAT, 0);
    class_addmethod(unpackxbee_class, (t_method)unpackxbee_verbosity, gensym("verbosity"), A_DEFFLOAT, 0);
}

/* fin unpackxbee.c*/
