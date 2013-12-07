/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_mp3 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

/*
 mp3play~.c - Mpeg Layer III Player for PD
 Version:0.1
 05-18-2000
 written by Thomas Musil (musil_at_iem.at), Norbert Math (math_at_iem.kug.ac.at)
 IEM Graz

 debugged for windows 013-03-2003

 This MPEG Player is based on the mpglib 0.2 by Michael Hipp which comes with mpg123-0.59r

 please see the README file for copyright notices!

 */

#include "m_pd.h"
#include "iemlib.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#ifndef _WIN32
#include <sys/signal.h>
#include <unistd.h>
#endif


#define IEM_MPEG1

#ifdef _WIN32
# undef WIN32
# define WIN32

# define M_PI 3.14159265358979323846
# define M_SQRT2 1.41421356237309504880
# define REAL_IS_FLOAT
# define NEW_DCT9

# define random rand
# define srandom srand

#endif

#ifdef REAL_IS_FLOAT
#  define real float
#elif defined(REAL_IS_LONG_DOUBLE)
#  define real long double
#else
#  define real double
#endif

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

/* AUDIOBUFSIZE = n*64 with n=1,2,3 ...  */
#define         AUDIOBUFSIZE    16384

#define         FALSE                   0
#define         TRUE                    1
#define         SBLIMIT                 32
#define         SSLIMIT                 18
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

#define MAXFRAMESIZE 1792
/* Pre Shift fo 16 to 8 bit converter table */
#define AUSHIFT (3)
#define BOOL int
#define MP3_EX  -2
#define MP3_ERR -1
#define MP3_OK  0
#define MP3_NEED_MORE 1

#define WRITE_SAMPLE(samples,sum,clip) \
    if( (sum) > 32767.0) { *(samples) = 0x7fff; (clip)++; } \
    else if( (sum) < -32768.0) { *(samples) = -0x8000; (clip)++; } \
    else { *(samples) = sum; }


#define HDRCMPMASK 0xfffffd00


#define    MY_MP3_MALLOC_IN_SIZE    16384
#define    MY_MP3_MALLOC_IN_SIZE2   8192
#define    MY_MP3_MALLOC_OUT_SIZE   65536
#define    MY_MP3_MALLOC_FN         400
#define    MY_MP3_CHSAMP_PER_FRAME  1152

struct iemmp3_frame
{
    int stereo;
    int jsbound;
    int single;
    int lsf;
    int mpeg25;
    int header_change;
    int lay;
    int error_protection;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
    int framesize; /* computed framesize */
};

struct iemmp3_gr_info_s
{
    int scfsi;
    unsigned part2_3_length;
    unsigned big_values;
    unsigned scalefac_compress;
    unsigned block_type;
    unsigned mixed_block_flag;
    unsigned table_select[3];
    unsigned subblock_gain[3];
    unsigned maxband[3];
    unsigned maxbandl;
    unsigned maxb;
    unsigned region1start;
    unsigned region2start;
    unsigned preflag;
    unsigned scalefac_scale;
    unsigned count1table_select;
    real *full_gain[3];
    real *pow2gain;
};

struct iemmp3_III_sideinfo
{
    unsigned main_data_begin;
    unsigned private_bits;
    struct
    {
  struct iemmp3_gr_info_s gr[2];
    } ch[2];
};

struct iemmp3_buf
{
    unsigned char *pnt;
    long size;
    long pos;
    struct iemmp3_buf *next;
    struct iemmp3_buf *prev;
};

struct iemmp3Struct
{
    struct iemmp3_buf *head,*tail;
    int bsize;
    int framesize;
    int fsizeold;
    struct iemmp3_frame fr;
    unsigned char bsspace[2][MAXFRAMESIZE+512]; /* MAXFRAMESIZE */
    real hybrid_block[2][2][SBLIMIT*SSLIMIT];
    int hybrid_blc[2];
    unsigned long header;
    int bsnum;
    real synth_buffs[2][2][0x110];
    int  synth_bo;
};

struct iemmp3_bandInfoStruct
{
    short longIdx[23];
    short longDiff[22];
    short shortIdx[14];
    short shortDiff[13];
};

struct iemmp3_newHuff
{
    unsigned int linbits;
    short *table;
};


typedef struct _mp3play_tilde
{
    t_object x_obj;
    float length_sec;
    int samp_per_frame;
    int frame_counter;
    int time1_bang0_handle;
    float time_factor;
    int *begframeseek;
    int curframeseek;
    int maxframeseek;
    char *filename;
    float offset_sec;
    int file_is_open;
    int play_state;
    int mp3_encode_size;
    int mp3_out_index;
    int file_size;
    int file_block_num;
    int file_remain;
    int mp_is_init;
    int mp3_ch;
    int mp3_sr;
    int mp3_byterate;
    int obj_sr;
    int obj_n;
    int  down;
    float scale;
    FILE *fh;
    char *mp3inbuf;
    char *mp3outbuf;
    struct iemmp3Struct mp;
    t_clock  *x_clock;
    t_outlet *x_bangout;
    t_outlet *x_floatout;
    t_canvas  *x_canvas;
} t_mp3play_tilde;



static BOOL InitMP3 (struct iemmp3Struct *mp);
static BOOL InitAgainMP3(struct iemmp3Struct *mp);
static void ExitMP3 (struct iemmp3Struct *mp);
static int  decodeMP3 (struct iemmp3Struct *mp, char *in, int isize, char *out, int osize, int *done);
static void make_decode_tables (long scaleval);
static void init_layer3 (int down_sample_sblimit);
static int  decode_header (struct iemmp3_frame *fr,unsigned long newhead);
static int  do_layer3(struct iemmp3_frame *fr,unsigned char *pcm_sample,int *pcm_point,int *err);
static int  set_pointer(long backstep);
static int  synth_1to1_mono(real *bandPtr,unsigned char *samples,int *pnt);
static int  synth_1to1(real *bandPtr,int channel,unsigned char *out,int *pnt);
static struct iemmp3_buf *addbuf(struct iemmp3Struct *mp,char *buf,int size);
static void remove_buf(struct iemmp3Struct *mp);
static int read_head(struct iemmp3Struct *mp);
static int  read_buf_byte(struct iemmp3Struct *mp,int *err);
static int III_get_side_info_2(struct iemmp3_III_sideinfo *si,int stereo, int ms_stereo,long sfreq,int single);
#ifdef IEM_MPEG1
static int III_get_side_info_1(struct iemmp3_III_sideinfo *si,int stereo,int ms_stereo,long sfreq,int single);
static int  III_get_scale_factors_1(int *scf,struct iemmp3_gr_info_s *gr_info);
#endif
static int  III_get_scale_factors_2(int *scf,struct iemmp3_gr_info_s *gr_info,int i_stereo);
static int  III_dequantize_sample(real xr[SBLIMIT][SSLIMIT],int *scf,struct iemmp3_gr_info_s *gr_info,int sfreq,int part2bits);
static void III_i_stereo(real xr_buf[2][SBLIMIT][SSLIMIT],int *scalefac,struct iemmp3_gr_info_s *gr_info,int sfreq,int ms_stereo,int lsf);
static void III_antialias(real xr[SBLIMIT][SSLIMIT],struct iemmp3_gr_info_s *gr_info);
static void III_hybrid(real fsIn[SBLIMIT][SSLIMIT],real tsOut[SSLIMIT][SBLIMIT],int ch,struct iemmp3_gr_info_s *gr_info);
static void dct64(real *a,real *b,real *c);
static void dct64_1(real *out0,real *out1,real *b1,real *b2,real *samples);
static void dct36(real *inbuf,real *o1,real *o2,real *wintab,real *tsbuf);
static void dct12(real *in,real *rawout1,real *rawout2,register real *wi,register real *ts);
static unsigned int getbits(int number_of_bits);
static unsigned int getbits_fast(int number_of_bits);
static unsigned int get1bit(void);

static void mp3play_tilde_tick(t_mp3play_tilde *x);


struct iemmp3Struct *iemmp3_gmp;

int iemmp3_tabsel_123[2][3][16] = {
    { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
    {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
    {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },

    { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
    {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
    {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} }
};

long iemmp3_freqs[9] = { 44100, 48000, 32000,
22050, 24000, 16000 ,
11025 , 12000 , 8000 };

int iemmp3_bitindex;
unsigned char *iemmp3_wordpointer;

static real iemmp3_ispow[8207];
static real iemmp3_aa_ca[8],iemmp3_aa_cs[8];
static real iemmp3_COS1[12][6];
static real iemmp3_win[4][36];
static real iemmp3_win1[4][36];
static real iemmp3_gainpow2[256+118+4];
static real iemmp3_COS9[9];
static real iemmp3_COS6_1,iemmp3_COS6_2;
static real iemmp3_tfcos36[9];
static real iemmp3_tfcos12[3];

int iemmp3_longLimit[9][23];
int iemmp3_shortLimit[9][14];

struct iemmp3_bandInfoStruct bandInfo[9] = {

    /* MPEG 1.0 */
    { {0,4,8,12,16,20,24,30,36,44,52,62,74, 90,110,134,162,196,238,288,342,418,576},
    {4,4,4,4,4,4,6,6,8, 8,10,12,16,20,24,28,34,42,50,54, 76,158},
    {0,4*3,8*3,12*3,16*3,22*3,30*3,40*3,52*3,66*3, 84*3,106*3,136*3,192*3},
    {4,4,4,4,6,8,10,12,14,18,22,30,56} } ,

    { {0,4,8,12,16,20,24,30,36,42,50,60,72, 88,106,128,156,190,230,276,330,384,576},
    {4,4,4,4,4,4,6,6,6, 8,10,12,16,18,22,28,34,40,46,54, 54,192},
    {0,4*3,8*3,12*3,16*3,22*3,28*3,38*3,50*3,64*3, 80*3,100*3,126*3,192*3},
    {4,4,4,4,6,6,10,12,14,16,20,26,66} } ,

    { {0,4,8,12,16,20,24,30,36,44,54,66,82,102,126,156,194,240,296,364,448,550,576} ,
    {4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102, 26} ,
    {0,4*3,8*3,12*3,16*3,22*3,30*3,42*3,58*3,78*3,104*3,138*3,180*3,192*3} ,
    {4,4,4,4,6,8,12,16,20,26,34,42,12} }  ,

    /* MPEG 2.0 */
    { {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54 } ,
    {0,4*3,8*3,12*3,18*3,24*3,32*3,42*3,56*3,74*3,100*3,132*3,174*3,192*3} ,
    {4,4,4,6,6,8,10,14,18,26,32,42,18 } } ,

    { {0,6,12,18,24,30,36,44,54,66,80,96,114,136,162,194,232,278,330,394,464,540,576},
    {6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,52,64,70,76,36 } ,
    {0,4*3,8*3,12*3,18*3,26*3,36*3,48*3,62*3,80*3,104*3,136*3,180*3,192*3} ,
    {4,4,4,6,8,10,12,14,18,24,32,44,12 } } ,

    { {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
    {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54 },
    {0,4*3,8*3,12*3,18*3,26*3,36*3,48*3,62*3,80*3,104*3,134*3,174*3,192*3},
    {4,4,4,6,8,10,12,14,18,24,30,40,18 } } ,
    /* MPEG 2.5 */
    { {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576} ,
    {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54},
    {0,12,24,36,54,78,108,144,186,240,312,402,522,576},
    {4,4,4,6,8,10,12,14,18,24,30,40,18} },
    { {0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576} ,
    {6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54},
    {0,12,24,36,54,78,108,144,186,240,312,402,522,576},
    {4,4,4,6,8,10,12,14,18,24,30,40,18} },
    { {0,12,24,36,48,60,72,88,108,132,160,192,232,280,336,400,476,566,568,570,572,574,576},
    {12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2},
    {0, 24, 48, 72,108,156,216,288,372,480,486,492,498,576},
    {8,8,8,12,16,20,24,28,36,2,2,2,26} } ,
};

static int iemmp3_mapbuf0[9][152];
static int iemmp3_mapbuf1[9][156];
static int iemmp3_mapbuf2[9][44];
static int *iemmp3_map[9][3];
static int *iemmp3_mapend[9][3];

static unsigned int iemmp3_n_slen2[512]; /* MPEG 2.0 slen for 'normal' mode */
static unsigned int iemmp3_slen2[256]; /* MPEG 2.0 slen for intensity stereo */

static real iemmp3_tan1_1[16],iemmp3_tan2_1[16],iemmp3_tan1_2[16],iemmp3_tan2_2[16];
static real iemmp3_pow1_1[2][16],iemmp3_pow2_1[2][16],iemmp3_pow1_2[2][16],iemmp3_pow2_2[2][16];

static int iemmp3_pretab1[22] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};
static int iemmp3_pretab2[22] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

real iemmp3_decwin[512+32];
static real iemmp3_cos64[16],iemmp3_cos32[8],iemmp3_cos16[4],iemmp3_cos8[2],iemmp3_cos4[1];
real *iemmp3_pnts[] = { iemmp3_cos64,iemmp3_cos32,iemmp3_cos16,iemmp3_cos8,iemmp3_cos4 };

static long iemmp3_intwinbase[] = {
    0,    -1,    -1,    -1,    -1,    -1,    -1,    -2,    -2,    -2,
    -2,    -3,    -3,    -4,    -4,    -5,    -5,    -6,    -7,    -7,
    -8,    -9,   -10,   -11,   -13,   -14,   -16,   -17,   -19,   -21,
    -24,   -26,   -29,   -31,   -35,   -38,   -41,   -45,   -49,   -53,
    -58,   -63,   -68,   -73,   -79,   -85,   -91,   -97,  -104,  -111,
    -117,  -125,  -132,  -139,  -147,  -154,  -161,  -169,  -176,  -183,
    -190,  -196,  -202,  -208,  -213,  -218,  -222,  -225,  -227,  -228,
    -228,  -227,  -224,  -221,  -215,  -208,  -200,  -189,  -177,  -163,
    -146,  -127,  -106,   -83,   -57,   -29,     2,    36,    72,   111,
    153,   197,   244,   294,   347,   401,   459,   519,   581,   645,
    711,   779,   848,   919,   991,  1064,  1137,  1210,  1283,  1356,
    1428,  1498,  1567,  1634,  1698,  1759,  1817,  1870,  1919,  1962,
    2001,  2032,  2057,  2075,  2085,  2087,  2080,  2063,  2037,  2000,
    1952,  1893,  1822,  1739,  1644,  1535,  1414,  1280,  1131,   970,
    794,   605,   402,   185,   -45,  -288,  -545,  -814, -1095, -1388,
    -1692, -2006, -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788,
    -5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597, -7910, -8209,
    -8491, -8755, -8998, -9219, -9416, -9585, -9727, -9838, -9916, -9959,
    -9966, -9935, -9863, -9750, -9592, -9389, -9139, -8840, -8492, -8092,
    -7640, -7134, -6574, -5959, -5288, -4561, -3776, -2935, -2037, -1082,
    -70,   998,  2122,  3300,  4533,  5818,  7154,  8540,  9975, 11455,
    12980, 14548, 16155, 17799, 19478, 21189, 22929, 24694, 26482, 28289,
    30112, 31947, 33791, 35640, 37489, 39336, 41176, 43006, 44821, 46617,
    48390, 50137, 51853, 53534, 55178, 56778, 58333, 59838, 61289, 62684,
    64019, 65290, 66494, 67629, 68692, 69679, 70590, 71420, 72169, 72835,
    73415, 73908, 74313, 74630, 74856, 74992, 75038 };

static short iemmp3_tab0[] =
{
    0
};

static short iemmp3_tab1[] =
{
    -5,  -3,  -1,  17,   1,  16,   0
};

static short iemmp3_tab2[] =
{
    -15, -11,  -9,  -5,  -3,  -1,  34,   2,  18,  -1,  33,  32,  17,  -1,   1,
    16,   0
};

static short iemmp3_tab3[] =
{
    -13, -11,  -9,  -5,  -3,  -1,  34,   2,  18,  -1,  33,  32,  16,  17,  -1,
    1,   0
};

static short iemmp3_tab5[] =
{
    -29, -25, -23, -15,  -7,  -5,  -3,  -1,  51,  35,  50,  49,  -3,  -1,  19,
    3,  -1,  48,  34,  -3,  -1,  18,  33,  -1,   2,  32,  17,  -1,   1,  16,
    0
};

static short iemmp3_tab6[] =
{
    -25, -19, -13,  -9,  -5,  -3,  -1,  51,   3,  35,  -1,  50,  48,  -1,  19,
    49,  -3,  -1,  34,   2,  18,  -3,  -1,  33,  32,   1,  -1,  17,  -1,  16,
    0
};

static short iemmp3_tab7[] =
{
    -69, -65, -57, -39, -29, -17, -11,  -7,  -3,  -1,  85,  69,  -1,  84,  83,
    -1,  53,  68,  -3,  -1,  37,  82,  21,  -5,  -1,  81,  -1,   5,  52,  -1,
    80,  -1,  67,  51,  -5,  -3,  -1,  36,  66,  20,  -1,  65,  64, -11,  -7,
    -3,  -1,   4,  35,  -1,  50,   3,  -1,  19,  49,  -3,  -1,  48,  34,  18,
    -5,  -1,  33,  -1,   2,  32,  17,  -1,   1,  16,   0
};

static short iemmp3_tab8[] =
{
    -65, -63, -59, -45, -31, -19, -13,  -7,  -5,  -3,  -1,  85,  84,  69,  83,
    -3,  -1,  53,  68,  37,  -3,  -1,  82,   5,  21,  -5,  -1,  81,  -1,  52,
    67,  -3,  -1,  80,  51,  36,  -5,  -3,  -1,  66,  20,  65,  -3,  -1,   4,
    64,  -1,  35,  50,  -9,  -7,  -3,  -1,  19,  49,  -1,   3,  48,  34,  -1,
    2,  32,  -1,  18,  33,  17,  -3,  -1,   1,  16,   0
};

static short iemmp3_tab9[] =
{
    -63, -53, -41, -29, -19, -11,  -5,  -3,  -1,  85,  69,  53,  -1,  83,  -1,
    84,   5,  -3,  -1,  68,  37,  -1,  82,  21,  -3,  -1,  81,  52,  -1,  67,
    -1,  80,   4,  -7,  -3,  -1,  36,  66,  -1,  51,  64,  -1,  20,  65,  -5,
    -3,  -1,  35,  50,  19,  -1,  49,  -1,   3,  48,  -5,  -3,  -1,  34,   2,
    18,  -1,  33,  32,  -3,  -1,  17,   1,  -1,  16,   0
};

static short iemmp3_tab10[] =
{
    -125,-121,-111, -83, -55, -35, -21, -13,  -7,  -3,  -1, 119, 103,  -1, 118,
    87,  -3,  -1, 117, 102,  71,  -3,  -1, 116,  86,  -1, 101,  55,  -9,  -3,
    -1, 115,  70,  -3,  -1,  85,  84,  99,  -1,  39, 114, -11,  -5,  -3,  -1,
    100,   7, 112,  -1,  98,  -1,  69,  53,  -5,  -1,   6,  -1,  83,  68,  23,
    -17,  -5,  -1, 113,  -1,  54,  38,  -5,  -3,  -1,  37,  82,  21,  -1,  81,
    -1,  52,  67,  -3,  -1,  22,  97,  -1,  96,  -1,   5,  80, -19, -11,  -7,
    -3,  -1,  36,  66,  -1,  51,   4,  -1,  20,  65,  -3,  -1,  64,  35,  -1,
    50,   3,  -3,  -1,  19,  49,  -1,  48,  34,  -7,  -3,  -1,  18,  33,  -1,
    2,  32,  17,  -1,   1,  16,   0
};

static short iemmp3_tab11[] =
{
    -121,-113, -89, -59, -43, -27, -17,  -7,  -3,  -1, 119, 103,  -1, 118, 117,
    -3,  -1, 102,  71,  -1, 116,  -1,  87,  85,  -5,  -3,  -1,  86, 101,  55,
    -1, 115,  70,  -9,  -7,  -3,  -1,  69,  84,  -1,  53,  83,  39,  -1, 114,
    -1, 100,   7,  -5,  -1, 113,  -1,  23, 112,  -3,  -1,  54,  99,  -1,  96,
    -1,  68,  37, -13,  -7,  -5,  -3,  -1,  82,   5,  21,  98,  -3,  -1,  38,
    6,  22,  -5,  -1,  97,  -1,  81,  52,  -5,  -1,  80,  -1,  67,  51,  -1,
    36,  66, -15, -11,  -7,  -3,  -1,  20,  65,  -1,   4,  64,  -1,  35,  50,
    -1,  19,  49,  -5,  -3,  -1,   3,  48,  34,  33,  -5,  -1,  18,  -1,   2,
    32,  17,  -3,  -1,   1,  16,   0
};

static short iemmp3_tab12[] =
{
    -115, -99, -73, -45, -27, -17,  -9,  -5,  -3,  -1, 119, 103, 118,  -1,  87,
    117,  -3,  -1, 102,  71,  -1, 116, 101,  -3,  -1,  86,  55,  -3,  -1, 115,
    85,  39,  -7,  -3,  -1, 114,  70,  -1, 100,  23,  -5,  -1, 113,  -1,   7,
    112,  -1,  54,  99, -13,  -9,  -3,  -1,  69,  84,  -1,  68,  -1,   6,   5,
    -1,  38,  98,  -5,  -1,  97,  -1,  22,  96,  -3,  -1,  53,  83,  -1,  37,
    82, -17,  -7,  -3,  -1,  21,  81,  -1,  52,  67,  -5,  -3,  -1,  80,   4,
    36,  -1,  66,  20,  -3,  -1,  51,  65,  -1,  35,  50, -11,  -7,  -5,  -3,
    -1,  64,   3,  48,  19,  -1,  49,  34,  -1,  18,  33,  -7,  -5,  -3,  -1,
    2,  32,   0,  17,  -1,   1,  16
};

static short iemmp3_tab13[] =
{
    -509,-503,-475,-405,-333,-265,-205,-153,-115, -83, -53, -35, -21, -13,  -9,
    -7,  -5,  -3,  -1, 254, 252, 253, 237, 255,  -1, 239, 223,  -3,  -1, 238,
    207,  -1, 222, 191,  -9,  -3,  -1, 251, 206,  -1, 220,  -1, 175, 233,  -1,
    236, 221,  -9,  -5,  -3,  -1, 250, 205, 190,  -1, 235, 159,  -3,  -1, 249,
    234,  -1, 189, 219, -17,  -9,  -3,  -1, 143, 248,  -1, 204,  -1, 174, 158,
    -5,  -1, 142,  -1, 127, 126, 247,  -5,  -1, 218,  -1, 173, 188,  -3,  -1,
    203, 246, 111, -15,  -7,  -3,  -1, 232,  95,  -1, 157, 217,  -3,  -1, 245,
    231,  -1, 172, 187,  -9,  -3,  -1,  79, 244,  -3,  -1, 202, 230, 243,  -1,
    63,  -1, 141, 216, -21,  -9,  -3,  -1,  47, 242,  -3,  -1, 110, 156,  15,
    -5,  -3,  -1, 201,  94, 171,  -3,  -1, 125, 215,  78, -11,  -5,  -3,  -1,
    200, 214,  62,  -1, 185,  -1, 155, 170,  -1,  31, 241, -23, -13,  -5,  -1,
    240,  -1, 186, 229,  -3,  -1, 228, 140,  -1, 109, 227,  -5,  -1, 226,  -1,
    46,  14,  -1,  30, 225, -15,  -7,  -3,  -1, 224,  93,  -1, 213, 124,  -3,
    -1, 199,  77,  -1, 139, 184,  -7,  -3,  -1, 212, 154,  -1, 169, 108,  -1,
    198,  61, -37, -21,  -9,  -5,  -3,  -1, 211, 123,  45,  -1, 210,  29,  -5,
    -1, 183,  -1,  92, 197,  -3,  -1, 153, 122, 195,  -7,  -5,  -3,  -1, 167,
    151,  75, 209,  -3,  -1,  13, 208,  -1, 138, 168, -11,  -7,  -3,  -1,  76,
    196,  -1, 107, 182,  -1,  60,  44,  -3,  -1, 194,  91,  -3,  -1, 181, 137,
    28, -43, -23, -11,  -5,  -1, 193,  -1, 152,  12,  -1, 192,  -1, 180, 106,
    -5,  -3,  -1, 166, 121,  59,  -1, 179,  -1, 136,  90, -11,  -5,  -1,  43,
    -1, 165, 105,  -1, 164,  -1, 120, 135,  -5,  -1, 148,  -1, 119, 118, 178,
    -11,  -3,  -1,  27, 177,  -3,  -1,  11, 176,  -1, 150,  74,  -7,  -3,  -1,
    58, 163,  -1,  89, 149,  -1,  42, 162, -47, -23,  -9,  -3,  -1,  26, 161,
    -3,  -1,  10, 104, 160,  -5,  -3,  -1, 134,  73, 147,  -3,  -1,  57,  88,
    -1, 133, 103,  -9,  -3,  -1,  41, 146,  -3,  -1,  87, 117,  56,  -5,  -1,
    131,  -1, 102,  71,  -3,  -1, 116,  86,  -1, 101, 115, -11,  -3,  -1,  25,
    145,  -3,  -1,   9, 144,  -1,  72, 132,  -7,  -5,  -1, 114,  -1,  70, 100,
    40,  -1, 130,  24, -41, -27, -11,  -5,  -3,  -1,  55,  39,  23,  -1, 113,
    -1,  85,   7,  -7,  -3,  -1, 112,  54,  -1,  99,  69,  -3,  -1,  84,  38,
    -1,  98,  53,  -5,  -1, 129,  -1,   8, 128,  -3,  -1,  22,  97,  -1,   6,
    96, -13,  -9,  -5,  -3,  -1,  83,  68,  37,  -1,  82,   5,  -1,  21,  81,
    -7,  -3,  -1,  52,  67,  -1,  80,  36,  -3,  -1,  66,  51,  20, -19, -11,
    -5,  -1,  65,  -1,   4,  64,  -3,  -1,  35,  50,  19,  -3,  -1,  49,   3,
    -1,  48,  34,  -3,  -1,  18,  33,  -1,   2,  32,  -3,  -1,  17,   1,  16,
    0
};

static short iemmp3_tab15[] =
{
    -495,-445,-355,-263,-183,-115, -77, -43, -27, -13,  -7,  -3,  -1, 255, 239,
    -1, 254, 223,  -1, 238,  -1, 253, 207,  -7,  -3,  -1, 252, 222,  -1, 237,
    191,  -1, 251,  -1, 206, 236,  -7,  -3,  -1, 221, 175,  -1, 250, 190,  -3,
    -1, 235, 205,  -1, 220, 159, -15,  -7,  -3,  -1, 249, 234,  -1, 189, 219,
    -3,  -1, 143, 248,  -1, 204, 158,  -7,  -3,  -1, 233, 127,  -1, 247, 173,
    -3,  -1, 218, 188,  -1, 111,  -1, 174,  15, -19, -11,  -3,  -1, 203, 246,
    -3,  -1, 142, 232,  -1,  95, 157,  -3,  -1, 245, 126,  -1, 231, 172,  -9,
    -3,  -1, 202, 187,  -3,  -1, 217, 141,  79,  -3,  -1, 244,  63,  -1, 243,
    216, -33, -17,  -9,  -3,  -1, 230,  47,  -1, 242,  -1, 110, 240,  -3,  -1,
    31, 241,  -1, 156, 201,  -7,  -3,  -1,  94, 171,  -1, 186, 229,  -3,  -1,
    125, 215,  -1,  78, 228, -15,  -7,  -3,  -1, 140, 200,  -1,  62, 109,  -3,
    -1, 214, 227,  -1, 155, 185,  -7,  -3,  -1,  46, 170,  -1, 226,  30,  -5,
    -1, 225,  -1,  14, 224,  -1,  93, 213, -45, -25, -13,  -7,  -3,  -1, 124,
    199,  -1,  77, 139,  -1, 212,  -1, 184, 154,  -7,  -3,  -1, 169, 108,  -1,
    198,  61,  -1, 211, 210,  -9,  -5,  -3,  -1,  45,  13,  29,  -1, 123, 183,
    -5,  -1, 209,  -1,  92, 208,  -1, 197, 138, -17,  -7,  -3,  -1, 168,  76,
    -1, 196, 107,  -5,  -1, 182,  -1, 153,  12,  -1,  60, 195,  -9,  -3,  -1,
    122, 167,  -1, 166,  -1, 192,  11,  -1, 194,  -1,  44,  91, -55, -29, -15,
    -7,  -3,  -1, 181,  28,  -1, 137, 152,  -3,  -1, 193,  75,  -1, 180, 106,
    -5,  -3,  -1,  59, 121, 179,  -3,  -1, 151, 136,  -1,  43,  90, -11,  -5,
    -1, 178,  -1, 165,  27,  -1, 177,  -1, 176, 105,  -7,  -3,  -1, 150,  74,
    -1, 164, 120,  -3,  -1, 135,  58, 163, -17,  -7,  -3,  -1,  89, 149,  -1,
    42, 162,  -3,  -1,  26, 161,  -3,  -1,  10, 160, 104,  -7,  -3,  -1, 134,
    73,  -1, 148,  57,  -5,  -1, 147,  -1, 119,   9,  -1,  88, 133, -53, -29,
    -13,  -7,  -3,  -1,  41, 103,  -1, 118, 146,  -1, 145,  -1,  25, 144,  -7,
    -3,  -1,  72, 132,  -1,  87, 117,  -3,  -1,  56, 131,  -1, 102,  71,  -7,
    -3,  -1,  40, 130,  -1,  24, 129,  -7,  -3,  -1, 116,   8,  -1, 128,  86,
    -3,  -1, 101,  55,  -1, 115,  70, -17,  -7,  -3,  -1,  39, 114,  -1, 100,
    23,  -3,  -1,  85, 113,  -3,  -1,   7, 112,  54,  -7,  -3,  -1,  99,  69,
    -1,  84,  38,  -3,  -1,  98,  22,  -3,  -1,   6,  96,  53, -33, -19,  -9,
    -5,  -1,  97,  -1,  83,  68,  -1,  37,  82,  -3,  -1,  21,  81,  -3,  -1,
    5,  80,  52,  -7,  -3,  -1,  67,  36,  -1,  66,  51,  -1,  65,  -1,  20,
    4,  -9,  -3,  -1,  35,  50,  -3,  -1,  64,   3,  19,  -3,  -1,  49,  48,
    34,  -9,  -7,  -3,  -1,  18,  33,  -1,   2,  32,  17,  -3,  -1,   1,  16,
    0
};

static short iemmp3_tab16[] =
{
    -509,-503,-461,-323,-103, -37, -27, -15,  -7,  -3,  -1, 239, 254,  -1, 223,
    253,  -3,  -1, 207, 252,  -1, 191, 251,  -5,  -1, 175,  -1, 250, 159,  -3,
    -1, 249, 248, 143,  -7,  -3,  -1, 127, 247,  -1, 111, 246, 255,  -9,  -5,
    -3,  -1,  95, 245,  79,  -1, 244, 243, -53,  -1, 240,  -1,  63, -29, -19,
    -13,  -7,  -5,  -1, 206,  -1, 236, 221, 222,  -1, 233,  -1, 234, 217,  -1,
    238,  -1, 237, 235,  -3,  -1, 190, 205,  -3,  -1, 220, 219, 174, -11,  -5,
    -1, 204,  -1, 173, 218,  -3,  -1, 126, 172, 202,  -5,  -3,  -1, 201, 125,
    94, 189, 242, -93,  -5,  -3,  -1,  47,  15,  31,  -1, 241, -49, -25, -13,
    -5,  -1, 158,  -1, 188, 203,  -3,  -1, 142, 232,  -1, 157, 231,  -7,  -3,
    -1, 187, 141,  -1, 216, 110,  -1, 230, 156, -13,  -7,  -3,  -1, 171, 186,
    -1, 229, 215,  -1,  78,  -1, 228, 140,  -3,  -1, 200,  62,  -1, 109,  -1,
    214, 155, -19, -11,  -5,  -3,  -1, 185, 170, 225,  -1, 212,  -1, 184, 169,
    -5,  -1, 123,  -1, 183, 208, 227,  -7,  -3,  -1,  14, 224,  -1,  93, 213,
    -3,  -1, 124, 199,  -1,  77, 139, -75, -45, -27, -13,  -7,  -3,  -1, 154,
    108,  -1, 198,  61,  -3,  -1,  92, 197,  13,  -7,  -3,  -1, 138, 168,  -1,
    153,  76,  -3,  -1, 182, 122,  60, -11,  -5,  -3,  -1,  91, 137,  28,  -1,
    192,  -1, 152, 121,  -1, 226,  -1,  46,  30, -15,  -7,  -3,  -1, 211,  45,
    -1, 210, 209,  -5,  -1,  59,  -1, 151, 136,  29,  -7,  -3,  -1, 196, 107,
    -1, 195, 167,  -1,  44,  -1, 194, 181, -23, -13,  -7,  -3,  -1, 193,  12,
    -1,  75, 180,  -3,  -1, 106, 166, 179,  -5,  -3,  -1,  90, 165,  43,  -1,
    178,  27, -13,  -5,  -1, 177,  -1,  11, 176,  -3,  -1, 105, 150,  -1,  74,
    164,  -5,  -3,  -1, 120, 135, 163,  -3,  -1,  58,  89,  42, -97, -57, -33,
    -19, -11,  -5,  -3,  -1, 149, 104, 161,  -3,  -1, 134, 119, 148,  -5,  -3,
    -1,  73,  87, 103, 162,  -5,  -1,  26,  -1,  10, 160,  -3,  -1,  57, 147,
    -1,  88, 133,  -9,  -3,  -1,  41, 146,  -3,  -1, 118,   9,  25,  -5,  -1,
    145,  -1, 144,  72,  -3,  -1, 132, 117,  -1,  56, 131, -21, -11,  -5,  -3,
    -1, 102,  40, 130,  -3,  -1,  71, 116,  24,  -3,  -1, 129, 128,  -3,  -1,
    8,  86,  55,  -9,  -5,  -1, 115,  -1, 101,  70,  -1,  39, 114,  -5,  -3,
    -1, 100,  85,   7,  23, -23, -13,  -5,  -1, 113,  -1, 112,  54,  -3,  -1,
    99,  69,  -1,  84,  38,  -3,  -1,  98,  22,  -1,  97,  -1,   6,  96,  -9,
    -5,  -1,  83,  -1,  53,  68,  -1,  37,  82,  -1,  81,  -1,  21,   5, -33,
    -23, -13,  -7,  -3,  -1,  52,  67,  -1,  80,  36,  -3,  -1,  66,  51,  20,
    -5,  -1,  65,  -1,   4,  64,  -1,  35,  50,  -3,  -1,  19,  49,  -3,  -1,
    3,  48,  34,  -3,  -1,  18,  33,  -1,   2,  32,  -3,  -1,  17,   1,  16,
    0
};

static short iemmp3_tab24[] =
{
    -451,-117, -43, -25, -15,  -7,  -3,  -1, 239, 254,  -1, 223, 253,  -3,  -1,
    207, 252,  -1, 191, 251,  -5,  -1, 250,  -1, 175, 159,  -1, 249, 248,  -9,
    -5,  -3,  -1, 143, 127, 247,  -1, 111, 246,  -3,  -1,  95, 245,  -1,  79,
    244, -71,  -7,  -3,  -1,  63, 243,  -1,  47, 242,  -5,  -1, 241,  -1,  31,
    240, -25,  -9,  -1,  15,  -3,  -1, 238, 222,  -1, 237, 206,  -7,  -3,  -1,
    236, 221,  -1, 190, 235,  -3,  -1, 205, 220,  -1, 174, 234, -15,  -7,  -3,
    -1, 189, 219,  -1, 204, 158,  -3,  -1, 233, 173,  -1, 218, 188,  -7,  -3,
    -1, 203, 142,  -1, 232, 157,  -3,  -1, 217, 126,  -1, 231, 172, 255,-235,
    -143, -77, -45, -25, -15,  -7,  -3,  -1, 202, 187,  -1, 141, 216,  -5,  -3,
    -1,  14, 224,  13, 230,  -5,  -3,  -1, 110, 156, 201,  -1,  94, 186,  -9,
    -5,  -1, 229,  -1, 171, 125,  -1, 215, 228,  -3,  -1, 140, 200,  -3,  -1,
    78,  46,  62, -15,  -7,  -3,  -1, 109, 214,  -1, 227, 155,  -3,  -1, 185,
    170,  -1, 226,  30,  -7,  -3,  -1, 225,  93,  -1, 213, 124,  -3,  -1, 199,
    77,  -1, 139, 184, -31, -15,  -7,  -3,  -1, 212, 154,  -1, 169, 108,  -3,
    -1, 198,  61,  -1, 211,  45,  -7,  -3,  -1, 210,  29,  -1, 123, 183,  -3,
    -1, 209,  92,  -1, 197, 138, -17,  -7,  -3,  -1, 168, 153,  -1,  76, 196,
    -3,  -1, 107, 182,  -3,  -1, 208,  12,  60,  -7,  -3,  -1, 195, 122,  -1,
    167,  44,  -3,  -1, 194,  91,  -1, 181,  28, -57, -35, -19,  -7,  -3,  -1,
    137, 152,  -1, 193,  75,  -5,  -3,  -1, 192,  11,  59,  -3,  -1, 176,  10,
    26,  -5,  -1, 180,  -1, 106, 166,  -3,  -1, 121, 151,  -3,  -1, 160,   9,
    144,  -9,  -3,  -1, 179, 136,  -3,  -1,  43,  90, 178,  -7,  -3,  -1, 165,
    27,  -1, 177, 105,  -1, 150, 164, -17,  -9,  -5,  -3,  -1,  74, 120, 135,
    -1,  58, 163,  -3,  -1,  89, 149,  -1,  42, 162,  -7,  -3,  -1, 161, 104,
    -1, 134, 119,  -3,  -1,  73, 148,  -1,  57, 147, -63, -31, -15,  -7,  -3,
    -1,  88, 133,  -1,  41, 103,  -3,  -1, 118, 146,  -1,  25, 145,  -7,  -3,
    -1,  72, 132,  -1,  87, 117,  -3,  -1,  56, 131,  -1, 102,  40, -17,  -7,
    -3,  -1, 130,  24,  -1,  71, 116,  -5,  -1, 129,  -1,   8, 128,  -1,  86,
    101,  -7,  -5,  -1,  23,  -1,   7, 112, 115,  -3,  -1,  55,  39, 114, -15,
    -7,  -3,  -1,  70, 100,  -1,  85, 113,  -3,  -1,  54,  99,  -1,  69,  84,
    -7,  -3,  -1,  38,  98,  -1,  22,  97,  -5,  -3,  -1,   6,  96,  53,  -1,
    83,  68, -51, -37, -23, -15,  -9,  -3,  -1,  37,  82,  -1,  21,  -1,   5,
    80,  -1,  81,  -1,  52,  67,  -3,  -1,  36,  66,  -1,  51,  20,  -9,  -5,
    -1,  65,  -1,   4,  64,  -1,  35,  50,  -1,  19,  49,  -7,  -5,  -3,  -1,
    3,  48,  34,  18,  -1,  33,  -1,   2,  32,  -3,  -1,  17,   1,  -1,  16,
    0
};

static short iemmp3_tab_c0[] =
{
    -29, -21, -13,  -7,  -3,  -1,  11,  15,  -1,  13,  14,  -3,  -1,   7,   5,
    9,  -3,  -1,   6,   3,  -1,  10,  12,  -3,  -1,   2,   1,  -1,   4,   8,
    0
};

static short iemmp3_tab_c1[] =
{
    -15,  -7,  -3,  -1,  15,  14,  -1,  13,  12,  -3,  -1,  11,  10,  -1,   9,
    8,  -7,  -3,  -1,   7,   6,  -1,   5,   4,  -3,  -1,   3,   2,  -1,   1,
    0
};



static struct iemmp3_newHuff iemmp3_ht[] =
{
    { /* 0 */ 0 , iemmp3_tab0  } ,
    { /* 2 */ 0 , iemmp3_tab1  } ,
    { /* 3 */ 0 , iemmp3_tab2  } ,
    { /* 3 */ 0 , iemmp3_tab3  } ,
    { /* 0 */ 0 , iemmp3_tab0  } ,
    { /* 4 */ 0 , iemmp3_tab5  } ,
    { /* 4 */ 0 , iemmp3_tab6  } ,
    { /* 6 */ 0 , iemmp3_tab7  } ,
    { /* 6 */ 0 , iemmp3_tab8  } ,
    { /* 6 */ 0 , iemmp3_tab9  } ,
    { /* 8 */ 0 , iemmp3_tab10 } ,
    { /* 8 */ 0 , iemmp3_tab11 } ,
    { /* 8 */ 0 , iemmp3_tab12 } ,
    { /* 16 */ 0 , iemmp3_tab13 } ,
    { /* 0  */ 0 , iemmp3_tab0  } ,
    { /* 16 */ 0 , iemmp3_tab15 } ,

    { /* 16 */ 1 , iemmp3_tab16 } ,
    { /* 16 */ 2 , iemmp3_tab16 } ,
    { /* 16 */ 3 , iemmp3_tab16 } ,
    { /* 16 */ 4 , iemmp3_tab16 } ,
    { /* 16 */ 6 , iemmp3_tab16 } ,
    { /* 16 */ 8 , iemmp3_tab16 } ,
    { /* 16 */ 10, iemmp3_tab16 } ,
    { /* 16 */ 13, iemmp3_tab16 } ,
    { /* 16 */ 4 , iemmp3_tab24 } ,
    { /* 16 */ 5 , iemmp3_tab24 } ,
    { /* 16 */ 6 , iemmp3_tab24 } ,
    { /* 16 */ 7 , iemmp3_tab24 } ,
    { /* 16 */ 8 , iemmp3_tab24 } ,
    { /* 16 */ 9 , iemmp3_tab24 } ,
    { /* 16 */ 11, iemmp3_tab24 } ,
    { /* 16 */ 13, iemmp3_tab24 }
};

static struct iemmp3_newHuff iemmp3_htc[] =
{
    { /* 1 , 1 , */ 0 , iemmp3_tab_c0 } ,
    { /* 1 , 1 , */ 0 , iemmp3_tab_c1 }
};


static t_class *mp3play_tilde_class;



static BOOL InitMP3(struct iemmp3Struct *mp)
{
    memset(mp,0,sizeof(struct iemmp3Struct));

    mp->framesize = 0;
    mp->fsizeold = -1;
    mp->bsize = 0;
    mp->head = mp->tail = NULL;
    mp->fr.single = -1;
    mp->bsnum = 0;
    mp->synth_bo = 1;

    make_decode_tables(32767);
    init_layer3(SBLIMIT);

    return !0;
}

static BOOL InitAgainMP3(struct iemmp3Struct *mp)
{
    memset(mp,0,sizeof(struct iemmp3Struct));

    mp->framesize = 0;
    mp->fsizeold = -1;
    mp->bsize = 0;
    mp->head = mp->tail = NULL;
    mp->fr.single = -1;
    mp->bsnum = 0;
    mp->synth_bo = 1;

    return !0;
}

static void ExitMP3(struct iemmp3Struct *mp)
{
    struct iemmp3_buf *b,*bn;

    b = mp->tail;
    while(b)
    {
  free(b->pnt);
  bn = b->next;
  free(b);
  b = bn;
    }
}

static int decodeMP3(struct iemmp3Struct *mp, char *in, int isize, char *out, int osize, int *done)
{
    int len,err=1;

    iemmp3_gmp = mp;

    if(osize < 4608)
    {
  post("To less out space\n");
  return MP3_ERR;
    }

    if(in)
    {
  if(addbuf(mp,in,isize) == NULL)
  {
      return MP3_ERR;
  }
    }

    /* First decode header */
    if(mp->framesize == 0)
    {
  if(mp->bsize < 4)
  {
      return MP3_NEED_MORE;
  }
  if(!read_head(mp))
      return MP3_EX;
  if(decode_header(&mp->fr,mp->header) == MP3_EX)
      return MP3_EX;
  mp->framesize = mp->fr.framesize;
    }

    if(mp->fr.framesize > mp->bsize)
  return MP3_NEED_MORE;

    iemmp3_wordpointer = mp->bsspace[mp->bsnum] + 512;
    mp->bsnum = (mp->bsnum + 1) & 0x1;
    iemmp3_bitindex = 0;

    len = 0;
    while(len < mp->framesize)
    {
  int nlen;
  int blen = mp->tail->size - mp->tail->pos;
  if( (mp->framesize - len) <= blen)
  {
      nlen = mp->framesize-len;
  }
  else
  {
      nlen = blen;
  }
  memcpy(iemmp3_wordpointer+len,mp->tail->pnt+mp->tail->pos,nlen);
  len += nlen;
  mp->tail->pos += nlen;
  mp->bsize -= nlen;
  if(mp->tail->pos == mp->tail->size)
  {
      remove_buf(mp);
  }
    }

    *done = 0;
    if(mp->fr.error_protection)
  getbits(16);
    do_layer3(&mp->fr,(unsigned char *) out,done,&err);
    if(!err)
  return MP3_EX;

    mp->fsizeold = mp->framesize;
    mp->framesize = 0;

    return MP3_OK;
}

static void make_decode_tables(long scaleval)
{
    int i,j,k,kr,divv;
    real *table,*costab;

    for(i=0;i<5;i++)
    {
  kr=0x10>>i; divv=0x40>>i;
  costab = iemmp3_pnts[i];
  for(k=0;k<kr;k++)
      costab[k] = 1.0 / (2.0 * cos(M_PI * ((double) k * 2.0 + 1.0) / (double) divv));
    }

    table = iemmp3_decwin;
    scaleval = -scaleval;
    for(i=0,j=0;i<256;i++,j++,table+=32)
    {
  if(table < iemmp3_decwin+512+16)
      table[16] = table[0] = (double) iemmp3_intwinbase[j] / 65536.0 * (double) scaleval;
  if(i % 32 == 31)
      table -= 1023;
  if(i % 64 == 63)
      scaleval = - scaleval;
    }

    for( /* i=256 */ ;i<512;i++,j--,table+=32)
    {
  if(table < iemmp3_decwin+512+16)
      table[16] = table[0] = (double) iemmp3_intwinbase[j] / 65536.0 * (double) scaleval;
  if(i % 32 == 31)
      table -= 1023;
  if(i % 64 == 63)
      scaleval = - scaleval;
    }
}


static void init_layer3(int down_sample_sblimit)
{
    int i,j,k,l;

    for(i=-256;i<118+4;i++)
  iemmp3_gainpow2[i+256] = pow((double)2.0,-0.25 * (double) (i+210) );

    for(i=0;i<8207;i++)
  iemmp3_ispow[i] = pow((double)i,(double)4.0/3.0);

    for (i=0;i<8;i++)
    {
  static double Ci[8]={-0.6,-0.535,-0.33,-0.185,-0.095,-0.041,-0.0142,-0.0037};
  double sq=sqrt(1.0+Ci[i]*Ci[i]);
  iemmp3_aa_cs[i] = 1.0/sq;
  iemmp3_aa_ca[i] = Ci[i]/sq;
    }

    for(i=0;i<18;i++)
    {
  iemmp3_win[0][i]    = iemmp3_win[1][i]    = 0.5 * sin( M_PI / 72.0 * (double) (2*(i+0) +1) ) / cos ( M_PI * (double) (2*(i+0) +19) / 72.0 );
  iemmp3_win[0][i+18] = iemmp3_win[3][i+18] = 0.5 * sin( M_PI / 72.0 * (double) (2*(i+18)+1) ) / cos ( M_PI * (double) (2*(i+18)+19) / 72.0 );
    }
    for(i=0;i<6;i++)
    {
  iemmp3_win[1][i+18] = 0.5 / cos ( M_PI * (double) (2*(i+18)+19) / 72.0 );
  iemmp3_win[3][i+12] = 0.5 / cos ( M_PI * (double) (2*(i+12)+19) / 72.0 );
  iemmp3_win[1][i+24] = 0.5 * sin( M_PI / 24.0 * (double) (2*i+13) ) / cos ( M_PI * (double) (2*(i+24)+19) / 72.0 );
  iemmp3_win[1][i+30] = iemmp3_win[3][i] = 0.0;
  iemmp3_win[3][i+6 ] = 0.5 * sin( M_PI / 24.0 * (double) (2*i+1) )  / cos ( M_PI * (double) (2*(i+6 )+19) / 72.0 );
    }

    for(i=0;i<9;i++)
  iemmp3_COS9[i] = cos( M_PI / 18.0 * (double) i);

    for(i=0;i<9;i++)
  iemmp3_tfcos36[i] = 0.5 / cos ( M_PI * (double) (i*2+1) / 36.0 );
    for(i=0;i<3;i++)
  iemmp3_tfcos12[i] = 0.5 / cos ( M_PI * (double) (i*2+1) / 12.0 );

    iemmp3_COS6_1 = cos( M_PI / 6.0 * (double) 1);
    iemmp3_COS6_2 = cos( M_PI / 6.0 * (double) 2);

    for(i=0;i<12;i++)
    {
  iemmp3_win[2][i]  = 0.5 * sin( M_PI / 24.0 * (double) (2*i+1) ) / cos ( M_PI * (double) (2*i+7) / 24.0 );
  for(j=0;j<6;j++)
      iemmp3_COS1[i][j] = cos( M_PI / 24.0 * (double) ((2*i+7)*(2*j+1)) );
    }

    for(j=0;j<4;j++)
    {
  static int len[4] = { 36,36,12,36 };
  for(i=0;i<len[j];i+=2)
      iemmp3_win1[j][i] = + iemmp3_win[j][i];
  for(i=1;i<len[j];i+=2)
      iemmp3_win1[j][i] = - iemmp3_win[j][i];
    }

    for(i=0;i<16;i++)
    {
  double t = tan( (double) i * M_PI / 12.0 );
  iemmp3_tan1_1[i] = t / (1.0+t);
  iemmp3_tan2_1[i] = 1.0 / (1.0 + t);
  iemmp3_tan1_2[i] = M_SQRT2 * t / (1.0+t);
  iemmp3_tan2_2[i] = M_SQRT2 / (1.0 + t);

  for(j=0;j<2;j++)
  {
      double base = pow(2.0,-0.25*(j+1.0));
      double p1=1.0,p2=1.0;
      if(i > 0)
      {
    if( i & 1 )
        p1 = pow(base,(i+1.0)*0.5);
    else
        p2 = pow(base,i*0.5);
      }
      iemmp3_pow1_1[j][i] = p1;
      iemmp3_pow2_1[j][i] = p2;
      iemmp3_pow1_2[j][i] = M_SQRT2 * p1;
      iemmp3_pow2_2[j][i] = M_SQRT2 * p2;
  }
    }

    for(j=0;j<9;j++)
    {
  struct iemmp3_bandInfoStruct *bi = &bandInfo[j];
  int *mp;
  int cb,lwin;
  short *bdf;

  mp = iemmp3_map[j][0] = iemmp3_mapbuf0[j];
  bdf = bi->longDiff;
  for(i=0,cb = 0; cb < 8 ; cb++,i+=*bdf++)
  {
      *mp++ = (*bdf) >> 1;
      *mp++ = i;
      *mp++ = 3;
      *mp++ = cb;
  }
  bdf = bi->shortDiff+3;
  for(cb=3;cb<13;cb++)
  {
      int l1 = (*bdf++) >> 1;
      for(lwin=0;lwin<3;lwin++)
      {
    *mp++ = l1;
    *mp++ = i + lwin;
    *mp++ = lwin;
    *mp++ = cb;
      }
      i += 6*l1;
  }
  iemmp3_mapend[j][0] = mp;

  mp = iemmp3_map[j][1] = iemmp3_mapbuf1[j];
  bdf = bi->shortDiff+0;
  for(i=0,cb=0;cb<13;cb++)
  {
      int l1 = (*bdf++) >> 1;
      for(lwin=0;lwin<3;lwin++)
      {
    *mp++ = l1;
    *mp++ = i + lwin;
    *mp++ = lwin;
    *mp++ = cb;
      }
      i += 6*l1;
  }
  iemmp3_mapend[j][1] = mp;

  mp = iemmp3_map[j][2] = iemmp3_mapbuf2[j];
  bdf = bi->longDiff;
  for(cb = 0; cb < 22 ; cb++)
  {
      *mp++ = (*bdf++) >> 1;
      *mp++ = cb;
  }
  iemmp3_mapend[j][2] = mp;
    }

    for(j=0;j<9;j++)
    {
  for(i=0;i<23;i++)
  {
      iemmp3_longLimit[j][i] = (bandInfo[j].longIdx[i] - 1 + 8) / 18 + 1;
      if(iemmp3_longLimit[j][i] > (down_sample_sblimit) )
    iemmp3_longLimit[j][i] = down_sample_sblimit;
  }
  for(i=0;i<14;i++)
  {
      iemmp3_shortLimit[j][i] = (bandInfo[j].shortIdx[i] - 1) / 18 + 1;
      if(iemmp3_shortLimit[j][i] > (down_sample_sblimit) )
    iemmp3_shortLimit[j][i] = down_sample_sblimit;
  }
    }

    for(i=0;i<5;i++)
    {
  for(j=0;j<6;j++)
  {
      for(k=0;k<6;k++)
      {
    int n = k + j * 6 + i * 36;
    iemmp3_slen2[n] = i|(j<<3)|(k<<6)|(3<<12);
      }
  }
    }
    for(i=0;i<4;i++)
    {
  for(j=0;j<4;j++)
  {
      for(k=0;k<4;k++)
      {
    int n = k + j * 4 + i * 16;
    iemmp3_slen2[n+180] = i|(j<<3)|(k<<6)|(4<<12);
      }
  }
    }
    for(i=0;i<4;i++)
    {
  for(j=0;j<3;j++)
  {
      int n = j + i * 3;
      iemmp3_slen2[n+244] = i|(j<<3) | (5<<12);
      iemmp3_n_slen2[n+500] = i|(j<<3) | (2<<12) | (1<<15);
  }
    }

    for(i=0;i<5;i++)
    {
  for(j=0;j<5;j++)
  {
      for(k=0;k<4;k++)
      {
    for(l=0;l<4;l++)
    {
        int n = l + k * 4 + j * 16 + i * 80;
        iemmp3_n_slen2[n] = i|(j<<3)|(k<<6)|(l<<9)|(0<<12);
    }
      }
  }
    }
    for(i=0;i<5;i++)
    {
  for(j=0;j<5;j++)
  {
      for(k=0;k<4;k++)
      {
    int n = k + j * 4 + i * 20;
    iemmp3_n_slen2[n+400] = i|(j<<3)|(k<<6)|(1<<12);
      }
  }
    }
}

static void remove_buf(struct iemmp3Struct *mp)
{
    struct iemmp3_buf *buf = mp->tail;

    mp->tail = buf->next;
    if(mp->tail)
  mp->tail->prev = NULL;
    else
    {
  mp->tail = mp->head = NULL;
    }
    free(buf->pnt);
    free(buf);
}

static struct iemmp3_buf *addbuf(struct iemmp3Struct *mp,char *buf,int size)
{
    struct iemmp3_buf *nbuf;

    nbuf = malloc( sizeof(struct iemmp3_buf) );
    if(!nbuf)
    {
  post("Out of memory!\n");
  return NULL;
    }
    nbuf->pnt = malloc(size);
    if(!nbuf->pnt)
    {
  free(nbuf);
  return NULL;
    }
    nbuf->size = size;
    memcpy(nbuf->pnt,buf,size);
    nbuf->next = NULL;
    nbuf->prev = mp->head;
    nbuf->pos = 0;
    if(!mp->tail)
    {
  mp->tail = nbuf;
    }
    else
    {
  mp->head->next = nbuf;
    }
    mp->head = nbuf;
    mp->bsize += size;
    return nbuf;
}

static int read_buf_byte(struct iemmp3Struct *mp,int *err)
{
    unsigned int b;
    int pos;

    pos = mp->tail->pos;
    while(pos >= mp->tail->size)
    {
  remove_buf(mp);
  pos = mp->tail->pos;
  if(!mp->tail)
  {
      post("Fatal error!\n");
      *err = 0;
      return(0);
  }
    }
    b = mp->tail->pnt[pos];
    mp->bsize--;
    mp->tail->pos++;
    return(b);
}

static int read_head(struct iemmp3Struct *mp)
{
    unsigned long head;
    int err=1;

    head = read_buf_byte(mp,&err);
    head <<= 8;
    head |= read_buf_byte(mp,&err);
    head <<= 8;
    head |= read_buf_byte(mp,&err);
    head <<= 8;
    head |= read_buf_byte(mp,&err);
    mp->header = head;
    return(err);
}

static int decode_header(struct iemmp3_frame *fr,unsigned long newhead)
{
    if( newhead & (1<<20) )
    {
  fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
  fr->mpeg25 = 0;
    }
    else
    {
  fr->lsf = 1;
  fr->mpeg25 = 1;
    }
    fr->lay = 4-((newhead>>17)&3);
    if( ((newhead>>10)&0x3) == 0x3)
    {
  post("Stream error");
  return(MP3_EX);
    }
    if(fr->mpeg25)
    {
  fr->sampling_frequency = 6 + ((newhead>>10)&0x3);
    }
    else
  fr->sampling_frequency = ((newhead>>10)&0x3) + (fr->lsf*3);
    fr->error_protection = ((newhead>>16)&0x1)^0x1;
    if(fr->mpeg25) /* allow Bitrate change for 2.5 ... */
  fr->bitrate_index = ((newhead>>12)&0xf);
    fr->bitrate_index = ((newhead>>12)&0xf);
    fr->padding   = ((newhead>>9)&0x1);
    fr->extension = ((newhead>>8)&0x1);
    fr->mode      = ((newhead>>6)&0x3);
    fr->mode_ext  = ((newhead>>4)&0x3);
    fr->copyright = ((newhead>>3)&0x1);
    fr->original  = ((newhead>>2)&0x1);
    fr->emphasis  = newhead & 0x3;
    fr->stereo    = (fr->mode == MPG_MD_MONO) ? 1 : 2;
    if(!fr->bitrate_index)
    {
  post("Free format not supported.\n");
  return(0);
    }
    switch(fr->lay)
    {
    case 1:
  post("Layer I not supported!\n");
  break;
    case 2:
  post("Layer II not supported!\n");
  break;
    case 3:
  fr->framesize  = (long) iemmp3_tabsel_123[fr->lsf][2][fr->bitrate_index] * 144000;
  fr->framesize /= iemmp3_freqs[fr->sampling_frequency]<<(fr->lsf);
  fr->framesize = fr->framesize + fr->padding - 4;
  //post("Framesize: %d",fr->framesize);
  break;
    default:
  post("Sorry, unknown layer type.\n");
  return (0);
    }
    return(1);
}

static unsigned int getbits(int number_of_bits)
{
    unsigned long rval;

    if(!number_of_bits)
  return 0;
    {
  rval = iemmp3_wordpointer[0];
  rval <<= 8;
  rval |= iemmp3_wordpointer[1];
  rval <<= 8;
  rval |= iemmp3_wordpointer[2];
  rval <<= iemmp3_bitindex;
  rval &= 0xffffff;
  iemmp3_bitindex += number_of_bits;
  rval >>= (24-number_of_bits);
  iemmp3_wordpointer += (iemmp3_bitindex>>3);
  iemmp3_bitindex &= 7;
    }
    return rval;
}

static unsigned int getbits_fast(int number_of_bits)
{
    unsigned long rval;

    {
  rval = iemmp3_wordpointer[0];
  rval <<= 8;
  rval |= iemmp3_wordpointer[1];
  rval <<= iemmp3_bitindex;
  rval &= 0xffff;
  iemmp3_bitindex += number_of_bits;
  rval >>= (16-number_of_bits);
  iemmp3_wordpointer += (iemmp3_bitindex>>3);
  iemmp3_bitindex &= 7;
    }
    return rval;
}

static unsigned int get1bit(void)
{
    unsigned char rval;
    rval = *iemmp3_wordpointer << iemmp3_bitindex;

    iemmp3_bitindex++;
    iemmp3_wordpointer += (iemmp3_bitindex>>3);
    iemmp3_bitindex &= 7;

    return rval>>7;
}

static int do_layer3(struct iemmp3_frame *fr,unsigned char *pcm_sample,int *pcm_point,int *err)
{
    int gr, ch, ss,clip=0;
    int scalefacs[39]; /* max 39 for short[13][3] mode, mixed: 38, long: 22 */
    struct iemmp3_III_sideinfo sideinfo;
    int stereo = fr->stereo;
    int single = fr->single;
    int ms_stereo,i_stereo;
    int sfreq = fr->sampling_frequency;
    int stereo1,granules;

    if(stereo == 1)
    { /* stream is mono */
  stereo1 = 1;
  single = 0;
    }
    else if(single >= 0) /* stream is stereo, but force to mono */
  stereo1 = 1;
    else
  stereo1 = 2;

    if(fr->mode == MPG_MD_JOINT_STEREO)
    {
  ms_stereo = fr->mode_ext & 0x2;
  i_stereo  = fr->mode_ext & 0x1;
    }
    else
  ms_stereo = i_stereo = 0;

    if(fr->lsf)
    {
  granules = 1;
  if(!III_get_side_info_2(&sideinfo,stereo,ms_stereo,sfreq,single))
  {
      *err = 0;
      return 0;
  }
    }
    else
    {
  granules = 2;
#ifdef IEM_MPEG1
  if(!III_get_side_info_1(&sideinfo,stereo,ms_stereo,sfreq,single))
  {
      *err = 0;
      return 0;
  }
#else
  post("Not supported\n");
#endif
    }

    if(set_pointer(sideinfo.main_data_begin) == MP3_ERR)
  return 0;

    for (gr=0;gr<granules;gr++)
    {
  static real hybridIn[2][SBLIMIT][SSLIMIT];
  static real hybridOut[2][SSLIMIT][SBLIMIT];

  {
      struct iemmp3_gr_info_s *gr_info = &(sideinfo.ch[0].gr[gr]);
      long part2bits;
      if(fr->lsf)
    part2bits = III_get_scale_factors_2(scalefacs,gr_info,0);
      else
      {
#ifdef IEM_MPEG1
    part2bits = III_get_scale_factors_1(scalefacs,gr_info);
#else
    post("Not supported\n");
#endif
      }
      if(III_dequantize_sample(hybridIn[0], scalefacs,gr_info,sfreq,part2bits))
    return clip;
  }
  if(stereo == 2)
  {
      struct iemmp3_gr_info_s *gr_info = &(sideinfo.ch[1].gr[gr]);
      long part2bits;
      if(fr->lsf)
    part2bits = III_get_scale_factors_2(scalefacs,gr_info,i_stereo);
      else
      {
#ifdef IEM_MPEG1
    part2bits = III_get_scale_factors_1(scalefacs,gr_info);
#else
    post("Not supported\n");
#endif
      }

      if(III_dequantize_sample(hybridIn[1],scalefacs,gr_info,sfreq,part2bits))
    return clip;

      if(ms_stereo)
      {
    int i;
    for(i=0;i<SBLIMIT*SSLIMIT;i++)
    {
        real tmp0,tmp1;
        tmp0 = ((real *) hybridIn[0])[i];
        tmp1 = ((real *) hybridIn[1])[i];
        ((real *) hybridIn[1])[i] = tmp0 - tmp1;
        ((real *) hybridIn[0])[i] = tmp0 + tmp1;
    }
      }

      if(i_stereo)
    III_i_stereo(hybridIn,scalefacs,gr_info,sfreq,ms_stereo,fr->lsf);

      if(ms_stereo || i_stereo || (single == 3) )
      {
    if(gr_info->maxb > sideinfo.ch[0].gr[gr].maxb)
        sideinfo.ch[0].gr[gr].maxb = gr_info->maxb;
    else
        gr_info->maxb = sideinfo.ch[0].gr[gr].maxb;
      }

      switch(single)
      {
      case 3:
    {
        register unsigned int i;
        register real *in0 = (real *) hybridIn[0],*in1 = (real *) hybridIn[1];
        for(i=0;i<SSLIMIT*gr_info->maxb;i++,in0++)
      *in0 = (*in0 + *in1++); /* *0.5 done by pow-scale */
    }
    break;
      case 1:
    {
        register unsigned int i;
        register real *in0 = (real *) hybridIn[0],*in1 = (real *) hybridIn[1];
        for(i=0;i<SSLIMIT*gr_info->maxb;i++)
      *in0++ = *in1++;
    }
    break;
      }
  }

  for(ch=0;ch<stereo1;ch++)
  {
      struct iemmp3_gr_info_s *gr_info = &(sideinfo.ch[ch].gr[gr]);
      III_antialias(hybridIn[ch],gr_info);
      III_hybrid(hybridIn[ch], hybridOut[ch], ch,gr_info);
  }

  for(ss=0;ss<SSLIMIT;ss++)
  {
      if(single >= 0)
      {
    clip += synth_1to1_mono(hybridOut[0][ss],pcm_sample,pcm_point);
      }
      else
      {
    int p1 = *pcm_point;
    clip += synth_1to1(hybridOut[0][ss],0,pcm_sample,&p1);
    clip += synth_1to1(hybridOut[1][ss],1,pcm_sample,pcm_point);
      }
  }
    }

    return clip;
}

static int III_get_side_info_2(struct iemmp3_III_sideinfo *si,int stereo,
             int ms_stereo,long sfreq,int single)
{
    int ch;
    int powdiff = (single == 3) ? 4 : 0;

    si->main_data_begin = getbits(8);
    if (stereo == 1)
  si->private_bits = get1bit();
    else
  si->private_bits = getbits_fast(2);

    for (ch=0; ch<stereo; ch++)
    {
  register struct iemmp3_gr_info_s *gr_info = &(si->ch[ch].gr[0]);

  gr_info->part2_3_length = getbits(12);
  gr_info->big_values = getbits_fast(9);
  if(gr_info->big_values > 288)
  {
      post("big_values too large!\n");
      gr_info->big_values = 288;
  }
  gr_info->pow2gain = iemmp3_gainpow2+256 - getbits_fast(8) + powdiff;
  if(ms_stereo)
      gr_info->pow2gain += 2;
  gr_info->scalefac_compress = getbits(9);
  /* window-switching flag == 1 for block_Type != 0 .. and block-type == 0 -> win-sw-flag = 0 */
  if(get1bit())
  {
      int i;
      gr_info->block_type = getbits_fast(2);
      gr_info->mixed_block_flag = get1bit();
      gr_info->table_select[0] = getbits_fast(5);
      gr_info->table_select[1] = getbits_fast(5);
      /*
       * table_select[2] not needed, because there is no region2,
       * but to satisfy some verifications tools we set it either.
       */
      gr_info->table_select[2] = 0;
      for(i=0;i<3;i++)
    gr_info->full_gain[i] = gr_info->pow2gain + (getbits_fast(3)<<3);
      if(gr_info->block_type == 0)
      {
    post("Blocktype == 0 and window-switching == 1 not allowed.\n");
    return(0);
      }
      /* region_count/start parameters are implicit in this case. */
      /* check this again! */
      if(gr_info->block_type == 2)
    gr_info->region1start = 36>>1;
      else if(sfreq == 8)
    /* check this for 2.5 and sfreq=8 */
    gr_info->region1start = 108>>1;
      else
    gr_info->region1start = 54>>1;
      gr_info->region2start = 576>>1;
  }
  else
  {
      int i,r0c,r1c;
      for (i=0; i<3; i++)
    gr_info->table_select[i] = getbits_fast(5);
      r0c = getbits_fast(4);
      r1c = getbits_fast(3);
      gr_info->region1start = bandInfo[sfreq].longIdx[r0c+1] >> 1 ;
      gr_info->region2start = bandInfo[sfreq].longIdx[r0c+1+r1c+1] >> 1;
      gr_info->block_type = 0;
      gr_info->mixed_block_flag = 0;
  }
  gr_info->scalefac_scale = get1bit();
  gr_info->count1table_select = get1bit();
    }
    return(1);
}


#ifdef IEM_MPEG1
static int III_get_side_info_1(struct iemmp3_III_sideinfo *si,int stereo,
             int ms_stereo,long sfreq,int single)
{
    int ch, gr;
    int powdiff = (single == 3) ? 4 : 0;

    si->main_data_begin = getbits(9);
    if (stereo == 1)
  si->private_bits = getbits_fast(5);
    else
  si->private_bits = getbits_fast(3);

    for (ch=0; ch<stereo; ch++)
    {
  si->ch[ch].gr[0].scfsi = -1;
  si->ch[ch].gr[1].scfsi = getbits_fast(4);
    }

    for (gr=0; gr<2; gr++)
    {
  for (ch=0; ch<stereo; ch++)
  {
      register struct iemmp3_gr_info_s *gr_info = &(si->ch[ch].gr[gr]);

      gr_info->part2_3_length = getbits(12);
      gr_info->big_values = getbits_fast(9);
      if(gr_info->big_values > 288)
      {
    post("big_values too large!\n");
    gr_info->big_values = 288;
      }
      gr_info->pow2gain = iemmp3_gainpow2+256 - getbits_fast(8) + powdiff;
      if(ms_stereo)
    gr_info->pow2gain += 2;
      gr_info->scalefac_compress = getbits_fast(4);
      /* window-switching flag == 1 for block_Type != 0 .. and block-type == 0 -> win-sw-flag = 0 */
      if(get1bit())
      {
    int i;

    gr_info->block_type = getbits_fast(2);
    gr_info->mixed_block_flag = get1bit();
    gr_info->table_select[0] = getbits_fast(5);
    gr_info->table_select[1] = getbits_fast(5);
    /*
     * table_select[2] not needed, because there is no region2,
     * but to satisfy some verifications tools we set it either.
     */
    gr_info->table_select[2] = 0;
    for(i=0;i<3;i++)
        gr_info->full_gain[i] = gr_info->pow2gain + (getbits_fast(3)<<3);

    if(gr_info->block_type == 0)
    {
        post("Blocktype == 0 and window-switching == 1 not allowed.\n");
        return(0);
    }
    /* region_count/start parameters are implicit in this case. */
    gr_info->region1start = 36>>1;
    gr_info->region2start = 576>>1;
      }
      else
      {
    int i,r0c,r1c;
    for (i=0; i<3; i++)
        gr_info->table_select[i] = getbits_fast(5);
    r0c = getbits_fast(4);
    r1c = getbits_fast(3);
    gr_info->region1start = bandInfo[sfreq].longIdx[r0c+1] >> 1 ;
    gr_info->region2start = bandInfo[sfreq].longIdx[r0c+1+r1c+1] >> 1;
    gr_info->block_type = 0;
    gr_info->mixed_block_flag = 0;
      }
      gr_info->preflag = get1bit();
      gr_info->scalefac_scale = get1bit();
      gr_info->count1table_select = get1bit();
  }
    }
    return(1);
}
#endif

static int set_pointer(long backstep)
{
    unsigned char *bsbufold;
    if(iemmp3_gmp->fsizeold < 0 && backstep > 0)
    {
  post("Can't step back %ld!\n",backstep);
  return MP3_ERR;
    }
    bsbufold = iemmp3_gmp->bsspace[iemmp3_gmp->bsnum] + 512;
    iemmp3_wordpointer -= backstep;
    if (backstep)
  memcpy(iemmp3_wordpointer,bsbufold+iemmp3_gmp->fsizeold-backstep,backstep);
    iemmp3_bitindex = 0;
    return MP3_OK;
}

#ifdef IEM_MPEG1
static int III_get_scale_factors_1(int *scf,struct iemmp3_gr_info_s *gr_info)
{
    static unsigned char slen[2][16] = {
  {0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
  {0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}
    };
    int numbits;
    int num0 = slen[0][gr_info->scalefac_compress];
    int num1 = slen[1][gr_info->scalefac_compress];

    if (gr_info->block_type == 2)
    {
  int i=18;

  numbits = (num0 + num1) * 18;
  if (gr_info->mixed_block_flag)
  {
      for (i=8;i;i--)
    *scf++ = getbits_fast(num0);
      i = 9;
      numbits -= num0; /* num0 * 17 + num1 * 18 */
  }

  for (;i;i--)
      *scf++ = getbits_fast(num0);
  for (i = 18; i; i--)
      *scf++ = getbits_fast(num1);
  *scf++ = 0; *scf++ = 0; *scf++ = 0; /* short[13][0..2] = 0 */
    }
    else
    {
  int i;
  int scfsi = gr_info->scfsi;

  if(scfsi < 0)
  { /* scfsi < 0 => granule == 0 */
      for(i=11;i;i--)
    *scf++ = getbits_fast(num0);
      for(i=10;i;i--)
    *scf++ = getbits_fast(num1);
      numbits = (num0 + num1) * 10 + num0;
  }
  else
  {
      numbits = 0;
      if(!(scfsi & 0x8))
      {
    for (i=6;i;i--)
        *scf++ = getbits_fast(num0);
    numbits += num0 * 6;
      }
      else
      {
    *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
    *scf++ = 0; *scf++ = 0; *scf++ = 0;
      }

      if(!(scfsi & 0x4))
      {
    for (i=5;i;i--)
        *scf++ = getbits_fast(num0);
    numbits += num0 * 5;
      }
      else
      {
    *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
    *scf++ = 0; *scf++ = 0;
      }

      if(!(scfsi & 0x2))
      {
    for(i=5;i;i--)
        *scf++ = getbits_fast(num1);
    numbits += num1 * 5;
      }
      else
      {
    *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
    *scf++ = 0; *scf++ = 0;
      }

      if(!(scfsi & 0x1))
      {
    for (i=5;i;i--)
        *scf++ = getbits_fast(num1);
    numbits += num1 * 5;
      }
      else
      {
    *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
    *scf++ = 0; *scf++ = 0;
      }
  }

  *scf++ = 0;  /* no l[21] in original sources */
    }
    return numbits;
}
#endif

static int III_get_scale_factors_2(int *scf,struct iemmp3_gr_info_s *gr_info,int i_stereo)
{
    unsigned char *pnt;
    int i,j;
    unsigned int slen;
    int n = 0;
    int numbits = 0;

    static unsigned char stab[3][6][4] = {
  { { 6, 5, 5,5 } , { 6, 5, 7,3 } , { 11,10,0,0} ,
  { 7, 7, 7,0 } , { 6, 6, 6,3 } , {  8, 8,5,0} } ,
  { { 9, 9, 9,9 } , { 9, 9,12,6 } , { 18,18,0,0} ,
  {12,12,12,0 } , {12, 9, 9,6 } , { 15,12,9,0} } ,
  { { 6, 9, 9,9 } , { 6, 9,12,6 } , { 15,18,0,0} ,
  { 6,15,12,0 } , { 6,12, 9,6 } , {  6,18,9,0} } };

    if(i_stereo) /* i_stereo AND second channel -> do_layer3() checks this */
  slen = iemmp3_slen2[gr_info->scalefac_compress>>1];
    else
  slen = iemmp3_n_slen2[gr_info->scalefac_compress];

    gr_info->preflag = (slen>>15) & 0x1;

    n = 0;
    if( gr_info->block_type == 2 )
    {
  n++;
  if(gr_info->mixed_block_flag)
      n++;
    }

    pnt = stab[n][(slen>>12)&0x7];

    for(i=0;i<4;i++)
    {
  int num = slen & 0x7;
  slen >>= 3;
  if(num)
  {
      for(j=0;j<(int)(pnt[i]);j++)
    *scf++ = getbits_fast(num);
      numbits += pnt[i] * num;
  }
  else
  {
      for(j=0;j<(int)(pnt[i]);j++)
    *scf++ = 0;
  }
    }

    n = (n << 1) + 1;
    for(i=0;i<n;i++)
  *scf++ = 0;

    return numbits;
}



static int III_dequantize_sample(real xr[SBLIMIT][SSLIMIT],int *scf,
         struct iemmp3_gr_info_s *gr_info,int sfreq,int part2bits)
{
    int shift = 1 + gr_info->scalefac_scale;
    real *xrpnt = (real *) xr;
    int l[3],l3;
    int part2remain = gr_info->part2_3_length - part2bits;
    int *me;

    {
  int bv       = gr_info->big_values;
  int region1  = gr_info->region1start;
  int region2  = gr_info->region2start;

  l3 = ((576>>1)-bv)>>1;
  /*
   * we may lose the 'odd' bit here !!
   * check this later again
   */
  if(bv <= region1)
  {
      l[0] = bv; l[1] = 0; l[2] = 0;
  }
  else
  {
      l[0] = region1;
      if(bv <= region2)
      {
    l[1] = bv - l[0];  l[2] = 0;
      }
      else
      {
    l[1] = region2 - l[0]; l[2] = bv - region2;
      }
  }
    }

    if(gr_info->block_type == 2)
    {
  /*
   * decoding with short or mixed mode BandIndex table
   */
  int i,max[4];
  int step=0,lwin=0,cb=0;
  register real v = 0.0;
  register int *m,mc;

  if(gr_info->mixed_block_flag)
  {
      max[3] = -1;
      max[0] = max[1] = max[2] = 2;
      m = iemmp3_map[sfreq][0];
      me = iemmp3_mapend[sfreq][0];
  }
  else
  {
      max[0] = max[1] = max[2] = max[3] = -1;
      /* max[3] not really needed in this case */
      m = iemmp3_map[sfreq][1];
      me = iemmp3_mapend[sfreq][1];
  }

  mc = 0;
  for(i=0;i<2;i++)
  {
      int lp = l[i];
      struct iemmp3_newHuff *h = iemmp3_ht+gr_info->table_select[i];
      for(;lp;lp--,mc--)
      {
    register int x,y;
    if( (!mc) )
    {
        mc = *m++;
        xrpnt = ((real *) xr) + (*m++);
        lwin = *m++;
        cb = *m++;
        if(lwin == 3)
        {
      v = gr_info->pow2gain[(*scf++) << shift];
      step = 1;
        }
        else
        {
      v = gr_info->full_gain[lwin][(*scf++) << shift];
      step = 3;
        }
    }
    {
        register short *val = h->table;
        while((y=*val++)<0)
        {
      if (get1bit())
          val -= y;
      part2remain--;
        }
        x = y >> 4;
        y &= 0xf;
    }
    if(x == 15)
    {
        max[lwin] = cb;
        part2remain -= h->linbits+1;
        x += getbits(h->linbits);
        if(get1bit())
      *xrpnt = -iemmp3_ispow[x] * v;
        else
      *xrpnt =  iemmp3_ispow[x] * v;
    }
    else if(x)
    {
        max[lwin] = cb;
        if(get1bit())
      *xrpnt = -iemmp3_ispow[x] * v;
        else
      *xrpnt =  iemmp3_ispow[x] * v;
        part2remain--;
    }
    else
        *xrpnt = 0.0;
    xrpnt += step;
    if(y == 15)
    {
        max[lwin] = cb;
        part2remain -= h->linbits+1;
        y += getbits(h->linbits);
        if(get1bit())
      *xrpnt = -iemmp3_ispow[y] * v;
        else
      *xrpnt =  iemmp3_ispow[y] * v;
    }
    else if(y)
    {
        max[lwin] = cb;
        if(get1bit())
      *xrpnt = -iemmp3_ispow[y] * v;
        else
      *xrpnt =  iemmp3_ispow[y] * v;
        part2remain--;
    }
    else
        *xrpnt = 0.0;
    xrpnt += step;
      }
  }
  for(;l3 && (part2remain > 0);l3--)
  {
      struct iemmp3_newHuff *h = iemmp3_htc+gr_info->count1table_select;
      register short *val = h->table,a;

      while((a=*val++)<0)
      {
    part2remain--;
    if(part2remain < 0)
    {
        part2remain++;
        a = 0;
        break;
    }
    if (get1bit())
        val -= a;
      }

      for(i=0;i<4;i++)
      {
    if(!(i & 1))
    {
        if(!mc)
        {
      mc = *m++;
      xrpnt = ((real *) xr) + (*m++);
      lwin = *m++;
      cb = *m++;
      if(lwin == 3)
      {
          v = gr_info->pow2gain[(*scf++) << shift];
          step = 1;
      }
      else
      {
          v = gr_info->full_gain[lwin][(*scf++) << shift];
          step = 3;
      }
        }
        mc--;
    }
    if( (a & (0x8>>i)) )
    {
        max[lwin] = cb;
        part2remain--;
        if(part2remain < 0)
        {
      part2remain++;
      break;
        }
        if(get1bit())
      *xrpnt = -v;
        else
      *xrpnt = v;
    }
    else
        *xrpnt = 0.0;
    xrpnt += step;
      }
  }

  while( m < me )
  {
      if(!mc)
      {
    mc = *m++;
    xrpnt = ((real *) xr) + *m++;
    if( (*m++) == 3)
        step = 1;
    else
        step = 3;
    m++; /* cb */
      }
      mc--;
      *xrpnt = 0.0;
      xrpnt += step;
      *xrpnt = 0.0;
      xrpnt += step;
      /* we could add a little opt. here:
       * if we finished a band for window 3 or a long band
       * further bands could copied in a simple loop without a
       * special 'map' decoding
       */
  }

  gr_info->maxband[0] = max[0]+1;
  gr_info->maxband[1] = max[1]+1;
  gr_info->maxband[2] = max[2]+1;
  gr_info->maxbandl = max[3]+1;

  {
      int rmax = max[0] > max[1] ? max[0] : max[1];
      rmax = (rmax > max[2] ? rmax : max[2]) + 1;
      gr_info->maxb = rmax ? iemmp3_shortLimit[sfreq][rmax] : iemmp3_longLimit[sfreq][max[3]+1];
  }

    }
    else
    {
  /*
   * decoding with 'long' BandIndex table (block_type != 2)
   */
  int *pretab = gr_info->preflag ? iemmp3_pretab1 : iemmp3_pretab2;
  int i,max = -1;
  int cb = 0;
  register int *m = iemmp3_map[sfreq][2];
  register real v = 0.0;
  register int mc = 0;

  /*
   * long hash table values
   */
  for(i=0;i<3;i++)
  {
      int lp = l[i];
      struct iemmp3_newHuff *h = iemmp3_ht+gr_info->table_select[i];

      for(;lp;lp--,mc--)
      {
    int x,y;

    if(!mc)
    {
        mc = *m++;
        v = gr_info->pow2gain[((*scf++) + (*pretab++)) << shift];
        cb = *m++;
    }
    {
        register short *val = h->table;
        while((y=*val++)<0)
        {
      if (get1bit())
          val -= y;
      part2remain--;
        }
        x = y >> 4;
        y &= 0xf;
    }
    if (x == 15)
    {
        max = cb;
        part2remain -= h->linbits+1;
        x += getbits(h->linbits);
        if(get1bit())
      *xrpnt++ = -iemmp3_ispow[x] * v;
        else
      *xrpnt++ =  iemmp3_ispow[x] * v;
    }
    else if(x)
    {
        max = cb;
        if(get1bit())
      *xrpnt++ = -iemmp3_ispow[x] * v;
        else
      *xrpnt++ =  iemmp3_ispow[x] * v;
        part2remain--;
    }
    else
        *xrpnt++ = 0.0;

    if (y == 15)
    {
        max = cb;
        part2remain -= h->linbits+1;
        y += getbits(h->linbits);
        if(get1bit())
      *xrpnt++ = -iemmp3_ispow[y] * v;
        else
      *xrpnt++ =  iemmp3_ispow[y] * v;
    }
    else if(y)
    {
        max = cb;
        if(get1bit())
      *xrpnt++ = -iemmp3_ispow[y] * v;
        else
      *xrpnt++ =  iemmp3_ispow[y] * v;
        part2remain--;
    }
    else
        *xrpnt++ = 0.0;
      }
  }

  /*
   * short (count1table) values
   */
  for(;l3 && (part2remain > 0);l3--)
  {
      struct iemmp3_newHuff *h = iemmp3_htc+gr_info->count1table_select;
      register short *val = h->table,a;

      while((a=*val++)<0)
      {
    part2remain--;
    if(part2remain < 0)
    {
        part2remain++;
        a = 0;
        break;
    }
    if (get1bit())
        val -= a;
      }

      for(i=0;i<4;i++)
      {
    if(!(i & 1))
    {
        if(!mc)
        {
      mc = *m++;
      cb = *m++;
      v = gr_info->pow2gain[((*scf++) + (*pretab++)) << shift];
        }
        mc--;
    }
    if ( (a & (0x8>>i)) )
    {
        max = cb;
        part2remain--;
        if(part2remain < 0)
        {
      part2remain++;
      break;
        }
        if(get1bit())
      *xrpnt++ = -v;
        else
      *xrpnt++ = v;
    }
    else
        *xrpnt++ = 0.0;
      }
  }

  /*
   * zero part
   */
  for(i=(&xr[SBLIMIT][0]-xrpnt)>>1;i;i--)
  {
      *xrpnt++ = 0.0;
      *xrpnt++ = 0.0;
  }

  gr_info->maxbandl = max+1;
  gr_info->maxb = iemmp3_longLimit[sfreq][gr_info->maxbandl];
    }

    while( part2remain > 16 )
    {
  getbits(16); /* Dismiss stuffing Bits */
  part2remain -= 16;
    }
    if(part2remain > 0)
  getbits(part2remain);
    else if(part2remain < 0)
    {
  post("mpg123: Can't rewind stream by %d bits!\n",-part2remain);
  return 1; /* -> error */
    }
    return 0;
}

static void III_i_stereo(real xr_buf[2][SBLIMIT][SSLIMIT],int *scalefac,
       struct iemmp3_gr_info_s *gr_info,int sfreq,int ms_stereo,int lsf)
{
    real (*xr)[SBLIMIT*SSLIMIT] = (real (*)[SBLIMIT*SSLIMIT] ) xr_buf;
    struct iemmp3_bandInfoStruct *bi = &bandInfo[sfreq];
    real *tab1,*tab2;

    if(lsf)
    {
  int p = gr_info->scalefac_compress & 0x1;
  if(ms_stereo)
  {
      tab1 = iemmp3_pow1_2[p]; tab2 = iemmp3_pow2_2[p];
  }
  else
  {
      tab1 = iemmp3_pow1_1[p]; tab2 = iemmp3_pow2_1[p];
  }
    }
    else
    {
  if(ms_stereo)
  {
      tab1 = iemmp3_tan1_2; tab2 = iemmp3_tan2_2;
  }
  else
  {
      tab1 = iemmp3_tan1_1; tab2 = iemmp3_tan2_1;
  }
    }

    if (gr_info->block_type == 2)
    {
  int lwin,do_l = 0;
  if( gr_info->mixed_block_flag )
      do_l = 1;

  for (lwin=0;lwin<3;lwin++) /* process each window */
  {
      /* get first band with zero values */
      int is_p,sb,idx,sfb = gr_info->maxband[lwin];  /* sfb is minimal 3 for mixed mode */
      if(sfb > 3)
    do_l = 0;

      for(;sfb<12;sfb++)
      {
    is_p = scalefac[sfb*3+lwin-gr_info->mixed_block_flag]; /* scale: 0-15 */
    if(is_p != 7) {
        real t1,t2;
        sb = bi->shortDiff[sfb];
        idx = bi->shortIdx[sfb] + lwin;
        t1 = tab1[is_p]; t2 = tab2[is_p];
        for (; sb > 0; sb--,idx+=3)
        {
      real v = xr[0][idx];
      xr[0][idx] = v * t1;
      xr[1][idx] = v * t2;
        }
    }
      }

#if 1
      /* in the original: copy 10 to 11 , here: copy 11 to 12
       maybe still wrong??? (copy 12 to 13?) */
      is_p = scalefac[11*3+lwin-gr_info->mixed_block_flag]; /* scale: 0-15 */
      sb = bi->shortDiff[12];
      idx = bi->shortIdx[12] + lwin;
#else
      is_p = scalefac[10*3+lwin-gr_info->mixed_block_flag]; /* scale: 0-15 */
      sb = bi->shortDiff[11];
      idx = bi->shortIdx[11] + lwin;
#endif
      if(is_p != 7)
      {
    real t1,t2;
    t1 = tab1[is_p]; t2 = tab2[is_p];
    for ( ; sb > 0; sb--,idx+=3 )
    {
        real v = xr[0][idx];
        xr[0][idx] = v * t1;
        xr[1][idx] = v * t2;
    }
      }
  } /* end for(lwin; .. ; . ) */

  if (do_l)
  {
      /* also check l-part, if ALL bands in the three windows are 'empty'
       * and mode = mixed_mode
       */
      int sfb = gr_info->maxbandl;
      int idx = bi->longIdx[sfb];

      for ( ; sfb<8; sfb++ )
      {
    int sb = bi->longDiff[sfb];
    int is_p = scalefac[sfb]; /* scale: 0-15 */
    if(is_p != 7) {
        real t1,t2;
        t1 = tab1[is_p]; t2 = tab2[is_p];
        for ( ; sb > 0; sb--,idx++)
        {
      real v = xr[0][idx];
      xr[0][idx] = v * t1;
      xr[1][idx] = v * t2;
        }
    }
    else
        idx += sb;
      }
  }
    }
    else /* ((gr_info->block_type != 2)) */
    {
  int sfb = gr_info->maxbandl;
  int is_p,idx = bi->longIdx[sfb];
  for ( ; sfb<21; sfb++)
  {
      int sb = bi->longDiff[sfb];
      is_p = scalefac[sfb]; /* scale: 0-15 */
      if(is_p != 7) {
    real t1,t2;
    t1 = tab1[is_p]; t2 = tab2[is_p];
    for ( ; sb > 0; sb--,idx++)
    {
        real v = xr[0][idx];
        xr[0][idx] = v * t1;
        xr[1][idx] = v * t2;
    }
      }
      else
    idx += sb;
  }

  is_p = scalefac[20]; /* copy l-band 20 to l-band 21 */
  if(is_p != 7)
  {
      int sb;
      real t1 = tab1[is_p],t2 = tab2[is_p];

      for ( sb = bi->longDiff[21]; sb > 0; sb--,idx++ )
      {
    real v = xr[0][idx];
    xr[0][idx] = v * t1;
    xr[1][idx] = v * t2;
      }
  }
    } /* ... */
}


static void III_antialias(real xr[SBLIMIT][SSLIMIT],struct iemmp3_gr_info_s *gr_info)
{
    int sblim;

    if(gr_info->block_type == 2)
    {
  if(!gr_info->mixed_block_flag)
      return;
  sblim = 1;
    }
    else {
  sblim = gr_info->maxb-1;
    }

    /* 31 alias-reduction operations between each pair of sub-bands */
    /* with 8 butterflies between each pair                         */

    {
  int sb;
  real *xr1=(real *) xr[1];

  for(sb=sblim;sb;sb--,xr1+=10)
  {
      int ss;
      real *cs=iemmp3_aa_cs,*ca=iemmp3_aa_ca;
      real *xr2 = xr1;

      for(ss=7;ss>=0;ss--)
      {       /* upper and lower butterfly inputs */
    register real bu = *--xr2,bd = *xr1;
    *xr2   = (bu * (*cs)   ) - (bd * (*ca)   );
    *xr1++ = (bd * (*cs++) ) + (bu * (*ca++) );
      }
  }
    }
}

static void III_hybrid(real fsIn[SBLIMIT][SSLIMIT],real tsOut[SSLIMIT][SBLIMIT],
           int ch,struct iemmp3_gr_info_s *gr_info)
{
    real *tspnt = (real *) tsOut;
    real (*block)[2][SBLIMIT*SSLIMIT] = iemmp3_gmp->hybrid_block;
    int *blc = iemmp3_gmp->hybrid_blc;
    real *rawout1,*rawout2;
    int bt;
    unsigned int sb = 0;

    {
  int b = blc[ch];
  rawout1=block[b][ch];
  b=-b+1;
  rawout2=block[b][ch];
  blc[ch] = b;
    }


    if(gr_info->mixed_block_flag) {
  sb = 2;
  dct36(fsIn[0],rawout1,rawout2,iemmp3_win[0],tspnt);
  dct36(fsIn[1],rawout1+18,rawout2+18,iemmp3_win1[0],tspnt+1);
  rawout1 += 36; rawout2 += 36; tspnt += 2;
    }

    bt = gr_info->block_type;
    if(bt == 2) {
  for (; sb<gr_info->maxb; sb+=2,tspnt+=2,rawout1+=36,rawout2+=36) {
      dct12(fsIn[sb],rawout1,rawout2,iemmp3_win[2],tspnt);
      dct12(fsIn[sb+1],rawout1+18,rawout2+18,iemmp3_win1[2],tspnt+1);
  }
    }
    else {
  for (; sb<gr_info->maxb; sb+=2,tspnt+=2,rawout1+=36,rawout2+=36) {
      dct36(fsIn[sb],rawout1,rawout2,iemmp3_win[bt],tspnt);
      dct36(fsIn[sb+1],rawout1+18,rawout2+18,iemmp3_win1[bt],tspnt+1);
  }
    }

    for(;sb<SBLIMIT;sb++,tspnt++) {
  int i;
  for(i=0;i<SSLIMIT;i++) {
      tspnt[i*SBLIMIT] = *rawout1++;
      *rawout2++ = 0.0;
  }
    }
}

static void dct64_1(real *out0,real *out1,real *b1,real *b2,real *samples)
{

    {
  register real *costab = iemmp3_pnts[0];

  b1[0x00] = samples[0x00] + samples[0x1F];
  b1[0x1F] = (samples[0x00] - samples[0x1F]) * costab[0x0];

  b1[0x01] = samples[0x01] + samples[0x1E];
  b1[0x1E] = (samples[0x01] - samples[0x1E]) * costab[0x1];

  b1[0x02] = samples[0x02] + samples[0x1D];
  b1[0x1D] = (samples[0x02] - samples[0x1D]) * costab[0x2];

  b1[0x03] = samples[0x03] + samples[0x1C];
  b1[0x1C] = (samples[0x03] - samples[0x1C]) * costab[0x3];

  b1[0x04] = samples[0x04] + samples[0x1B];
  b1[0x1B] = (samples[0x04] - samples[0x1B]) * costab[0x4];

  b1[0x05] = samples[0x05] + samples[0x1A];
  b1[0x1A] = (samples[0x05] - samples[0x1A]) * costab[0x5];

  b1[0x06] = samples[0x06] + samples[0x19];
  b1[0x19] = (samples[0x06] - samples[0x19]) * costab[0x6];

  b1[0x07] = samples[0x07] + samples[0x18];
  b1[0x18] = (samples[0x07] - samples[0x18]) * costab[0x7];

  b1[0x08] = samples[0x08] + samples[0x17];
  b1[0x17] = (samples[0x08] - samples[0x17]) * costab[0x8];

  b1[0x09] = samples[0x09] + samples[0x16];
  b1[0x16] = (samples[0x09] - samples[0x16]) * costab[0x9];

  b1[0x0A] = samples[0x0A] + samples[0x15];
  b1[0x15] = (samples[0x0A] - samples[0x15]) * costab[0xA];

  b1[0x0B] = samples[0x0B] + samples[0x14];
  b1[0x14] = (samples[0x0B] - samples[0x14]) * costab[0xB];

  b1[0x0C] = samples[0x0C] + samples[0x13];
  b1[0x13] = (samples[0x0C] - samples[0x13]) * costab[0xC];

  b1[0x0D] = samples[0x0D] + samples[0x12];
  b1[0x12] = (samples[0x0D] - samples[0x12]) * costab[0xD];

  b1[0x0E] = samples[0x0E] + samples[0x11];
  b1[0x11] = (samples[0x0E] - samples[0x11]) * costab[0xE];

  b1[0x0F] = samples[0x0F] + samples[0x10];
  b1[0x10] = (samples[0x0F] - samples[0x10]) * costab[0xF];
    }


    {
  register real *costab = iemmp3_pnts[1];

  b2[0x00] = b1[0x00] + b1[0x0F];
  b2[0x0F] = (b1[0x00] - b1[0x0F]) * costab[0];
  b2[0x01] = b1[0x01] + b1[0x0E];
  b2[0x0E] = (b1[0x01] - b1[0x0E]) * costab[1];
  b2[0x02] = b1[0x02] + b1[0x0D];
  b2[0x0D] = (b1[0x02] - b1[0x0D]) * costab[2];
  b2[0x03] = b1[0x03] + b1[0x0C];
  b2[0x0C] = (b1[0x03] - b1[0x0C]) * costab[3];
  b2[0x04] = b1[0x04] + b1[0x0B];
  b2[0x0B] = (b1[0x04] - b1[0x0B]) * costab[4];
  b2[0x05] = b1[0x05] + b1[0x0A];
  b2[0x0A] = (b1[0x05] - b1[0x0A]) * costab[5];
  b2[0x06] = b1[0x06] + b1[0x09];
  b2[0x09] = (b1[0x06] - b1[0x09]) * costab[6];
  b2[0x07] = b1[0x07] + b1[0x08];
  b2[0x08] = (b1[0x07] - b1[0x08]) * costab[7];

  b2[0x10] = b1[0x10] + b1[0x1F];
  b2[0x1F] = (b1[0x1F] - b1[0x10]) * costab[0];
  b2[0x11] = b1[0x11] + b1[0x1E];
  b2[0x1E] = (b1[0x1E] - b1[0x11]) * costab[1];
  b2[0x12] = b1[0x12] + b1[0x1D];
  b2[0x1D] = (b1[0x1D] - b1[0x12]) * costab[2];
  b2[0x13] = b1[0x13] + b1[0x1C];
  b2[0x1C] = (b1[0x1C] - b1[0x13]) * costab[3];
  b2[0x14] = b1[0x14] + b1[0x1B];
  b2[0x1B] = (b1[0x1B] - b1[0x14]) * costab[4];
  b2[0x15] = b1[0x15] + b1[0x1A];
  b2[0x1A] = (b1[0x1A] - b1[0x15]) * costab[5];
  b2[0x16] = b1[0x16] + b1[0x19];
  b2[0x19] = (b1[0x19] - b1[0x16]) * costab[6];
  b2[0x17] = b1[0x17] + b1[0x18];
  b2[0x18] = (b1[0x18] - b1[0x17]) * costab[7];
    }

    {
  register real *costab = iemmp3_pnts[2];

  b1[0x00] = b2[0x00] + b2[0x07];
  b1[0x07] = (b2[0x00] - b2[0x07]) * costab[0];
  b1[0x01] = b2[0x01] + b2[0x06];
  b1[0x06] = (b2[0x01] - b2[0x06]) * costab[1];
  b1[0x02] = b2[0x02] + b2[0x05];
  b1[0x05] = (b2[0x02] - b2[0x05]) * costab[2];
  b1[0x03] = b2[0x03] + b2[0x04];
  b1[0x04] = (b2[0x03] - b2[0x04]) * costab[3];

  b1[0x08] = b2[0x08] + b2[0x0F];
  b1[0x0F] = (b2[0x0F] - b2[0x08]) * costab[0];
  b1[0x09] = b2[0x09] + b2[0x0E];
  b1[0x0E] = (b2[0x0E] - b2[0x09]) * costab[1];
  b1[0x0A] = b2[0x0A] + b2[0x0D];
  b1[0x0D] = (b2[0x0D] - b2[0x0A]) * costab[2];
  b1[0x0B] = b2[0x0B] + b2[0x0C];
  b1[0x0C] = (b2[0x0C] - b2[0x0B]) * costab[3];

  b1[0x10] = b2[0x10] + b2[0x17];
  b1[0x17] = (b2[0x10] - b2[0x17]) * costab[0];
  b1[0x11] = b2[0x11] + b2[0x16];
  b1[0x16] = (b2[0x11] - b2[0x16]) * costab[1];
  b1[0x12] = b2[0x12] + b2[0x15];
  b1[0x15] = (b2[0x12] - b2[0x15]) * costab[2];
  b1[0x13] = b2[0x13] + b2[0x14];
  b1[0x14] = (b2[0x13] - b2[0x14]) * costab[3];

  b1[0x18] = b2[0x18] + b2[0x1F];
  b1[0x1F] = (b2[0x1F] - b2[0x18]) * costab[0];
  b1[0x19] = b2[0x19] + b2[0x1E];
  b1[0x1E] = (b2[0x1E] - b2[0x19]) * costab[1];
  b1[0x1A] = b2[0x1A] + b2[0x1D];
  b1[0x1D] = (b2[0x1D] - b2[0x1A]) * costab[2];
  b1[0x1B] = b2[0x1B] + b2[0x1C];
  b1[0x1C] = (b2[0x1C] - b2[0x1B]) * costab[3];
    }

    {
  register real const cos0 = iemmp3_pnts[3][0];
  register real const cos1 = iemmp3_pnts[3][1];

  b2[0x00] = b1[0x00] + b1[0x03];
  b2[0x03] = (b1[0x00] - b1[0x03]) * cos0;
  b2[0x01] = b1[0x01] + b1[0x02];
  b2[0x02] = (b1[0x01] - b1[0x02]) * cos1;

  b2[0x04] = b1[0x04] + b1[0x07];
  b2[0x07] = (b1[0x07] - b1[0x04]) * cos0;
  b2[0x05] = b1[0x05] + b1[0x06];
  b2[0x06] = (b1[0x06] - b1[0x05]) * cos1;

  b2[0x08] = b1[0x08] + b1[0x0B];
  b2[0x0B] = (b1[0x08] - b1[0x0B]) * cos0;
  b2[0x09] = b1[0x09] + b1[0x0A];
  b2[0x0A] = (b1[0x09] - b1[0x0A]) * cos1;

  b2[0x0C] = b1[0x0C] + b1[0x0F];
  b2[0x0F] = (b1[0x0F] - b1[0x0C]) * cos0;
  b2[0x0D] = b1[0x0D] + b1[0x0E];
  b2[0x0E] = (b1[0x0E] - b1[0x0D]) * cos1;

  b2[0x10] = b1[0x10] + b1[0x13];
  b2[0x13] = (b1[0x10] - b1[0x13]) * cos0;
  b2[0x11] = b1[0x11] + b1[0x12];
  b2[0x12] = (b1[0x11] - b1[0x12]) * cos1;

  b2[0x14] = b1[0x14] + b1[0x17];
  b2[0x17] = (b1[0x17] - b1[0x14]) * cos0;
  b2[0x15] = b1[0x15] + b1[0x16];
  b2[0x16] = (b1[0x16] - b1[0x15]) * cos1;

  b2[0x18] = b1[0x18] + b1[0x1B];
  b2[0x1B] = (b1[0x18] - b1[0x1B]) * cos0;
  b2[0x19] = b1[0x19] + b1[0x1A];
  b2[0x1A] = (b1[0x19] - b1[0x1A]) * cos1;

  b2[0x1C] = b1[0x1C] + b1[0x1F];
  b2[0x1F] = (b1[0x1F] - b1[0x1C]) * cos0;
  b2[0x1D] = b1[0x1D] + b1[0x1E];
  b2[0x1E] = (b1[0x1E] - b1[0x1D]) * cos1;
    }

    {
  register real const cos0 = iemmp3_pnts[4][0];

  b1[0x00] = b2[0x00] + b2[0x01];
  b1[0x01] = (b2[0x00] - b2[0x01]) * cos0;
  b1[0x02] = b2[0x02] + b2[0x03];
  b1[0x03] = (b2[0x03] - b2[0x02]) * cos0;
  b1[0x02] += b1[0x03];

  b1[0x04] = b2[0x04] + b2[0x05];
  b1[0x05] = (b2[0x04] - b2[0x05]) * cos0;
  b1[0x06] = b2[0x06] + b2[0x07];
  b1[0x07] = (b2[0x07] - b2[0x06]) * cos0;
  b1[0x06] += b1[0x07];
  b1[0x04] += b1[0x06];
  b1[0x06] += b1[0x05];
  b1[0x05] += b1[0x07];

  b1[0x08] = b2[0x08] + b2[0x09];
  b1[0x09] = (b2[0x08] - b2[0x09]) * cos0;
  b1[0x0A] = b2[0x0A] + b2[0x0B];
  b1[0x0B] = (b2[0x0B] - b2[0x0A]) * cos0;
  b1[0x0A] += b1[0x0B];

  b1[0x0C] = b2[0x0C] + b2[0x0D];
  b1[0x0D] = (b2[0x0C] - b2[0x0D]) * cos0;
  b1[0x0E] = b2[0x0E] + b2[0x0F];
  b1[0x0F] = (b2[0x0F] - b2[0x0E]) * cos0;
  b1[0x0E] += b1[0x0F];
  b1[0x0C] += b1[0x0E];
  b1[0x0E] += b1[0x0D];
  b1[0x0D] += b1[0x0F];

  b1[0x10] = b2[0x10] + b2[0x11];
  b1[0x11] = (b2[0x10] - b2[0x11]) * cos0;
  b1[0x12] = b2[0x12] + b2[0x13];
  b1[0x13] = (b2[0x13] - b2[0x12]) * cos0;
  b1[0x12] += b1[0x13];

  b1[0x14] = b2[0x14] + b2[0x15];
  b1[0x15] = (b2[0x14] - b2[0x15]) * cos0;
  b1[0x16] = b2[0x16] + b2[0x17];
  b1[0x17] = (b2[0x17] - b2[0x16]) * cos0;
  b1[0x16] += b1[0x17];
  b1[0x14] += b1[0x16];
  b1[0x16] += b1[0x15];
  b1[0x15] += b1[0x17];

  b1[0x18] = b2[0x18] + b2[0x19];
  b1[0x19] = (b2[0x18] - b2[0x19]) * cos0;
  b1[0x1A] = b2[0x1A] + b2[0x1B];
  b1[0x1B] = (b2[0x1B] - b2[0x1A]) * cos0;
  b1[0x1A] += b1[0x1B];

  b1[0x1C] = b2[0x1C] + b2[0x1D];
  b1[0x1D] = (b2[0x1C] - b2[0x1D]) * cos0;
  b1[0x1E] = b2[0x1E] + b2[0x1F];
  b1[0x1F] = (b2[0x1F] - b2[0x1E]) * cos0;
  b1[0x1E] += b1[0x1F];
  b1[0x1C] += b1[0x1E];
  b1[0x1E] += b1[0x1D];
  b1[0x1D] += b1[0x1F];
    }

    out0[0x10*16] = b1[0x00];
    out0[0x10*12] = b1[0x04];
    out0[0x10* 8] = b1[0x02];
    out0[0x10* 4] = b1[0x06];
    out0[0x10* 0] = b1[0x01];
    out1[0x10* 0] = b1[0x01];
    out1[0x10* 4] = b1[0x05];
    out1[0x10* 8] = b1[0x03];
    out1[0x10*12] = b1[0x07];

    b1[0x08] += b1[0x0C];
    out0[0x10*14] = b1[0x08];
    b1[0x0C] += b1[0x0a];
    out0[0x10*10] = b1[0x0C];
    b1[0x0A] += b1[0x0E];
    out0[0x10* 6] = b1[0x0A];
    b1[0x0E] += b1[0x09];
    out0[0x10* 2] = b1[0x0E];
    b1[0x09] += b1[0x0D];
    out1[0x10* 2] = b1[0x09];
    b1[0x0D] += b1[0x0B];
    out1[0x10* 6] = b1[0x0D];
    b1[0x0B] += b1[0x0F];
    out1[0x10*10] = b1[0x0B];
    out1[0x10*14] = b1[0x0F];

    b1[0x18] += b1[0x1C];
    out0[0x10*15] = b1[0x10] + b1[0x18];
    out0[0x10*13] = b1[0x18] + b1[0x14];
    b1[0x1C] += b1[0x1a];
    out0[0x10*11] = b1[0x14] + b1[0x1C];
    out0[0x10* 9] = b1[0x1C] + b1[0x12];
    b1[0x1A] += b1[0x1E];
    out0[0x10* 7] = b1[0x12] + b1[0x1A];
    out0[0x10* 5] = b1[0x1A] + b1[0x16];
    b1[0x1E] += b1[0x19];
    out0[0x10* 3] = b1[0x16] + b1[0x1E];
    out0[0x10* 1] = b1[0x1E] + b1[0x11];
    b1[0x19] += b1[0x1D];
    out1[0x10* 1] = b1[0x11] + b1[0x19];
    out1[0x10* 3] = b1[0x19] + b1[0x15];
    b1[0x1D] += b1[0x1B];
    out1[0x10* 5] = b1[0x15] + b1[0x1D];
    out1[0x10* 7] = b1[0x1D] + b1[0x13];
    b1[0x1B] += b1[0x1F];
    out1[0x10* 9] = b1[0x13] + b1[0x1B];
    out1[0x10*11] = b1[0x1B] + b1[0x17];
    out1[0x10*13] = b1[0x17] + b1[0x1F];
    out1[0x10*15] = b1[0x1F];
}

/*
 * the call via dct64 is a trick to force GCC to use
 * (new) registers for the b1,b2 pointer to the bufs[xx] field
 */
void dct64(real *a,real *b,real *c)
{
    real bufs[0x40];
    dct64_1(a,b,bufs,bufs+0x20,c);
}

static void dct36(real *inbuf,real *o1,real *o2,real *wintab,real *tsbuf)
{
    {
  register real *in = inbuf;

  in[17]+=in[16]; in[16]+=in[15]; in[15]+=in[14];
  in[14]+=in[13]; in[13]+=in[12]; in[12]+=in[11];
  in[11]+=in[10]; in[10]+=in[9];  in[9] +=in[8];
  in[8] +=in[7];  in[7] +=in[6];  in[6] +=in[5];
  in[5] +=in[4];  in[4] +=in[3];  in[3] +=in[2];
  in[2] +=in[1];  in[1] +=in[0];

  in[17]+=in[15]; in[15]+=in[13]; in[13]+=in[11]; in[11]+=in[9];
  in[9] +=in[7];  in[7] +=in[5];  in[5] +=in[3];  in[3] +=in[1];


  {

#define MACRO0(v) { \
    real tmp; \
    out2[9+(v)] = (tmp = sum0 + sum1) * w[27+(v)]; \
    out2[8-(v)] = tmp * w[26-(v)];  } \
    sum0 -= sum1; \
    ts[SBLIMIT*(8-(v))] = out1[8-(v)] + sum0 * w[8-(v)]; \
    ts[SBLIMIT*(9+(v))] = out1[9+(v)] + sum0 * w[9+(v)];
#define MACRO1(v) { \
    real sum0,sum1; \
    sum0 = tmp1a + tmp2a; \
    sum1 = (tmp1b + tmp2b) * iemmp3_tfcos36[(v)]; \
    MACRO0(v); }
#define MACRO2(v) { \
    real sum0,sum1; \
    sum0 = tmp2a - tmp1a; \
    sum1 = (tmp2b - tmp1b) * iemmp3_tfcos36[(v)]; \
    MACRO0(v); }

      register const real *c = iemmp3_COS9;
      register real *out2 = o2;
      register real *w = wintab;
      register real *out1 = o1;
      register real *ts = tsbuf;

      real ta33,ta66,tb33,tb66;

      ta33 = in[2*3+0] * c[3];
      ta66 = in[2*6+0] * c[6];
      tb33 = in[2*3+1] * c[3];
      tb66 = in[2*6+1] * c[6];

      {
    real tmp1a,tmp2a,tmp1b,tmp2b;
    tmp1a =             in[2*1+0] * c[1] + ta33 + in[2*5+0] * c[5] + in[2*7+0] * c[7];
    tmp1b =             in[2*1+1] * c[1] + tb33 + in[2*5+1] * c[5] + in[2*7+1] * c[7];
    tmp2a = in[2*0+0] + in[2*2+0] * c[2] + in[2*4+0] * c[4] + ta66 + in[2*8+0] * c[8];
    tmp2b = in[2*0+1] + in[2*2+1] * c[2] + in[2*4+1] * c[4] + tb66 + in[2*8+1] * c[8];

    MACRO1(0);
    MACRO2(8);
      }

      {
    real tmp1a,tmp2a,tmp1b,tmp2b;
    tmp1a = ( in[2*1+0] - in[2*5+0] - in[2*7+0] ) * c[3];
    tmp1b = ( in[2*1+1] - in[2*5+1] - in[2*7+1] ) * c[3];
    tmp2a = ( in[2*2+0] - in[2*4+0] - in[2*8+0] ) * c[6] - in[2*6+0] + in[2*0+0];
    tmp2b = ( in[2*2+1] - in[2*4+1] - in[2*8+1] ) * c[6] - in[2*6+1] + in[2*0+1];

    MACRO1(1);
    MACRO2(7);
      }

      {
    real tmp1a,tmp2a,tmp1b,tmp2b;
    tmp1a =             in[2*1+0] * c[5] - ta33 - in[2*5+0] * c[7] + in[2*7+0] * c[1];
    tmp1b =             in[2*1+1] * c[5] - tb33 - in[2*5+1] * c[7] + in[2*7+1] * c[1];
    tmp2a = in[2*0+0] - in[2*2+0] * c[8] - in[2*4+0] * c[2] + ta66 + in[2*8+0] * c[4];
    tmp2b = in[2*0+1] - in[2*2+1] * c[8] - in[2*4+1] * c[2] + tb66 + in[2*8+1] * c[4];

    MACRO1(2);
    MACRO2(6);
      }

      {
    real tmp1a,tmp2a,tmp1b,tmp2b;
    tmp1a =             in[2*1+0] * c[7] - ta33 + in[2*5+0] * c[1] - in[2*7+0] * c[5];
    tmp1b =             in[2*1+1] * c[7] - tb33 + in[2*5+1] * c[1] - in[2*7+1] * c[5];
    tmp2a = in[2*0+0] - in[2*2+0] * c[4] + in[2*4+0] * c[8] + ta66 - in[2*8+0] * c[2];
    tmp2b = in[2*0+1] - in[2*2+1] * c[4] + in[2*4+1] * c[8] + tb66 - in[2*8+1] * c[2];

    MACRO1(3);
    MACRO2(5);
      }

      {
    real sum0,sum1;
    sum0 =  in[2*0+0] - in[2*2+0] + in[2*4+0] - in[2*6+0] + in[2*8+0];
    sum1 = (in[2*0+1] - in[2*2+1] + in[2*4+1] - in[2*6+1] + in[2*8+1] ) * iemmp3_tfcos36[4];
    MACRO0(4);
      }
  }

    }
}

/*
 * new DCT12
 */
static void dct12(real *in,real *rawout1,real *rawout2,register real *wi,register real *ts)
{
#define DCT12_PART1 \
    in5 = in[5*3];  \
    in5 += (in4 = in[4*3]); \
    in4 += (in3 = in[3*3]); \
    in3 += (in2 = in[2*3]); \
    in2 += (in1 = in[1*3]); \
    in1 += (in0 = in[0*3]); \
    \
    in5 += in3; in3 += in1; \
    \
    in2 *= iemmp3_COS6_1; \
    in3 *= iemmp3_COS6_1; \

#define DCT12_PART2 \
    in0 += in4 * iemmp3_COS6_2; \
    \
    in4 = in0 + in2;     \
    in0 -= in2;          \
    \
    in1 += in5 * iemmp3_COS6_2; \
    \
    in5 = (in1 + in3) * iemmp3_tfcos12[0]; \
    in1 = (in1 - in3) * iemmp3_tfcos12[2]; \
    \
    in3 = in4 + in5;    \
    in4 -= in5;         \
    \
    in2 = in0 + in1;    \
    in0 -= in1;


    {
  real in0,in1,in2,in3,in4,in5;
  register real *out1 = rawout1;
  ts[SBLIMIT*0] = out1[0]; ts[SBLIMIT*1] = out1[1]; ts[SBLIMIT*2] = out1[2];
  ts[SBLIMIT*3] = out1[3]; ts[SBLIMIT*4] = out1[4]; ts[SBLIMIT*5] = out1[5];

  DCT12_PART1

  {
      real tmp0,tmp1 = (in0 - in4);
      {
    real tmp2 = (in1 - in5) * iemmp3_tfcos12[1];
    tmp0 = tmp1 + tmp2;
    tmp1 -= tmp2;
      }
      ts[(17-1)*SBLIMIT] = out1[17-1] + tmp0 * wi[11-1];
      ts[(12+1)*SBLIMIT] = out1[12+1] + tmp0 * wi[6+1];
      ts[(6 +1)*SBLIMIT] = out1[6 +1] + tmp1 * wi[1];
      ts[(11-1)*SBLIMIT] = out1[11-1] + tmp1 * wi[5-1];
  }

  DCT12_PART2

      ts[(17-0)*SBLIMIT] = out1[17-0] + in2 * wi[11-0];
  ts[(12+0)*SBLIMIT] = out1[12+0] + in2 * wi[6+0];
  ts[(12+2)*SBLIMIT] = out1[12+2] + in3 * wi[6+2];
  ts[(17-2)*SBLIMIT] = out1[17-2] + in3 * wi[11-2];

  ts[(6+0)*SBLIMIT]  = out1[6+0] + in0 * wi[0];
  ts[(11-0)*SBLIMIT] = out1[11-0] + in0 * wi[5-0];
  ts[(6+2)*SBLIMIT]  = out1[6+2] + in4 * wi[2];
  ts[(11-2)*SBLIMIT] = out1[11-2] + in4 * wi[5-2];
    }

    in++;

    {
  real in0,in1,in2,in3,in4,in5;
  register real *out2 = rawout2;

  DCT12_PART1

  {
      real tmp0,tmp1 = (in0 - in4);
      {
    real tmp2 = (in1 - in5) * iemmp3_tfcos12[1];
    tmp0 = tmp1 + tmp2;
    tmp1 -= tmp2;
      }
      out2[5-1] = tmp0 * wi[11-1];
      out2[0+1] = tmp0 * wi[6+1];
      ts[(12+1)*SBLIMIT] += tmp1 * wi[1];
      ts[(17-1)*SBLIMIT] += tmp1 * wi[5-1];
  }

  DCT12_PART2

      out2[5-0] = in2 * wi[11-0];
  out2[0+0] = in2 * wi[6+0];
  out2[0+2] = in3 * wi[6+2];
  out2[5-2] = in3 * wi[11-2];

  ts[(12+0)*SBLIMIT] += in0 * wi[0];
  ts[(17-0)*SBLIMIT] += in0 * wi[5-0];
  ts[(12+2)*SBLIMIT] += in4 * wi[2];
  ts[(17-2)*SBLIMIT] += in4 * wi[5-2];
    }

    in++;

    {
  real in0,in1,in2,in3,in4,in5;
  register real *out2 = rawout2;
  out2[12]=out2[13]=out2[14]=out2[15]=out2[16]=out2[17]=0.0;

  DCT12_PART1

  {
      real tmp0,tmp1 = (in0 - in4);
      {
    real tmp2 = (in1 - in5) * iemmp3_tfcos12[1];
    tmp0 = tmp1 + tmp2;
    tmp1 -= tmp2;
      }
      out2[11-1] = tmp0 * wi[11-1];
      out2[6 +1] = tmp0 * wi[6+1];
      out2[0+1] += tmp1 * wi[1];
      out2[5-1] += tmp1 * wi[5-1];
  }

  DCT12_PART2

      out2[11-0] = in2 * wi[11-0];
  out2[6 +0] = in2 * wi[6+0];
  out2[6 +2] = in3 * wi[6+2];
  out2[11-2] = in3 * wi[11-2];

  out2[0+0] += in0 * wi[0];
  out2[5-0] += in0 * wi[5-0];
  out2[0+2] += in4 * wi[2];
  out2[5-2] += in4 * wi[5-2];
    }
}

static int synth_1to1_mono(real *bandPtr,unsigned char *samples,int *pnt)
{
    short samples_tmp[64];
    short *tmp1 = samples_tmp;
    int i,ret;
    int pnt1 = 0;

    ret = synth_1to1(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
    samples += *pnt;

    for(i=0;i<32;i++)
    {
  *( (short *) samples) = *tmp1;
  samples += 2;
  tmp1 += 2;
    }
    *pnt += 64;

    return ret;
}


static int synth_1to1(real *bandPtr,int channel,unsigned char *out,int *pnt)
{
    static const int step = 2;
    int bo;
    short *samples = (short *) (out + *pnt);

    real *b0,(*buf)[0x110];
    int clip = 0;
    int bo1;

    bo = iemmp3_gmp->synth_bo;

    if(!channel)
    {
  bo--;
  bo &= 0xf;
  buf = iemmp3_gmp->synth_buffs[0];
    }
    else
    {
  samples++;
  buf = iemmp3_gmp->synth_buffs[1];
    }

    if(bo & 0x1)
    {
  b0 = buf[0];
  bo1 = bo;
  dct64(buf[1]+((bo+1)&0xf),buf[0]+bo,bandPtr);
    }
    else
    {
  b0 = buf[1];
  bo1 = bo+1;
  dct64(buf[0]+bo,buf[1]+bo+1,bandPtr);
    }

    iemmp3_gmp->synth_bo = bo;

    {
  register int j;
  real *window = iemmp3_decwin + 16 - bo1;

  for (j=16;j;j--,b0+=0x10,window+=0x20,samples+=step)
  {
      real sum;
      sum  = window[0x0] * b0[0x0];
      sum -= window[0x1] * b0[0x1];
      sum += window[0x2] * b0[0x2];
      sum -= window[0x3] * b0[0x3];
      sum += window[0x4] * b0[0x4];
      sum -= window[0x5] * b0[0x5];
      sum += window[0x6] * b0[0x6];
      sum -= window[0x7] * b0[0x7];
      sum += window[0x8] * b0[0x8];
      sum -= window[0x9] * b0[0x9];
      sum += window[0xA] * b0[0xA];
      sum -= window[0xB] * b0[0xB];
      sum += window[0xC] * b0[0xC];
      sum -= window[0xD] * b0[0xD];
      sum += window[0xE] * b0[0xE];
      sum -= window[0xF] * b0[0xF];
      WRITE_SAMPLE(samples,sum,clip);
  }

  {
      real sum;
      sum  = window[0x0] * b0[0x0];
      sum += window[0x2] * b0[0x2];
      sum += window[0x4] * b0[0x4];
      sum += window[0x6] * b0[0x6];
      sum += window[0x8] * b0[0x8];
      sum += window[0xA] * b0[0xA];
      sum += window[0xC] * b0[0xC];
      sum += window[0xE] * b0[0xE];
      WRITE_SAMPLE(samples,sum,clip);
      b0-=0x10,window-=0x20,samples+=step;
  }
  window += bo1<<1;

  for (j=15;j;j--,b0-=0x10,window-=0x20,samples+=step)
  {
      real sum;
      sum = -window[-0x1] * b0[0x0];
      sum -= window[-0x2] * b0[0x1];
      sum -= window[-0x3] * b0[0x2];
      sum -= window[-0x4] * b0[0x3];
      sum -= window[-0x5] * b0[0x4];
      sum -= window[-0x6] * b0[0x5];
      sum -= window[-0x7] * b0[0x6];
      sum -= window[-0x8] * b0[0x7];
      sum -= window[-0x9] * b0[0x8];
      sum -= window[-0xA] * b0[0x9];
      sum -= window[-0xB] * b0[0xA];
      sum -= window[-0xC] * b0[0xB];
      sum -= window[-0xD] * b0[0xC];
      sum -= window[-0xE] * b0[0xD];
      sum -= window[-0xF] * b0[0xE];
      sum -= window[-0x0] * b0[0xF];
      WRITE_SAMPLE(samples,sum,clip);
  }
    }
    *pnt += 128;

    return clip;
}



static void *mp3play_tilde_new(void)
{
    char *vec;
    t_mp3play_tilde *x = (t_mp3play_tilde *)pd_new(mp3play_tilde_class);

    x->file_is_open = 0;
    x->play_state = 0;
    x->mp3_encode_size = 0;
    x->mp3_out_index = 0;
    x->file_block_num = 0;
    x->file_remain = 0;
    x->file_size = 0;
    x->mp3_ch = 1;
    x->mp3_sr = 44100;
    x->obj_sr = 44100;
    x->obj_n = 1;
    x->samp_per_frame = 1152;
    x->mp3_byterate = 11;
    x->down = 0;
    x->scale = 1.0/32768.0;
    x->offset_sec = 0.0;
    x->fh = (FILE *)0L;
    x->mp3inbuf = (char *)getzbytes(MY_MP3_MALLOC_IN_SIZE);
    x->mp3outbuf = (char *)getzbytes(MY_MP3_MALLOC_OUT_SIZE);
    x->filename = (char *)getzbytes(MY_MP3_MALLOC_FN);
    x->begframeseek = (int *)0;
    x->curframeseek = 0;
    x->maxframeseek = 0;
    x->frame_counter = 0;
    x->time1_bang0_handle = 0;
    x->time_factor = 0.0;
    vec = x->filename;
    *vec = 0;
    InitMP3(&(x->mp));
    x->mp_is_init = 1;
    x->x_clock = clock_new(x, (t_method)mp3play_tilde_tick);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_floatout = outlet_new(&x->x_obj, &s_float);
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
    x->x_canvas = canvas_getcurrent();
    return (x);
}

static void mp3play_tilde_cleanup(t_mp3play_tilde *x)
{
    x->file_is_open = 0;
    fclose(x->fh);
    x->play_state = 0;
    x->mp3_out_index = 0;
    x->mp_is_init = 0;
    ExitMP3(&(x->mp));
    x->time1_bang0_handle = 0;
    clock_delay(x->x_clock, 0);
}

static int mp3play_tilde_read_frame_length_first(t_mp3play_tilde *x, int *frsz)
{
    int framesize, lsf, bitrate_index, sampling_frequency, padding, mpeg25, lay, ret=MP3_EX;
    int version, syncword;
    unsigned long head;
    unsigned char chead;

  size_t len;

    *frsz = 0;
    if((x->file_size) >= 4)
    {
  len = fread(&chead, 1, sizeof(char), x->fh);
  head = (unsigned int)chead;
  len = fread(&chead, 1, sizeof(char), x->fh);
  head <<= 8;
  head |= (unsigned int)chead;
  len = fread(&chead, 1, sizeof(char), x->fh);
  head <<= 8;
  head |= (unsigned int)chead;
  len = fread(&chead, 1, sizeof(char), x->fh);
  head <<= 8;
  head |= (unsigned int)chead;
  syncword = (head >> 20) & 0x0fff;
  version = ((head >> 19) & 0x01) ? 0 : 1;
  if((syncword & 0x01) == 0)
      version = 2;
  if(version == 0)
      x->samp_per_frame = 1152;
  else
      x->samp_per_frame = 576;

  if(head & (1<<20))
  {
      lsf = (head & (1<<19)) ? 0x0 : 0x1;
      mpeg25 = 0;
  }
  else
  {
      lsf = 1;
      mpeg25 = 1;
  }
  lay = 4-((head>>17)&3);
  if(((head>>10)&0x3) == 0x3)
  {
      post("Stream error");
      return(MP3_EX);
  }
  if(mpeg25)
      sampling_frequency = 6 + ((head>>10)&0x3);
  else
      sampling_frequency = ((head>>10)&0x3) + (lsf*3);
  x->mp3_sr = (int)iemmp3_freqs[sampling_frequency];
  if(mpeg25)
      bitrate_index = ((head>>12)&0xf);
  bitrate_index = ((head>>12)&0xf);
  padding   = ((head>>9)&0x1);
  if(!bitrate_index)
  {
      post("Free format not supported.\n");
      return(MP3_EX);
  }
  switch(lay)
  {
  case 1:
      post("Layer I not supported!\n");
      break;
  case 2:
      post("Layer II not supported!\n");
      break;
  case 3:
      framesize = iemmp3_tabsel_123[lsf][2][bitrate_index]*144000;
      framesize /= (iemmp3_freqs[sampling_frequency] << lsf);
      framesize += padding;
      *frsz = framesize;
      ret = 0;
      break;
  default:
      post("Sorry, unknown layer type.\n");
      break;
  }
  return(ret);
    }
    else
    {
  return(1);
    }
}

static int mp3play_tilde_read_frame_length_next(t_mp3play_tilde *x, int *frsz, int frame_counter)
{
    int framesize, lsf, bitrate_index, sampling_frequency, *begframeseek=x->begframeseek;
    unsigned long head;
    unsigned char chead;

    size_t len;

    *frsz = 0;
    if(begframeseek[frame_counter] < ((x->file_size) - 4))
    {
  len = fread(&chead, 1, sizeof(char), x->fh);
  head = (unsigned int)chead;
  len = fread(&chead, 1, sizeof(char), x->fh);
  head <<= 8;
  head |= (unsigned int)chead;
  len = fread(&chead, 1, sizeof(char), x->fh);
  head <<= 8;
  head |= (unsigned int)chead;
  len = fread(&chead, 1, sizeof(char), x->fh);
  head <<= 8;
  head |= (unsigned int)chead;
  if(head & (1<<20))
  {
      lsf = (head & (1<<19)) ? 0x0 : 0x1;
      sampling_frequency = ((head>>10)&0x3) + (lsf*3);
  }
  else
  {
      lsf = 1;
      sampling_frequency = 6 + ((head>>10)&0x3);
  }
  bitrate_index = ((head>>12)&0xf);
  framesize = iemmp3_tabsel_123[lsf][2][bitrate_index]*144000;
  framesize /= (iemmp3_freqs[sampling_frequency] << lsf);
  framesize += ((head>>9)&0x1);
  *frsz = framesize;
  return(0);
    }
    else
  return(1);
}

static int mp3play_tilde_calc_frames(t_mp3play_tilde *x)
{
    int *begframeseek=x->begframeseek, i, maxframeseek, framesize, frame_counter;
    float length;

    fseek(x->fh,0,SEEK_SET);
    if(!mp3play_tilde_read_frame_length_first(x, &framesize))
    {
  if(framesize <= 0)
  {
      return(MP3_EX);
  }
  i = framesize - 6;
  maxframeseek = (x->file_size) / i;
  if(!begframeseek)
  {
      x->begframeseek = (int *)getzbytes((maxframeseek)*sizeof(int));
      x->maxframeseek = maxframeseek;
  }
  else
  {
      if(maxframeseek > (x->maxframeseek))
      {
    freebytes(x->begframeseek, (x->maxframeseek)*sizeof(int));
    x->begframeseek = (int *)getzbytes((maxframeseek)*sizeof(int));
    x->maxframeseek = maxframeseek;
      }
  }
  begframeseek = x->begframeseek;
  begframeseek[0] = 0;
  begframeseek[1] = framesize;
  frame_counter = 1;
  /*x->curframeseek = maxframeseek;*/
  fseek(x->fh, begframeseek[1], SEEK_SET);
  while(!mp3play_tilde_read_frame_length_next(x, &framesize, frame_counter))
  {
      begframeseek[frame_counter+1] = begframeseek[frame_counter] + framesize;
      frame_counter++;
      fseek(x->fh, begframeseek[frame_counter], SEEK_SET);
  };
  frame_counter--;
  length = (float)(frame_counter)*(float)(x->samp_per_frame) / (float)(x->mp3_sr);
  x->length_sec = length;
  x->time_factor = length / (float)(frame_counter);
  x->curframeseek = frame_counter;
  fseek(x->fh,0,SEEK_SET);
  return(MP3_OK);
    }
    else
  return(MP3_EX);
}

static void mp3play_tilde_do_open(t_mp3play_tilde *x, char *str, int calc_it)
{
    int mp3_sr, obj_sr;
    int file_size, size, mp3_encode_return=MP3_OK, mp3_read_length, i, j, *begframeseek;
    static char *modes[4] = { "Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel" };
    static char *layers[4] = { "Unknown" , "I", "II", "III" };
    char completefilename[400];

    size_t len;

    if(x->file_is_open)
    {
      post("mp3play-ERROR: file is already open, please stop it first!");
    }
    else if(*str == 0)
    {
      post("mp3play-ERROR: there is no filename to open");
    }
    else
    {
      if(str[0] == '/')
      {
        strcpy(completefilename, str);
      }
      else if(((str[0] >= 'A')&&(str[0] <= 'Z')||
        (str[0] >= 'a')&&(str[0] <= 'z'))&&
        (str[1] == ':')&&(str[2] == '/'))
      {
        strcpy(completefilename, str);
      }
      else
      {
        strcpy(completefilename, canvas_getdir(x->x_canvas)->s_name);
        strcat(completefilename, "/");
        strcat(completefilename, str);
      }

  if((x->fh = sys_fopen(completefilename, "rb")) == NULL)
  {
      post("mp3play-ERROR: cannot open %s", completefilename);
  }
  else
  {
      strcpy(x->filename, completefilename);
      fseek(x->fh,0,SEEK_END);
      file_size = (int)ftell(x->fh);
      x->file_size = file_size;
      if(!x->mp_is_init)
      {
    InitAgainMP3(&(x->mp));
    x->mp_is_init = 1;
      }
      if(calc_it)
    mp3_encode_return = mp3play_tilde_calc_frames(x);
      if(mp3_encode_return == MP3_EX)
      {
    mp3play_tilde_cleanup(x);
    return;
      }
      if(x->frame_counter)
    x->frame_counter = (int)(x->offset_sec / x->time_factor);
      i = x->frame_counter;
      if(i > (x->curframeseek - 1))
    i = x->curframeseek - 1;
      begframeseek = x->begframeseek;
      j = begframeseek[i];
      fseek(x->fh,j,SEEK_SET);
      x->file_block_num = (file_size-j) / MY_MP3_MALLOC_IN_SIZE;
      x->file_remain = (file_size-j) - (x->file_block_num)*MY_MP3_MALLOC_IN_SIZE;
      if(x->file_block_num)
      {
    mp3_read_length = MY_MP3_MALLOC_IN_SIZE;
    x->file_block_num--;
      }
      else
      {
    mp3_read_length = x->file_remain;
    x->file_remain = 0;
      }
      if(mp3_read_length > 0)
      {
    len = fread(x->mp3inbuf, mp3_read_length, sizeof(char), x->fh);
    mp3_encode_return = decodeMP3(&(x->mp), x->mp3inbuf, mp3_read_length, x->mp3outbuf,
                MY_MP3_MALLOC_IN_SIZE2, &size);
    if(mp3_encode_return == MP3_EX)
    {
        mp3play_tilde_cleanup(x);
        return;
    }
    post ("MPEG %s, Layer: %s, Freq: %ld, mode: %s, modext: %d, BPF : %d",
          x->mp.fr.mpeg25 ? "2.5" : (x->mp.fr.lsf ? "2.0" : "1.0"),
          layers[x->mp.fr.lay],iemmp3_freqs[x->mp.fr.sampling_frequency],
          modes[x->mp.fr.mode],x->mp.fr.mode_ext,x->mp.fr.framesize+4);
    post ("Channels: %d, copyright: %s, original: %s, CRC: %s, emphasis: %d.",
          x->mp.fr.stereo,x->mp.fr.copyright?"Yes":"No",
          x->mp.fr.original?"Yes":"No",x->mp.fr.error_protection?"Yes":"No",
          x->mp.fr.emphasis);
    post ("Bitrate: %d Kbits/s, Extension value: %d",
          iemmp3_tabsel_123[x->mp.fr.lsf][x->mp.fr.lay-1]
          [x->mp.fr.bitrate_index],x->mp.fr.extension);
    post ("Original Soundfile-Length : %.3f sec.\n",x->length_sec);

    x->mp3_byterate = 128*iemmp3_tabsel_123[x->mp.fr.lsf][x->mp.fr.lay-1][x->mp.fr.bitrate_index];
    /* 1024/8 */

    if(x->mp.fr.stereo == 2)
        x->mp3_ch = 2;
    else if(x->mp.fr.stereo == 1)
        x->mp3_ch = 1;
    else
    {
        x->mp3_ch = 1;
        post("mp3_play~ WARNING: unknown number of channels : %d channels",
       x->mp.fr.stereo);
    }
    mp3_sr = (int)(iemmp3_freqs[x->mp.fr.sampling_frequency]);
    obj_sr = x->obj_sr;
    x->mp3_sr = mp3_sr;
    if(mp3_sr == obj_sr)
    {
        x->down = 0;
    }
    else if(2*mp3_sr == obj_sr)
    {
        x->down = 1;
    }
    else if(4*mp3_sr == obj_sr)
    {
        x->down = 2;
    }
    else if(mp3_sr == 2*obj_sr)
    {
        x->down = -1;
    }
    else if(mp3_sr == 4*obj_sr)
    {
        x->down = -2;
    }
    else
    {
        post("mp3_play~ WARNING: playing the filesamplerate of %d Hz at %d Hz", mp3_sr,obj_sr);
        x->down = 0;
    }

    if(mp3_encode_return == MP3_OK)
    {
        x->file_is_open = 1;
        x->mp3_encode_size = size * sizeof(char) / sizeof(short);
    }
    else
    {
        x->file_is_open = 0;
        x->mp3_out_index = 0;
        fclose(x->fh);
    }
      }
      else
      {
    x->file_is_open = 0;
    fclose(x->fh);
      }
  }
  x->play_state = 2;
  x->mp3_out_index = 0;
    }
}

static t_int *mp3play_tilde_perform(t_int *w)
{
    t_mp3play_tilde *x = (t_mp3play_tilde *)(w[1]);
    t_float *out1 = (t_float *)(w[2]);
    t_float *out2 = (t_float *)(w[3]);
    int n = (int)(w[4]);
    short *ivec = (short *)(x->mp3outbuf);
    int size, mp3_encode_return, mp3_read_length, mp3_ch, down;
    float scale = x->scale, outa, outb;
    int mp3_out_index = x->mp3_out_index;

    size_t len;

    if (!x->file_is_open)
  goto mp3play_tilde_labelzero;
    if (x->play_state != 1)
  goto mp3play_tilde_labelzero;

    if(mp3_out_index >= x->mp3_encode_size)
    {
  x->frame_counter++;
  x->time1_bang0_handle = 1;
  clock_delay(x->x_clock, 0);
  mp3_out_index = 0;
  mp3_encode_return = decodeMP3(&(x->mp), NULL, 0, x->mp3outbuf,
              MY_MP3_MALLOC_IN_SIZE2, &size);
  if(mp3_encode_return == MP3_OK)
      x->mp3_encode_size = size * sizeof(char) / sizeof(short);
  else if(mp3_encode_return == MP3_EX)
  {
      mp3play_tilde_cleanup(x);
      goto mp3play_tilde_labelzero;
  }
  else
  {
      if(x->file_block_num > 0)
      {
    mp3_read_length = MY_MP3_MALLOC_IN_SIZE;
    x->file_block_num--;
      }
      else if(x->file_remain > 0)
      {
    mp3_read_length = x->file_remain;
    x->file_remain = 0;
      }
      else
      {
    mp3play_tilde_cleanup(x);
    goto mp3play_tilde_labelzero;
      }

      len = fread(x->mp3inbuf, mp3_read_length, sizeof(char), x->fh);
      mp3_encode_return = decodeMP3(&(x->mp), x->mp3inbuf, mp3_read_length,
            x->mp3outbuf, MY_MP3_MALLOC_IN_SIZE2, &size);
      x->mp3_encode_size = size * sizeof(char) / sizeof(short);
      if(mp3_encode_return == MP3_EX)
      {
    mp3play_tilde_cleanup(x);
    goto mp3play_tilde_labelzero;
      }
  }
    }

    mp3_ch = x->mp3_ch;
    down = x->down;
    if(mp3_ch == 2)
    {
  if(down == 0)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + 2*n;
      while (n--)
      {
    *out1++ = scale * (float)(*ivec++);
    *out2++ = scale * (float)(*ivec++);
      }
  }
  else if(down == 1)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + n;
      n /= 2;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    outb = scale * (float)(*ivec++);
    *out1++ = outa;
    *out2++ = outb;
    *out1++ = outa;
    *out2++ = outb;
      }
  }
  else if(down == 2)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + n/2;
      n /= 4;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    outb = scale * (float)(*ivec++);
    *out1++ = outa;
    *out2++ = outb;
    *out1++ = outa;
    *out2++ = outb;
    *out1++ = outa;
    *out2++ = outb;
    *out1++ = outa;
    *out2++ = outb;
      }
  }
  else if(down == -1)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + 4*n;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    outb = scale * (float)(*ivec);
    ivec += 3;
    *out1++ = outa;
    *out2++ = outa;
      }
  }
  else if(down == -2)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + 8*n;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    outb = scale * (float)(*ivec);
    ivec += 7;
    *out1++ = outa;
    *out2++ = outa;
      }
  }
    }
    else
    {
  if(down == 0)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + n;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    *out1++ = outa;
    *out2++ = outa;
      }
  }
  else if(down == 1)
  {
      ivec += mp3_out_index;
      n /= 2;
      x->mp3_out_index = mp3_out_index + n;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    *out1++ = outa;
    *out2++ = outa;
    *out1++ = outa;
    *out2++ = outa;
      }
  }
  else if(down == 2)
  {
      ivec += mp3_out_index;
      n /= 4;
      x->mp3_out_index = mp3_out_index + n;
      while (n--)
      {
    outa = scale * (float)(*ivec++);
    *out1++ = outa;
    *out2++ = outa;
    *out1++ = outa;
    *out2++ = outa;
    *out1++ = outa;
    *out2++ = outa;
    *out1++ = outa;
    *out2++ = outa;
      }
  }
  else if(down == -1)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + 2*n;
      while (n--)
      {
    outa = scale * (float)(*ivec);
    ivec += 2;
    *out1++ = outa;
    *out2++ = outa;
      }
  }
  else if(down == -2)
  {
      ivec += mp3_out_index;
      x->mp3_out_index = mp3_out_index + 4*n;
      while (n--)
      {
    outa = scale * (float)(*ivec);
    ivec += 4;
    *out1++ = outa;
    *out2++ = outa;
      }
  }
    }
    return (w+5);

mp3play_tilde_labelzero:

    while (n--)
    {
  *out1++ = 0;
  *out2++ = 0;
    }
    return (w+5);
}

static void mp3play_tilde_dsp(t_mp3play_tilde *x, t_signal **sp)
{
    x->obj_sr = (int)(sp[0]->s_sr);
    x->obj_n = (int)(sp[0]->s_n);
    dsp_add(mp3play_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}



static void mp3play_tilde_stop(t_mp3play_tilde *x)
{
    if(x->file_is_open)
    {
  x->file_block_num = 0;
  x->file_remain = 0;
    }
}

static void mp3play_tilde_start(t_mp3play_tilde *x)
{
    if(x->file_is_open)
    {
  x->play_state = 1;
    }
}

static void mp3play_tilde_ft1(t_mp3play_tilde *x, t_floatarg offset)
{
    if(offset < 0.0)
  offset = 0.0;
    x->offset_sec = (float)offset;
}

static void mp3play_tilde_pause(t_mp3play_tilde *x)
{
    if(x->file_is_open)
    {
  if(x->play_state == 0)
      x->play_state = 1;
  else if(x->play_state == 1)
      x->play_state = 0;
    }
}

static void mp3play_tilde_open(t_mp3play_tilde *x, t_symbol *s)
{
    x->frame_counter = 0;
    mp3play_tilde_do_open(x, (char *)s->s_name, 1);
}

static void mp3play_tilde_open_again(t_mp3play_tilde *x)
{
    x->frame_counter = 0;
    mp3play_tilde_do_open(x, x->filename, 0);
}

static void mp3play_tilde_open_at(t_mp3play_tilde *x, t_symbol *s)
{
    x->frame_counter = 1;
    mp3play_tilde_do_open(x, (char *)s->s_name, 1);
}

static void mp3play_tilde_open_again_at(t_mp3play_tilde *x)
{
    x->frame_counter = 1;
    mp3play_tilde_do_open(x, x->filename, 0);
}


static void mp3play_tilde_tick(t_mp3play_tilde *x)
{
    if(x->time1_bang0_handle)
    {
  outlet_float(x->x_floatout, (float)(x->frame_counter)*(x->time_factor));
    }
    else
    {
  outlet_bang(x->x_bangout);
    }
}

static void mp3play_tilde_free(t_mp3play_tilde *x)
{
    if(x->mp_is_init)
  ExitMP3(&(x->mp));
    if(x->begframeseek)
  freebytes(x->begframeseek, (x->maxframeseek)*sizeof(int));
    freebytes(x->filename, MY_MP3_MALLOC_FN);
    freebytes(x->mp3outbuf, MY_MP3_MALLOC_OUT_SIZE);
    freebytes(x->mp3inbuf, MY_MP3_MALLOC_IN_SIZE);
    clock_free(x->x_clock);
}

void mp3play_tilde_setup(void)
{
    mp3play_tilde_class = class_new(gensym("mp3play~"), (t_newmethod)mp3play_tilde_new,
         (t_method)mp3play_tilde_free, sizeof(t_mp3play_tilde), 0, 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_start, gensym("start"), 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_ft1,
        gensym("ft1"), A_FLOAT, 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_stop, gensym("stop"), 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_pause, gensym("pause"), 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_open_again_at, gensym("open_again_at"), 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_open, gensym("open"), A_DEFSYM, 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_open_again, gensym("open_again"), 0);
    class_addmethod(mp3play_tilde_class, (t_method)mp3play_tilde_open_at, gensym("open_at"), A_DEFSYM, 0);
//    class_sethelpsymbol(mp3play_tilde_class, gensym("iemhelp/help-mp3play~"));
    /*post("\nmp3play~ written by thomas musil & norbert math\nV 0.1 iem graz
     austria 05 2000\n");*/
}
