/*
 *   DSPIfilters.h - Inline classes for biquad filters 
 *   Copyright (c) 2000 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef DSPIfilters_h
#define DSPIfilters_h


#include "DSPIcomplex.h"
#include "DSPI.h"
//#include <stdio.h>

/* orthogonal biquad */

class DSPIfilterOrtho {
    public:

        inline DSPIfilterOrtho(){resetState();resetCoef();resetSCoef();}
        inline ~DSPIfilterOrtho(){}

        inline void resetState(){d1A = d1B = d2A = d2B = 0.0;}
        inline void resetCoef(){ai = ar = c0 = c1 = c2 = 0.0;}
        inline void resetSCoef(){s_ai = s_ar = s_c0 = s_c1 = s_c2 = 0.0;}
        
        /*
         *		Biquad filters remarks
         *
         *		Q is defined with reference to the analog prototype:
         *		poles/zeros = w0 * (1/Q +- sqrt(1 - 1/Q^2))
         *
         *		the num/den polynomial then has the form:
         *		1 + 2s/Qw0 + (s/w0)^2
         *
         *		if Q < 1 => real valued poles/zeros
         *		if Q > 1 => complex values poles/zeros
         *		if Q = inf => imaginary poles/zeros
         *		if Q = sqrt(2) => 'maximally flat' poles/zeros
         *
         *		the analog prototypes are converted to the digital domain
         *		using the bilinear transform. hence the prewarping.
         */
        
        // make sure freq and Q are positive and within bounds
        inline void checkBounds(t_float &freq, t_float &Q)
        {
            freq = fabs(freq);
            Q = fabs(Q);

            t_float epsilon = .0001; // stability guard
            t_float fmin = 0.0 + epsilon;
            t_float fmax = 0.5 - epsilon;
            t_float Qmin = 1.1;

            if (freq < fmin) freq = fmin; 
            if (freq > fmax) freq = fmax;
            
            if (Q < Qmin) Q = Qmin;
            
        }
        
        inline void setAP(t_float freq, t_float Q) // allpass
        {
    
            // prototype: H(s) = (1 - 2s/Qw0 + (s/w0)^2) / (1 + 2s/Qw0 + (s/w0)^2)
            // s_p = - s_z (analog: symmetric wrt. im axis)
            // z_p = 1/z_z (digiatl: summ wrt. unit circle)
            checkBounds(freq, Q);

            // prewarp for bilin transfo
            freq = bilin_prewarp(freq);
            t_float zeta = 1.0/Q;
        
            DSPIcomplex p = bilin_stoz(DSPIcomplex(-zeta, (1.0-zeta*zeta))*freq);
            DSPIcomplex z = 1.0 / p;
            setPoleZeroNormalized(p, z, DSPIcomplex(1,0));
        
        
        }
        inline void setLP(t_float freq, t_float Q) // low pass
        {
            // prototype: H(s) = 1 / (1 + 2s/Qw0 + (s/w0)^2)
            // the bilinear transform has 2 zeros at NY
             
            checkBounds(freq, Q);
            freq = bilin_prewarp(freq);
            t_float zeta = 1/Q;
            
            DSPIcomplex p = bilin_stoz(DSPIcomplex(-zeta, (1.0-zeta*zeta))*freq);
            setPoleZeroNormalized(p, DSPIcomplex(-1, 0), DSPIcomplex(1, 0));
            
        }
        inline void setHP(t_float freq, t_float Q) // hi pass
        {
            // prototype: H(s) = (s/w0)^2 / (1 + 2s/Qw0 + (s/w0)^2)
            // the bilinear transform has 2 zeros at DC
             
            checkBounds(freq, Q);
            freq = bilin_prewarp(freq);
            t_float zeta = 1/Q;
            
            DSPIcomplex p = bilin_stoz(DSPIcomplex(-zeta, (1.0-zeta*zeta))*freq);
            setPoleZeroNormalized(p, DSPIcomplex(1, 0), DSPIcomplex(-1, 0));
            
        }

        inline void setBP(t_float freq, t_float Q) // band pass (1-allpass)
        {
            // prototype: 1/2 * (1 - H_allpass(z))
            setAP(freq, Q);
            t_float h = -0.5;
            c0 *= h;
            c1 *= h;
            c2 *= h;
            c0 -= h;
  
        }

        inline void setBR(t_float freq, t_float Q) // band reject
        {
            // prototype: H(s) = (1 - (s/w0)^2) / (1 + 2s/Qw0 + (s/w0)^2)
            checkBounds(freq, Q);
            // pole phasor
            DSPIcomplex z = DSPIcomplex(2.0 * M_PI * freq);
            
            // prewarp for bilin transfo
            freq = bilin_prewarp(freq);
            t_float zeta = 1/Q;
        
            DSPIcomplex p = bilin_stoz(DSPIcomplex(-zeta, (1.0-zeta*zeta))*freq);
            setPoleZeroNormalized(p, z, DSPIcomplex(1,0)); 
        }
        
        inline void setHS(t_float freq, t_float gain) // low shelf
        {
            // hi shelf = LP - g(LP-1)
            t_float Q = M_SQRT2;
            setLP(freq, Q); 
            c0 -= gain * (c0 - 1.0);
            c1 -= gain * (c1);
            c2 -= gain * (c2);
        }

        inline void setLS(t_float freq, t_float gain) // low shelf
        {
            // hi shelf = HP - g(HP-1)
            t_float Q = M_SQRT2;
            setHP(freq, Q); 
            c0 -= gain * (c0 - 1.0);
            c1 -= gain * (c1);
            c2 -= gain * (c2);
        }
        inline void setEQ(t_float freq, t_float Q, t_float gain)// param EQ
        {
            // EQ = (1+A)/2 + (1-A)/2 AP

            t_float a0 = 0.5 * (1.0 + gain);
            t_float a1 = 0.5 * (1.0 - gain);
            setAP(freq, Q);
            c0 *= a1;
            c1 *= a1;
            c2 *= a1;
            c0 += a0;
        }
        
        inline void setPoleZero
        (
            const DSPIcomplex& a, // pole
            const DSPIcomplex& b  // zero
        )
        {
            ar = a.r();
            ai = a.i();
            
            c0 = 1.0;
            c1 = 2.0 * (a.r() - b.r());
            c2 = (a.norm2() - b.norm2() - c1 * a.r()) / a.i();
        }

        
        inline void setPoleZeroNormalized
        (
            const DSPIcomplex& a, // pole
            const DSPIcomplex& b, // zero
            const DSPIcomplex& c  // gain = 1 at this freq
        )
        {
            setPoleZero(a, b);
            DSPIcomplex invComplexGain = ((c-a)*(c-a.conj()))/((c-b)*(c-b.conj()));
            t_float invGain = invComplexGain.norm();
            c0 *= invGain;
            c1 *= invGain;
            c2 *= invGain;
            
        }        

        
        // one channel bang
        inline void Bang
        (
            t_float &input, 
            t_float &output
        )
        {
            t_float d1t = ar * d1A + ai * d2A + input;
            t_float d2t = ar * d2A - ai * d1A;
            output = c0 * input + c1 * d1A + c2 * d2A;
            d1A = d1t;
            d2A = d2t;
        }
        
        // one channel bang smooth
        // a default s could be s = (1 - (.1)^(1/n))
        inline void BangSmooth
        (
            t_float &input, 	// input ref
            t_float &output,	// output ref
            t_float s 		// smooth pole
        )
        {
            t_float d1t = s_ar * d1A + s_ai * d2A + input;
            t_float d2t = s_ar * d2A - s_ai * d1A;
            s_ar += s * (ar - s_ar);
            s_ai += s * (ai - s_ai);
            output = s_c0 * input + s_c1 * d1A + s_c2 * d2A;
            d1A = d1t;
            d2A = d2t;
            s_c0 += s * (c0 - s_c0);
            s_c1 += s * (c1 - s_c1);
            s_c2 += s * (c2 - s_c2);
        }
        
        // two channel bang
        inline void Bang2
        (
            t_float &input1, 
            t_float &input2, 
            t_float &output1, 
            t_float &output2 
        )
        {
            t_float d1tA = ar * d1A + ai * d2A + input1;
            t_float d1tB = ar * d1B + ai * d2B + input2;
            t_float d2tA = ar * d2A - ai * d1A;
            t_float d2tB = ar * d2B - ai * d1B;
            output1 = c0 * input1 + d1A * c1 + d2A * c2;
            output2 = c0 * input2 + d1B * c1 + d2B * c2;
            d1A = d1tA;
            d2A = d2tA;
            d1B = d1tB;
            d2B = d2tB;
        }

        // two channel bang smooth
        inline void Bang2Smooth
        (
            t_float &input1, 
            t_float &input2, 
            t_float &output1, 
            t_float &output2,
            t_float s 
        )
        {
            t_float d1tA = s_ar * d1A + s_ai * d2A + input1;
            t_float d1tB = s_ar * d1B + s_ai * d2B + input2;
            t_float d2tA = s_ar * d2A - s_ai * d1A;
            t_float d2tB = s_ar * d2B - s_ai * d1B;
            s_ar += s * (ar - s_ar);
            s_ai += s * (ai - s_ai);
            output1 = s_c0 * input1 + d1A * s_c1 + d2A * s_c2;
            output2 = s_c0 * input2 + d1B * s_c1 + d2B * s_c2;
            d1A = d1tA;
            d2A = d2tA;
            d1B = d1tB;
            d2B = d2tB;
            s_c0 += s * (c0 - s_c0);
            s_c1 += s * (c1 - s_c1);
            s_c2 += s * (c2 - s_c2);
        }

	inline void killDenormals()
	{

	    // state data
	    t_float zero = 0.0;

	    d1A = DSPI_IS_DENORMAL(d1A) ? zero : d1A;
	    d2A = DSPI_IS_DENORMAL(d2A) ? zero : d2A;
	    d1B = DSPI_IS_DENORMAL(d1B) ? zero : d1B;
	    d2B = DSPI_IS_DENORMAL(d2B) ? zero : d2B;


	    /* test on athlon showed nuking smooth data does not
	     * present a noticable difference in performance however
	     * nuking state data is really necessary


	    // smooth data
	    t_float dai = ai - s_ai;
 	    t_float dar = ar - s_ar;
	    t_float dc0 = c0 - s_c0;
	    t_float dc1 = c1 - s_c1;
	    t_float dc2 = c2 - s_c2;


	    s_ai = DSPI_IS_DENORMAL(dai) ? ai : s_ai;
	    s_ar = DSPI_IS_DENORMAL(dar) ? ar : s_ar;
	    s_c0 = DSPI_IS_DENORMAL(dc0) ? c0 : s_c0;
	    s_c1 = DSPI_IS_DENORMAL(dc0) ? c1 : s_c1;
	    s_c2 = DSPI_IS_DENORMAL(dc0) ? c2 : s_c2;

	    */



	}
        
    private:
    
        // state data
        t_float d1A;
        t_float d2A;

        t_float d1B;
        t_float d2B;
        
        // pole data
        t_float ai;
        t_float s_ai;
        t_float ar;
        t_float s_ar;
        
        // zero data
        t_float c0;
        t_float s_c0;
        t_float c1;
        t_float s_c1;
        t_float c2;
        t_float s_c2;
        

};



class DSPIfilterSeries{
    public:
        inline DSPIfilterSeries() {DSPIfilterSeries(1);}
        inline ~DSPIfilterSeries() {delete [] biquad;};
        
        inline DSPIfilterSeries(int numberOfSections)
        {
            // create a set of biquads
            sections = numberOfSections;
            biquad = new DSPIfilterOrtho[numberOfSections];
        }

        inline void setButterHP(t_float freq)
        {
            /*  This member function computes the poles for a highpass butterworth filter.
             *  The filter is transformed to the digital domain using a bilinear transform.
             *  Every biquad section is normalized at NY.
             */
             
            t_float epsilon = .0001; // stability guard
            t_float min = 0.0 + epsilon;
            t_float max = 0.5 - epsilon;

            if (freq < min) freq = min; 
            if (freq > max) freq = max; 

            // prewarp cutoff frequency
            t_float omega = bilin_prewarp(freq);

            DSPIcomplex NY(-1,0);  //normalize at NY
            DSPIcomplex DC(1,0);  //all zeros will be at DC
            DSPIcomplex pole( (2*sections + 1) * M_PI / (4*sections)); // first pole of lowpass filter with omega == 1
            DSPIcomplex pole_inc(M_PI / (2*sections)); // phasor to get to next pole, see Porat p. 331
            
            for (int i=0; i<sections; i++)
            {
                // setup the biquad with the computed pole and zero and unit gain at NY
                biquad[i].setPoleZeroNormalized(
                    bilin_stoz(omega/pole),	// LP -> HP -> digital transfo 
                    DC, 					// all zeros at DC
                    NY);					// normalized (gain == 1) at NY
                pole *= pole_inc;			// compe next (lowpass) pole
            }
             
        }
        
        inline void setButterLP(t_float freq)
        {
            /*  This member function computes the poles for a lowpass butterworth filter.
             *  The filter is transformed to the digital domain using a bilinear transform.
             *  Every biquad section is normalized at DC.
             *  Doing it this way, only the pole locations need to be transformed.
             *  The constant gain factor can be computed by setting the DC gain of every section to 1.
             *  An analog butterworth is all-pole, meaning the bilinear transform has all zeros at -1
             */


            t_float epsilon = .0001; // stability guard
            t_float min = 0.0 + epsilon;
            t_float max = 0.5 - epsilon;


            if (freq < min) freq = min; 
            if (freq > max) freq = max; 

            // prewarp cutoff frequency
            t_float omega = bilin_prewarp(freq);
            
            DSPIcomplex DC(1,0);  //normalize at DC
            DSPIcomplex NY(-1,0); //all zeros will be at NY
            DSPIcomplex pole( (2*sections + 1) * M_PI / (4*sections)); 
            pole *= omega; // first pole, see Porat p. 331
            DSPIcomplex pole_inc(M_PI / (2*sections)); // phasor to get to next pole, see Porat p. 331
            
            for (int i=0; i<sections; i++)
            {
                // setup the biquad with the computed pole and zero and unit gain at DC
                biquad[i].setPoleZeroNormalized(bilin_stoz(pole), NY, DC);
                pole *= pole_inc;
            }
        }
        
        inline void resetState()
        {
            for (int i=0; i<sections; i++) biquad[i].resetState();
        }
        
        inline void Bang(t_float &input, t_float &output)
        {
            t_float x = input;
            for (int i=0; i<sections; i++)
            {
                biquad[i].Bang(x, x);
            }
            output = x;
        }
        inline void Bang2(t_float &input1, t_float &input2, t_float &output1, t_float &output2)
        {
            t_float x = input1;
            t_float y = input2;
            for (int i=0; i<sections; i++)
            {
                biquad[i].Bang2(x, y, x, y);
            }
            output1 = x;
            output2 = y;
        }
        
        inline void BangSmooth(t_float &input, t_float &output, t_float s)
        {
            t_float x = input;
            for (int i=0; i<sections; i++)
            {
                biquad[i].BangSmooth(x, x, s);
            }
            output = x;
        }
        inline void Bang2(t_float &input1, t_float &input2, t_float &output1, t_float &output2, t_float s)
        {
            t_float x = input1;
            t_float y = input2;
            for (int i=0; i<sections; i++)
            {
                biquad[i].Bang2Smooth(x, y, x, y, s);
            }
            output1 = x;
            output2 = y;
        }

    private:
        int sections;
        DSPIfilterOrtho *biquad;
        t_float gain;
};

#endif //DSPIfilters_h

