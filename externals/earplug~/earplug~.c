/* RT binaural filter: earplug~        */
/* based on KEMAR impulse measurement  */
/* Pei Xiang, summer 2004              */
/* Revised in Fall 2006 by Jorge Castellanos */
/* Revised in Spring 2009 by Hans-Christoph Steiner to compile in the data file */

#include <stdio.h>
#include <math.h>
#include "m_pd.h"
#include "earplug~.h"
#ifdef _MSC_VER /* Thes pragmas only apply to Microsoft's compiler */
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* elevation degree:      -40  -30  -20  -10   0   10  20  30  40  50  60  70  80  90   */
/* index array:             0    1    2    3   4    5   6   7   8   9  10  11  12  13   */
/* impulse reponse number: 29   31   37   37  37   37  37  31  29  23  19  13   7   1   */ 
/* 0 degree reponse index:  0   29   60   97 134  171 208 245 276 305 328 347 360 367   */

static t_class *earplug_class;

typedef struct _earplug
{
    t_object x_obj; 
    t_outlet *left_channel ; 
    t_outlet *right_channel ; 

    t_float azimuth ;             /* from 0 to 360 degrees */
    t_float elevation ;           /* from -40 to 90 (degrees) */
    t_float azi ;
    t_float ele ;
     
    t_float crossCoef[8192] ; 
    t_float azimScale[13] ;
    t_int azimOffset[13] ;

    t_float previousImpulse[2][128] ;
    t_float currentImpulse[2][128] ;
    t_float convBuffer[128] ;
    t_float (*impulses)[2][128];          /* a 3D array of 368x2x128 */
    t_float f ;                   /* dummy float for dsp */
    t_int bufferPin;
} t_earplug;

static t_int *earplug_perform(t_int *w)
{
    t_earplug *x = (t_earplug *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *right_out = (t_float *)(w[3]);
    t_float *left_out = (t_float *)(w[4]); 
    int blocksize = (int)(w[5]);

    unsigned ch_L, ch_R, i;

    x->azi = x->azimuth ; 
    x->ele = x->elevation ;
	
    if (x->ele < -40) 
		x->ele = -40;
    if (x->ele > 90)
		x->ele = 90;
    if (x->azi < 0 || x->azi > 360) 
		x->azi = 0;
    if (x->azi <= 180)
    {
		ch_L = 0;
		ch_R = 1;
	}
    else
    { 
		ch_L = 1;
		ch_R = 0;
		x->azi = 360.0 - x->azi;
	}
	
    x->ele *= 0.1;   /* divided by 10 since each elevation is 10 degrees apart */

    if (x->ele < 8.) // if elevation is less than 80 degrees...
    { 
		int elevInt = (int)floor(x->ele); // A quantized version of the elevation    
		unsigned elevGridIndex = elevInt + 4;  // Used as the index to the array of scaling factors for the azimuth (adding 4 because the lowest elevation is -4, so it starts at 0)
		unsigned azimIntUp = (unsigned)(x->azi * x->azimScale[elevGridIndex+1]); // 
        float azimFracUp = azimIntUp + 1.0 - x->azi * x->azimScale[elevGridIndex+1];
		float azimFracUpInv = 1.0 - azimFracUp;
		float elevFracUp = x->ele - elevInt * 1.0;
    	unsigned azimIntDown = (unsigned)(x->azi * x->azimScale[elevGridIndex]);
        float azimFracDown = azimIntDown + 1.0 - x->azi * x->azimScale[elevGridIndex];
		float azimFracDownInv = 1.0 - azimFracDown;
		float elevFracDown = 1.0 - elevFracUp;
		unsigned lowerIdx = x->azimOffset[elevGridIndex] + azimIntDown;
		unsigned upperIdx = x->azimOffset[elevGridIndex + 1] + azimIntUp;
		
		for (i = 0; i < 128; i++)
		{
			x->currentImpulse[ch_L][i] = elevFracDown *		// Interpolate the lower two HRIRs and multiply them by their "fraction"
										(azimFracDown * x->impulses[lowerIdx][0][i] + 
										azimFracDownInv * x->impulses[lowerIdx + 1][0][i]) + 
										elevFracUp *		// Interpolate the upper two HRIRs and multiply them by their "fraction"
										(azimFracUp * x->impulses[upperIdx][0][i] + 
										azimFracUpInv * x->impulses[upperIdx + 1][0][i]);   

			x->currentImpulse[ch_R][i] = elevFracDown *
										(azimFracDown * x->impulses[lowerIdx][1][i] + 
										azimFracDownInv * x->impulses[lowerIdx + 1][1][i]) + 
										elevFracUp * 
										(azimFracUp * x->impulses[upperIdx][1][i] + 
										azimFracUpInv * x->impulses[upperIdx + 1][1][i]);  
		}   

    }
    else	// if elevation is 80 degrees or more the interpolation requires only three points (because there's only one HRIR at 90 deg)
    {
    	unsigned azimIntDown = (unsigned)(x->azi * 0.033333); // Scale the azimuth to 12 (the number of HRIRs at 80 deg) discreet points
        float azimFracDown = azimIntDown + 1.0 - x->azi * 0.033333;
        float elevFracUp = x->ele - 8.0;
		float elevFracDown = 9.0 - x->ele;
		for (i = 0; i < 128; i++) {
			
			x->currentImpulse[ch_L][i] = elevFracDown * 
										( azimFracDown * x->impulses[360+azimIntDown][0][i] +	// These two lines interpolate the lower two HRIRs
										(1.0 - azimFracDown) * x->impulses[361+azimIntDown][0][i] )
										+ elevFracUp * x->impulses[367][0][i];	// multiply the 90 degree HRIR with its corresponding fraction
			x->currentImpulse[ch_R][i] = elevFracDown * 
										(azimFracDown * x->impulses[360+azimIntDown][1][i]  +
										(1.0 - azimFracDown) * x->impulses[361+azimIntDown][1][i])
										+ elevFracUp * x->impulses[367][1][i]; 
		}

	}

    float inSample;
	float convSum[2]; // to accumulate the sum during convolution.
    int blockScale = 8192 / blocksize;

	// Convolve the - interpolated - HRIRs (Left and Right) with the input signal.
    while (blocksize--)
    {
		convSum[0] = 0; 
		convSum[1] = 0;	

		inSample = *(in++);

		x->convBuffer[x->bufferPin] = inSample;
		unsigned scaledBlocksize = blocksize * blockScale;
		unsigned blocksizeDelta = 8191 - scaledBlocksize;
		for (i = 0; i < 128; i++)
		{ 
			convSum[0] += (x->previousImpulse[0][i] * x->crossCoef[blocksizeDelta] + 
							x->currentImpulse[0][i] * x->crossCoef[scaledBlocksize]) * 
							x->convBuffer[(x->bufferPin - i) &127];
			convSum[1] += (x->previousImpulse[1][i] * x->crossCoef[blocksizeDelta] + 
							x->currentImpulse[1][i] * x->crossCoef[scaledBlocksize]) * 
							x->convBuffer[(x->bufferPin - i) &127];

			x->previousImpulse[0][i] = x->currentImpulse[0][i];
			x->previousImpulse[1][i] = x->currentImpulse[1][i];
		}	
		x->bufferPin = (x->bufferPin + 1) & 127;
  
        *left_out++ = convSum[0];
     	*right_out++ = convSum[1];
    }
    return (w+6);
}

static void earplug_dsp(t_earplug *x, t_signal **sp)
{
		// callback, params, userdata, in_samples, out_L,		out_R,		blocksize.
    dsp_add(earplug_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void *earplug_new(t_floatarg azimArg, t_floatarg elevArg)
{
    t_earplug *x = (t_earplug *)pd_new(earplug_class);
    x->left_channel = outlet_new(&x->x_obj, gensym("signal"));
    x->right_channel = outlet_new(&x->x_obj, gensym("signal"));
    floatinlet_new(&x->x_obj, &x->azimuth) ;     /* 0 to 360 degrees */
    floatinlet_new(&x->x_obj, &x->elevation) ;   /* -40 to 90 degrees */

    x->azimuth = azimArg ;
    x->elevation = elevArg ;

    int i, j;
    FILE *fp;
    char buff[1024], *bufptr;
    int filedesc;

	filedesc = open_via_path(canvas_getdir(canvas_getcurrent())->s_name, "earplug_data.txt", "", buff, &bufptr, 1024, 0 );

    if (filedesc >= 0) // If there was no error opening the text file...
    {	
		post("[earplug~] found impulse reponse file, overriding defaults:") ;
        post("let's try loading %s/earplug_data.txt", buff);
        fp = fdopen(filedesc, "r") ;  
        for (i = 0; i < 368; i++) 
        {
            while(fgetc(fp) != 10) ;
            for (j = 0 ; j < 128 ; j++) {
				fscanf(fp, "%f %f ", &earplug_impulses[i][0][j], &earplug_impulses[i][1][j]);
            }
        }
        fclose(fp) ;
    }
    x->impulses = earplug_impulses;
    
    post("        earplug~: binaural filter with measured reponses\n") ;
    post("        elevation: -40 to 90 degrees. azimuth: 360") ;
    post("        dont let blocksize > 8192\n"); 
       
    for (i = 0; i < 128 ; i++)
    {
         x->convBuffer[i] = 0; 
		 x->previousImpulse[0][i] = 0; 
		 x->previousImpulse[1][i] = 0;
    }
	
    x->bufferPin = 0;

    for (i = 0; i < 8192 ; i++)
    {	
		x->crossCoef[i] = 1.0 * i / 8192;
	}

	// This is the scaling factor for the azimuth so that it corresponds to an HRTF in the KEMAR database
    x->azimScale[0] = 0.153846153; x->azimScale[8] = 0.153846153;   /* -40 and 40 degree */
    x->azimScale[1] = 0.166666666; x->azimScale[7] = 0.166666666;   /* -30 and 30 degree */
    x->azimScale[2] = 0.2; x->azimScale[3]=0.2; x->azimScale[4]=0.2; x->azimScale[5]=0.2; x->azimScale[6]=0.2;  /* -20 to 20 degree */
    x->azimScale[9] = 0.125;  		/* 50 degree */
    x->azimScale[10] = 0.1;		/* 60 degree */
    x->azimScale[11] = 0.066666666;      /* 70 degree */
    x->azimScale[12] = 0.033333333;	/* 80 degree */

    x->azimOffset[0] = 0 ; 
    x->azimOffset[1] = 29 ;
    x->azimOffset[2] = 60 ;
    x->azimOffset[3] = 97 ;
    x->azimOffset[4] = 134 ;
    x->azimOffset[5] = 171 ;
    x->azimOffset[6] = 208 ;
    x->azimOffset[7] = 245 ;
    x->azimOffset[8] = 276 ;
    x->azimOffset[9] = 305 ;
    x->azimOffset[10] = 328 ;
    x->azimOffset[11] = 347 ;
    x->azimOffset[12] = 360 ;

    return (x);
}

void earplug_tilde_setup(void)
{
    earplug_class = class_new(gensym("earplug~"), (t_newmethod)earplug_new, 0,
    	sizeof(t_earplug), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
   
    CLASS_MAINSIGNALIN(earplug_class, t_earplug, f);
   
    class_addmethod(earplug_class, (t_method)earplug_dsp, gensym("dsp"), 0);
}
