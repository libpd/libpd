/* this file contains a 37th attempt to write a general purpose iir filter toolset */

/* defined as inline functions with call by reference to enable compiler ref/deref optim */

/* the typedef */
#ifndef T
#define T t_float
#endif


/* the prototype for a word */
#define P static inline void
#define PP static void


/* the 'reference' arguments */
#define A *a
#define B *b
#define C *c
#define D *d
#define X *x
#define Y *y
#define S *s


/* the opcodes */

/* add */
P cadd  (T X, T Y, T A, T B, T C, T D) { X = A + C; Y = B + D;}
P cadd2 (T A, T B, T C, T D)           { A += C; B += D;}
P vcadd  (T X, T A, T C)                { cadd(x,x+1,a,a+1,c,c+1); }
P vcadd2 (T A, T C)                     { cadd2(a,a+1,c,c+1); }


/* mul */
P cmul_r (T X, T A, T B, T C, T D)    { X = A * C - B * D;}
P cmul_i (T Y, T A, T B, T C, T D)    { Y = A * D + B * C;}
P cmul (T X, T Y, T A, T B, T C, T D) 
{
    cmul_r (x, a, b, c, d);
    cmul_i (y, a, b, c, d);
}
P cmul2 (T A, T B, T C, T D)
{
    T x = A;
    T y = B;
    cmul (&x, &y, a, b, c, d);
    A = x;
    B = y;
}

P vcmul  (T X, T A, T C)  { cmul(x,x+1,a,a+1,c,c+1); }
P vcmul2 (T A, T C)       { cmul2(a,a+1,c,c+1); }


/* norm */
static inline t_float vcnorm(T X) { return hypot(x[0], x[1]); }



/* swap */
P vcswap(T Y, T X)
{
    t_float t[2] = {x[0], x[1]};
    x[0] = y[0];
    x[1] = y[1];
    y[0] = t[0];
    y[1] = t[1];
}


/* inverse */
P vcinv(T Y, T X)
{
    t_float scale = 1.0 / vcnorm(x);
    y[0] = scale * x[0];
    y[1] = scale * x[1];
}

P vcinv1(T X)
{
    t_float scale = 1.0 / vcnorm(x);
    x[0] *= scale;
    x[1] *= scale;
}

/* exp */

/* X = exp(Y) */
P vcexp2 (T Y, T X)
{
    T r = exp(x[0]);
    y[0] = cos (x[1]);
    y[1] = sin (x[1]);
    y[0] *= r;
    y[1] *= r;
}

P vcexp1 (T X)
{
    T y[2];
    vcexp2(y,x);
    x[0] = y[0];
    x[1] = y[1];
}

/* 
   FILTERS

   the transfer function is defined in terms of the "engineering" 
   bilateral z-transform of the discrete impulse response h[n].

   H(z) = Sum{n = -inf -> inf} h[n] z^(-n)

   a unit delay operating on a singnal S(z) is therefore 
   represented as z^(-1) S(z)

*/







/* biquads */

/* biquad, orthogonal (poles & zeros), real in, out, state, pole, output */
P biq_orth_r (T X, T Y, T S, T A, T C)
{
    Y = X + c[0] * s[0] - c[1] * s[1]; /* mind sign of c[1] */
    vcmul2(s, a);
    S += X;
}


/* biquad, orthogonal, complex one-pole, with scaling */

/* complex one pole: (output = s[0] + is[1]): C / (1-A z^(-1))  */

P one_pole_complex (T X, T Y, T S, T A, T C)
{
    vcmul(y, s, a);
    vcadd2(y, x);
    s[0] = y[0];
    s[1] = y[1];
    vcmul(y, s, c);
}

/* complex conj two pole:  (output = s[0]  : (Re(C) - Re(C*Conj(A))) / (1 - A z^(-1)) (1 - Conj(A) z^(-1)) */

P two_pole_complex_conj (T X, T Y, T S, T A, T C)
{
    vcmul2(s, a);
    s[0] += x[0];
    y[0] = s[0] * c[0] - s[1] * c[1];
}



/* support functions for IIR filter design */

/* evaluate pole and allzero TF in z^-1 given the complex zeros/poles:
   p(z) (or p(z)^-1) = \product (1-z_i z^-1) */
PP eval_zero_poly(t_float *val, t_float *arg, t_float *zeros, int nb_zeros)
{
    int i;
    t_float a[2] = {arg[0], arg[1]};
    vcinv1(a);
    val[0] = 1.0;
    val[1] = 0.0;
    a[0] *= -1;
    a[1] *= -1;
    for (i=0; i<nb_zeros; i++){
	t_float t[2];
	vcmul(t, a, zeros + 2*i);
	t[0] += 1.0;
	vcmul2(val, t);
    }
}

PP eval_pole_poly(t_float *val, t_float *arg, t_float *poles, int nb_poles)
{
    eval_zero_poly(val, arg, poles, nb_poles);
    vcinv1(val);
}


/* since it's more efficient to store half of the poles for a real impulse
   response, these functions compute p(z) conj(p(conj(z)))  */

PP eval_conj_zero_poly(t_float *val, t_float *arg, t_float *zeros, int nb_zeros)
{
    t_float t[2];
    t_float a[2] = {arg[0], arg[1]};
    eval_zero_poly(t, a, zeros, nb_zeros);
    a[1] *= -1;
    eval_zero_poly(val, a, zeros, nb_zeros);
    val[1] *= -1;
    vcmul2(val, t);
}

PP eval_conj_pole_poly(t_float *val, t_float *arg, t_float *poles, int nb_poles)
{
    eval_conj_zero_poly(val, arg, poles, nb_poles);
    vcinv1(val);
}

PP eval_conj_pole_zero_ratfunc(t_float *val, t_float *arg, t_float *poles, t_float *zeros, int nb_poles, int nb_zeros)
{
    t_float t[2];
    eval_conj_zero_poly(t, arg, zeros, nb_zeros);
    eval_conj_pole_poly(val, arg, poles, nb_zeros);
    vcmul2(val, t);
}



/* bandlimited IIR impulse:

   * design analog butterworth filter
   * obtain the partial fraction expansion of the transfer function
   * determine the state increment as a function of fractional delay of impulse location 
   (sample the impulse response)

*/
