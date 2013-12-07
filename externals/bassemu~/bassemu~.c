/* (c) 2006 Ch. Klippel
 * this software is gpl'ed software, read the file "LICENSE.txt" for details
 */

#include "m_pd.h"
#include "math.h"

/* -------------------------- bassemu~ ------------------------------ */
static t_class *bassemu_class;

#define VER_MAJ	0
#define VER_MIN	3
#define PI_2   	6.28218530717958647692
#define sinfact (2. * 6.28328)

typedef struct _bassemu
{
    t_object x_obj;
	float vco_inc, vco_actinc;
	float current_wave, ideal_wave, delta, vco_count;
	float pw;
	int   vco_type, hpf;
	float glide;
	float thisnote;
	float tune;
  	float vcf_cutoff, vcf_envmod, vcf_envdecay, vcf_reso, vcf_rescoeff;
	float vcf_a, vcf_b, vcf_c, vcf_c0, vcf_d1, vcf_d2, vcf_e0, vcf_e1;
	int  	vcf_envpos;
  	float vca_attack;
	float vca_decay;
	float vca_a;
	float vca_a0;
	int 	vca_mode;
	int		limit_type;
	int		ext_type;
	char	ext_pre;
	int		envinc;
	float decay;
	float pitch;
	float sr;
	float dummy;
} t_bassemu;


static t_int *bassemu_perform(t_int *ww)
{
    t_bassemu *x = (t_bassemu *)(ww[1]);
    t_float *inbuf = (t_float *)(ww[2]);
    t_float *outbuf = (t_float *)(ww[3]);
    int n = (int)(ww[4]);

	float w = 0, k=0, ts=0, is=0;

	// only compute if needed .......
	if (x->vca_mode != 2)
	{
		// begin bassemu dsp engine
		while(n--)
		{

			if (x->ext_type > 0)
			{
					is = (*inbuf++ * 0.48);
					if (is < -0.48) is = -0.48f;
					if (is > 0.48) is = 0.48f;
			}
			// update vcf
			if(x->vcf_envpos >= x->envinc) {
				w = x->vcf_e0 + x->vcf_c0;
				k = exp(-w/x->vcf_rescoeff);
				x->vcf_c0 *= x->vcf_envdecay;
				x->vcf_a = 2.0*cos(2.0*w) * k;
				x->vcf_b = -k*k;
				x->vcf_c = 1.0 - x->vcf_a - x->vcf_b;
				x->vcf_envpos = 0;
			}

			// update vco
			if (!x->glide) x->vco_actinc = x->vco_inc;  // handle glide
			else
			{
				if (x->vco_inc > x->vco_actinc)
					x->vco_actinc = (x->vco_actinc +
											((x->vco_inc -
											x->vco_actinc) /
											(x->glide * (x->sr/10.)) ) );

				if (x->vco_inc < x->vco_actinc)
					x->vco_actinc = (x->vco_actinc -
											((x->vco_actinc -
											x->vco_inc) /
											(x->glide * (x->sr/10.)) ) );
			}

			// select waveform
			switch((int)x->vco_type)
			{
				case 0 : 	// sawtooth
								x->ideal_wave = sin(x->vco_count);
								x->vco_count += x->vco_actinc;
							break;

				case 1 :  // rectangle
								if ((x->vco_count+0.5) <= x->pw)
									x->ideal_wave = -0.5;
								else
									x->ideal_wave = 0.5;
								x->vco_count += x->vco_actinc;
								break;

				case 2 :  // triangle
								if (x->vco_count == -0.5 )
									x->ideal_wave = (x->vco_count + 0.000001);
								else
								{
									if (x->vco_count <= 0.0 )
										x->ideal_wave = (x->ideal_wave + (x->vco_actinc * 2));
									else
										x->ideal_wave = (x->ideal_wave - (x->vco_actinc * 2));
								}
								x->vco_count += x->vco_actinc;
								break;

				case 3 : // sine
								x->ideal_wave = (sin(sinfact * (x->vco_count + 0.5)) / 2);
								x->vco_count += (x->vco_actinc / 2.);
								break;
				default : break;
			}


			// waveform rises faster than it falls
			if( x->vco_count <= 0.0 )
				x->current_wave = ( x->current_wave + ((x->ideal_wave - x->current_wave) *	0.95 ));
			else
				x->current_wave = ( x->current_wave + ((x->ideal_wave - x->current_wave) * 0.9 ));




			// recyle and end
			if (x->vco_count > 0.5)
				x->vco_count = (-0.5);

			// run external through VCO-HPF
			if(x->ext_pre)
				switch((int)x->ext_type)
				{
					case 1 :
									x->current_wave = is;
									break;
					case 2 :
									x->current_wave = ((x->current_wave + is) *0.5f);
									break;
					default : break;
				}

			ts = x->current_wave;

			// vco hpf function
			if( x->hpf )
			{
				x->delta = (x->delta * x->current_wave);
				x->delta = (x->delta * 0.99 );
				ts = ((x->delta*2)+0.5);
				x->delta = (x->delta - x->current_wave );
			}

			// update vca
			if(!x->vca_mode)
				x->vca_a += (x->vca_a0 - x->vca_a) * x->vca_attack;
			else if(x->vca_mode == 1)
			{
				x->vca_a *= x->vca_decay;
				if(x->vca_a < (1/65536.0))
				{
					x->vca_a = 0;
					x->vca_mode = 2;
				}
			}

			// mix external without filtering with VCO-HPF
			if(!x->ext_pre)
				switch((int)x->ext_type)
				{
					case 1 :
									ts = is;
									break;
					case 2 :
									ts = ((ts + is) * 0.5f);
									break;
					default : break;
				}


			// compute sample
			ts = x->vcf_a * x->vcf_d1 + x->vcf_b * x->vcf_d2 + x->vcf_c * ts * x->vca_a;
			x->vcf_d2 = x->vcf_d1;
			x->vcf_envpos++;
			x->vcf_d1 = ts;

			switch((int)x->limit_type)
			{
				case 1 :   //		 hard limit
									if (ts >  0.999) ts =  0.999;
									if (ts < -0.999) ts = -0.999;
									*outbuf++ = ts;
									break;

				case 2 :   //    sine limiting
									*outbuf++ = sin(ts);
									break;

				default :  //    no limiting et al
									*outbuf++ = ts;
									break;
			}

		}
	} //end vcamode != 2
	else
		while(n--)
		{
			*outbuf++ = 0.0f;
		}

	return (ww+5);
}

static void bassemu_dsp(t_bassemu *x, t_signal **sp)
{
	x->sr = sp[0]->s_sr;
	dsp_add(bassemu_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void recalc(t_bassemu *x)
{
  	x->vcf_e1 = exp(6.109 + 1.5876*(x->vcf_envmod) + 2.1553*(x->vcf_cutoff) - 1.2*(1.0-x->vcf_reso));
	x->vcf_e0 = exp(5.613 - 0.8*(x->vcf_envmod) + 2.1553*(x->vcf_cutoff) - 0.7696*(1.0-x->vcf_reso));
	x->vcf_e0 *=M_PI/x->sr;
	x->vcf_e1 *=M_PI/x->sr;
	x->vcf_e1 -= x->vcf_e0;
	x->vcf_envpos = x->envinc;
}

static void bassemu_note(t_bassemu *x, t_floatarg f)
{
// calculate note and trigger vca
	if(f != -1) { // note
		x->thisnote = x->pitch + f-57;
		x->vco_inc = ((x->tune/x->sr)*pow(2, (x->thisnote)*(1.0/12.0)) / 2.);
		x->vca_mode = 0;
		x->vcf_c0 = x->vcf_e1;
		x->vcf_envpos = x->envinc;
	}
	else x->vca_mode = 1;
}

static void bassemu_pitch(t_bassemu *x, t_floatarg f)
{

	x->thisnote -= x->pitch;
	x->pitch = f;
	x->thisnote += x->pitch;
	x->vco_inc = ((x->tune/x->sr)*pow(2, (x->thisnote)*(1.0/12.0)) / 2.);
}

static void bassemu_list(t_bassemu *x, t_symbol *s, int argc, t_atom *argv)
{
	if (argc >= 5)
	{
		// get decay
		if(argv[4].a_type == A_FLOAT && (atom_getfloatarg(4,argc,argv) != -1))
		{ // decay
			float d = atom_getfloatarg(4,argc,argv);
			x->decay = d;
			d = 0.2 + (2.3*d);
			d*=x->sr;
			x->vcf_envdecay = pow(0.1, 1.0/d * x->envinc);
		}
		recalc(x);
	}
	if (argc >= 4)
	{
		// get envelope modulation
		if(argv[3].a_type == A_FLOAT && (atom_getfloatarg(3,argc,argv) != -1))
		{ // envmod
			x->vcf_envmod = atom_getfloatarg(1,argc,argv);
		}
		recalc(x);
	}
	if (argc >= 3)
	{
		//get resonance
		if(argv[2].a_type == A_FLOAT && (atom_getfloatarg(2,argc,argv) != -1))
		{ // resonance
			x->vcf_reso = atom_getfloatarg(1,argc,argv);
			x->vcf_rescoeff = exp(-1.20 + 3.455*(x->vcf_reso));
		}
		recalc(x);
	}
	if (argc >= 2)
	{
		// get cutoff
		if(argv[1].a_type == A_FLOAT && (atom_getfloatarg(1,argc,argv) != -1))
		{ // cutoff
			x->vcf_cutoff = atom_getfloatarg(1,argc,argv);
		}
		recalc(x);
	}
	if (argc >= 1)
	{
		if(argv[0].a_type = A_FLOAT && (atom_getfloatarg(0,argc,argv) != -1))
		{ // note
			x->thisnote = atom_getfloatarg(0,argc,argv)-57;
			x->vco_inc = ((x->tune/x->sr)*pow(2, (x->thisnote)*(1.0/12.0)) / 2.);
			x->vca_mode = 0;
			x->vcf_c0 = x->vcf_e1;
			x->vcf_envpos = x->envinc;
		}
		else
			x->vca_mode = 1;
		recalc(x);
	}
}

static void bassemu_vco(t_bassemu *x, t_floatarg f)
{
	if ((f >= 0) && (f <= 8))
		x->vco_type = f;
	else
		x->vco_type = 0;
}

static void bassemu_hpf(t_bassemu *x, t_floatarg f)
{
	if ((f >= 0) && (f <= 1))
		x->hpf = f;
	else
		x->hpf = 0;
}

static void bassemu_glide(t_bassemu *x, t_floatarg f)
{
	if (f == 0)
		x->glide = 0;
	else
		x->glide = f;
}

static void bassemu_limit(t_bassemu *x, t_floatarg f)
{
	if ((f >= 0) && (f <=2)) x->limit_type = f;
}

static void bassemu_ext(t_bassemu *x, t_floatarg f)
{
	if ((f >= 0) && (f <=2)) { x->ext_type = f; x->ext_pre = 0; }
	if (f == 3) { x->ext_type = 1; x->ext_pre = 1; }
	if (f == 4) { x->ext_type = 2; x->ext_pre = 1; }
}

static void bassemu_tune(t_bassemu *x, t_floatarg f)
{
	x->tune = f;
	x->vco_inc = ((x->tune/x->sr)*pow(2, (x->thisnote)*(1.0/12.0)) / 2.0);
}

static void bassemu_envinc(t_bassemu *x, t_floatarg f)
{
	float d = x->decay;

	if (f >= 1) x->envinc = f;
	d = 0.2 + (2.3*d);
	d *= x->sr;
	x->vcf_envdecay = pow(0.1, 1.0/d * x->envinc);
}

static void bassemu_cutoff(t_bassemu *x, t_floatarg f)
{
	x->vcf_cutoff = f;
	recalc(x);
}

static void bassemu_reso(t_bassemu *x, t_floatarg f)
{
	x->vcf_reso = f;
	x->vcf_rescoeff = exp(-1.20 + 3.455*(x->vcf_reso));
	recalc(x);
}

static void bassemu_envmod(t_bassemu *x, t_floatarg f)
{
	x->vcf_envmod = f;
	recalc(x);
}

static void bassemu_decay(t_bassemu *x, t_floatarg f)
{
	float d = f;
	x->decay = d;
	d = 0.2 + (2.3*d);
	d*=x->sr;
	x->vcf_envdecay = pow(0.1, 1.0/d * x->envinc);
}

static void bassemu_pw(t_bassemu *x, t_floatarg f)
{
	x->pw = f;

	if (x->pw > 1.0)
		x->pw = 1.0;

	if (x->pw < 0.0)
		x->pw = 0.0;
}

static void bassemu_reset(t_bassemu *x, t_floatarg f)
{
	x->vco_inc = 0.0f;
	x->vco_actinc = 0.0f;
	x->current_wave = 0.0f;
	x->ideal_wave = 0.0f;
	x->delta = 0.0f;
	x->vco_count = 0.0f;
	x->pw = 0.5f;
	x->vco_type = 0;
	x->hpf = 0.0f;
	x->glide = 0.0f;
	x->tune = 440.0f;
	x->thisnote = 0;

	x->vcf_cutoff = 0.0;
	x->vcf_envmod = 0.0;
	x->vcf_envdecay = 0.0;
	x->vcf_reso = 0.0;
	x->vcf_rescoeff = 0.0f;
	x->vcf_a = 0.0;
	x->vcf_b = 0.0;
	x->vcf_c = 0.0;
	x->vcf_c0 = 0.0;
	x->vcf_d1 = 0.0;
	x->vcf_d2 = 0.0;
	x->vcf_e0 = 0.0;
	x->vcf_e1 = 0.0f;
	x->vcf_envpos = 64;
	x->vca_attack = (float)(1.0f - 0.94406088f);
	x->vca_decay  = (float)(0.99897516f);
	x->vca_a = 0.0f;
	x->vca_a0 = 0.5f;
	x->vca_mode = 2 ; // attack (0) / decay (1) / silent (2) mode

	x->limit_type = 2;

	x->ext_type = 0;
	x->ext_pre	= 0;

	x->envinc = 64;
	x->decay  = 0;
	x->pitch = 0;

}

static void *bassemu_new(t_symbol *s, int argc, t_atom *argv)
{
	unsigned int numargs;
    t_bassemu *x = (t_bassemu *)pd_new(bassemu_class);

	outlet_new(&x->x_obj, gensym("signal"));
	bassemu_reset(x,0);
	x->sr = 44100.;
    return (x);
}

static void bassemu_free(t_bassemu *x)
{
}


void bassemu_tilde_setup(void)
{
    bassemu_class = class_new(gensym("bassemu~"), (t_newmethod)bassemu_new,
        (t_method)bassemu_free, sizeof(t_bassemu), CLASS_DEFAULT, A_GIMME, 0);

    CLASS_MAINSIGNALIN(bassemu_class, t_bassemu, dummy);

    class_addmethod(bassemu_class, (t_method)bassemu_dsp,	gensym("dsp"), 0);
	class_addfloat (bassemu_class, (t_method)bassemu_note); // start/stop using a toggle
	class_addmethod(bassemu_class, (t_method)bassemu_list,	gensym("list"),		A_GIMME, 	0);
	class_addmethod(bassemu_class, (t_method)bassemu_vco,	gensym("vco"), 		A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_hpf,	gensym("hpf"), 		A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_glide,	gensym("glide"),	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_limit,	gensym("limit"), 	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_ext,	gensym("ext"), 		A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_tune,	gensym("tune"),		A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_envinc,gensym("envinc"),	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_reset,	gensym("reset"),	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_cutoff,gensym("cutoff"),	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_reso,	gensym("reso"),		A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_envmod,gensym("envmod"),	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_decay,	gensym("decay"),	A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_pw,	gensym("pw"),		A_DEFFLOAT, 0);
	class_addmethod(bassemu_class, (t_method)bassemu_pitch,	gensym("pitch"),	A_DEFFLOAT, 0);

	logpost(NULL, 4, "bassemu~: transistor bass emulation");
	logpost(NULL, 4, "bassemu~: version %i.%i",VER_MAJ, VER_MIN);
	logpost(NULL, 4, "bassemu~: (c) 2006 Ch. Klippel - ck@mamalala.de");
	logpost(NULL, 4, "bassemu~: this is gpl'ed software, see README for details\n");
}

