/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_bin_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#ifndef __IEMBINAMBI_H__
#define __IEMBINAMBI_H__

#define BIN_AMBI_LS_REAL 0
#define BIN_AMBI_LS_IND 0
#define BIN_AMBI_LS_MRG 1
#define BIN_AMBI_LS_MIR 2
#define BIN_AMBI_LS_PHT 3

typedef struct
{
    float real;
    float imag;
}
BIN_AMBI_COMPLEX;

#endif
