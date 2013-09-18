//altivec version by Chris Clepper
//
static t_int *partconv_perform(t_int *w)
{
    t_partconv *x = (t_partconv *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    int i;
    int j;
    int k;	// bin
    int p;	// partition
    int endpart;
    fftwf_complex *cursumbuf_fd;
    float *sumbuf1ptr;
    float *sumbuf2ptr;

    union {
        unsigned char c[16];
        vector unsigned char v;
    }permfill;

    union {
        float f[4];
        vector float v;
    }floatfill;

    vector float *load_input, *load_irpart;
    vector float store_multbuf1,store_multbuf2;
    vector float vinput_fd0, vinput_fd4; //input vectors
    vector float virpart_fd0, virpart_fd4;  //ir partition vectors
    vector float permtemp1357, permtemp0246;
    vector float vzero;// vscale;
    vector unsigned char input_0022, input_1133, perm_0246, perm_1357, perm_0123,perm_4567;
    vector float vtemp1, vtemp2, vtemp3, vtemp4, vtemp5, vtemp6, vtemp7, vtemp8;

    floatfill.f[0] = 0.f;
    floatfill.f[1] = 0.f;
    floatfill.f[2] = 0.f;
    floatfill.f[3] = 0.f;
    vzero = floatfill.v;

    //store_multbuf = vzero;

    floatfill.f[0] = x->scale;
    floatfill.f[1] = x->scale;
    floatfill.f[2] = x->scale;
    floatfill.f[3] = x->scale;
    //vscale = floatfill.v;

    //fill the permute buffer for the first input_fd multiply
    permfill.c[0] = 0; permfill.c[1] = 1; permfill.c[2] = 2; permfill.c[3] = 3; //first float
    permfill.c[4] = 0; permfill.c[5] = 1; permfill.c[6] = 2; permfill.c[7] = 3; //second float
    permfill.c[8] = 8; permfill.c[9] = 9; permfill.c[10] = 10; permfill.c[11] = 11; //third float
    permfill.c[12] = 8; permfill.c[13] = 9; permfill.c[14] = 10; permfill.c[15] = 11; //fourth float

    input_0022 = permfill.v;

    permfill.c[0] = 4; permfill.c[1] = 5; permfill.c[2] = 6; permfill.c[3] = 7; //first float
    permfill.c[4] = 4; permfill.c[5] = 5; permfill.c[6] = 6; permfill.c[7] = 7; //second float
    permfill.c[8] = 12; permfill.c[9] = 13; permfill.c[10] = 14; permfill.c[11] = 15; //third float
    permfill.c[12] = 12; permfill.c[13] = 13; permfill.c[14] = 14; permfill.c[15] = 15; //fourth float

    input_1133 = permfill.v;

    //perm_0246
    //0,1,2,3,        8,9,10,11,          16,17,18,19,          24,25,26,27
    permfill.c[0] = 0; permfill.c[1] = 1; permfill.c[2] = 2; permfill.c[3] = 3; //first float
    permfill.c[4] = 8; permfill.c[5] = 9; permfill.c[6] = 10; permfill.c[7] = 11; //second float
    permfill.c[8] = 16; permfill.c[9] = 17; permfill.c[10] = 18; permfill.c[11] = 19; //third float
    permfill.c[12] = 24; permfill.c[13] = 25; permfill.c[14] = 26; permfill.c[15] = 27; //fourth float

    perm_0246 = permfill.v;

    // perm_1357
    //         4,5,6,7,         12,13,14,15,           20,21,22,23,         28,29,30,31
    permfill.c[0] = 4; permfill.c[1] = 5; permfill.c[2] = 6; permfill.c[3] = 7; //first float
    permfill.c[4] = 12; permfill.c[5] = 13; permfill.c[6] = 14; permfill.c[7] = 15; //second float
    permfill.c[8] = 20; permfill.c[9] = 21; permfill.c[10] = 22; permfill.c[11] = 23; //third float
    permfill.c[12] = 28; permfill.c[13] = 29; permfill.c[14] = 30; permfill.c[15] = 31; //fourth float

    perm_1357 = permfill.v;

    // perm_0123  from [0,2,4,6] and [1,3,5,7]
    //         0,1,2,3	16,17,18,19	4,5,6,7	20,21,22,23
    permfill.c[0] = 0; permfill.c[1] = 1; permfill.c[2] = 2; permfill.c[3] = 3; //first float
    permfill.c[4] = 16; permfill.c[5] = 17; permfill.c[6] = 18; permfill.c[7] = 19; //second float
    permfill.c[8] = 4; permfill.c[9] = 5; permfill.c[10] = 6; permfill.c[11] = 7; //third float
    permfill.c[12] = 20; permfill.c[13] = 21; permfill.c[14] = 22; permfill.c[15] = 23; //fourth float

    perm_0123 = permfill.v;

    // perm_4567  from [0,2,4,6] and [1,3,5,7]
    //        8.9.10.11      24,25,26,27              12,13,14,15         28,29,30,31
    permfill.c[0] = 8; permfill.c[1] = 9; permfill.c[2] = 10; permfill.c[3] = 11; //first float
    permfill.c[4] = 24; permfill.c[5] = 25; permfill.c[6] = 26; permfill.c[7] = 27; //second float
    permfill.c[8] = 12; permfill.c[9] = 13; permfill.c[10] = 14; permfill.c[11] = 15; //third float
    permfill.c[12] = 28; permfill.c[13] = 29; permfill.c[14] = 30; permfill.c[15] = 31; //fourth float

    // perm_4567  from [0,2,4,6] and [1,3,5,7]
    //        8.9.10.11      24,25,26,27              12,13,14,15         28,29,30,31
    permfill.c[0] = 8; permfill.c[1] = 9; permfill.c[2] = 10; permfill.c[3] = 11; //first float
    permfill.c[4] = 24; permfill.c[5] = 25; permfill.c[6] = 26; permfill.c[7] = 27; //second float
    permfill.c[8] = 12; permfill.c[9] = 13; permfill.c[10] = 14; permfill.c[11] = 15; //third float
    permfill.c[12] = 28; permfill.c[13] = 29; permfill.c[14] = 30; permfill.c[15] = 31; //fourth float

    perm_4567 = permfill.v;
    

    memcpy(&(x->inbuf[x->inbufpos]), in, n*sizeof(float));  // gather a block of input into input buffer
    x->inbufpos += n;
    if (x->inbufpos >= x->partsize) {
        // input buffer is full, so we begin a new cycle

        if (x->pd_blocksize != n) {
            // the patch's blocksize has change since we last dealt the work
            x->pd_blocksize = n;
            partconv_deal_work(x);
        }

        x->inbufpos = 0;
        x->curcall = 0;
        x->curpart = 0;
        memcpy(x->input_td, x->inbuf, x->partsize * sizeof(float));  // copy 'gathering' input buffer into 'transform' buffer
        memset(&(x->input_td[x->partsize]), 0, (x->paddedsize - x->partsize) * sizeof(float));  // pad

        fftwf_execute(x->input_plan);  // transform the input

        // everything has been read out of prev sumbuf, so clear it
        memset(x->sumbuf->prev->td, 0,  x->paddedsize * sizeof(float));

        // advance sumbuf pointers
        x->sumbuf = x->sumbuf->next;
        x->sumbuf->readpos = 0;
        x->sumbuf->prev->readpos = x->partsize;
    }

    // convolve this call's portion of partitions
    endpart = x->curpart + x->parts_per_call[x->curcall];
    if (endpart > x->nparts)  // FIXME does this ever happen?
        endpart = x->nparts;
    for (p = x->curpart; p < endpart; p++) {
        //printf("convolving with partition %d\n", p);
        //
        // multiply the input block by the partition, accumulating the result in the appropriate sumbuf
        //

        // FIXME do this in a circular list-type fashion so we don't need "index"
        cursumbuf_fd =  x->sumbufs[(x->sumbuf->index + p) % x->nsumbufs].fd;

        for (k = 0; k < x->nbins; k+=4) {


            
            
            load_input = (vector float *)&x->input_fd[k][0];
            vinput_fd0 = vec_ld(0, (vector float *) load_input);

            vtemp1 = vec_perm(load_input[0],vzero,input_0022);

            load_input = (vector float *)&x->input_fd[k][4];
            //load input_fd[k][4]
            //vector will have input_fd[4,5,6,7]
            vinput_fd4 = vec_ld(0, (vector float *) &x->input_fd[k][4]);

            vtemp3 = vec_perm(load_input[0],vzero,input_0022);

            //vec_ld irpart[p][k][0]
            //vector will have irpart_fd[0,1,2,3]

            load_irpart = (vector float *) &x->irpart_fd[p][k][0];

            virpart_fd0 = vec_ld(0,&x->irpart_fd[p][k][0]);
            vtemp1 = vec_madd(vtemp1,load_irpart[0],vzero);

            load_irpart = (vector float *) &x->irpart_fd[p][k][4];
            virpart_fd4 = vec_ld(0,&x->irpart_fd[p][k][4]);
            vtemp3 = vec_madd(vtemp3,load_irpart[0],vzero);


            store_multbuf1 = vec_ld(0,&cursumbuf_fd[k][0]);

            store_multbuf2 = vec_ld(0,&cursumbuf_fd[k][4]);


            //vec_perm to line up the elements
            // irpart is fine
            // make vector of input_fd[0] [2] and [4] [6]
            //make vector of input_fd[1] [3] and [5] [7]
            //
            // permute only works on bytes so the first float is bytes 0,1,2,3 the second is 4,5,6,7 etc
            //
            // 0,1,2,3,        8,9,10,11,          16,17,18,19,          24,25,26,27
            //
            //         4,5,6,7,         12,13,14,15,           20,21,22,23,         28,29,30,31


            //vec_perm temp1 and temp3 into [0,2,4,6]
            permtemp0246 = vec_perm(vtemp1,vtemp3,perm_0246);

            //and [1,3,5,7]
            permtemp1357 = vec_perm(vtemp1,vtemp3,perm_1357);

            //vinput_fd[1,3,5,7]
            vtemp2 = vec_perm(vinput_fd0,vinput_fd4,perm_1357);

            //irpart[1,3,5,7]
            vtemp4 = vec_perm(virpart_fd0,virpart_fd4,perm_1357);

            //irpart[0,2,4,6]
            vtemp5 = vec_perm(virpart_fd0,virpart_fd4,perm_0246);

            //vec_nmsub  input_fd[1,3,5,7]  irpart[1,3,5,7] temp[0,2,4,6]
            vtemp6 = vec_nmsub(vtemp2,vtemp4,permtemp0246);

            //vec_madd  input_fd[1,3,5,7] irpart[0,2,4,6] temp[1,3,5,7]
            vtemp7 = vec_madd(vtemp2,vtemp5,permtemp1357);


            

            //vec_madd  all by scale - this is now done after the loop
          //  vtemp6 = vec_madd(vtemp6,vscale,vzero);

           // vtemp7 = vec_madd(vtemp7,vscale,vzero);


            //vec_perm data back into place - tricky!

            //vec_perm nmsub_result[0,2,4,6] madd_result [1,3,5,7]
            // results will be [0,1,2,3] [4,5,6,7]
            vtemp1 = vec_perm(vtemp6,vtemp7,perm_0123);

            vtemp2 = vec_perm(vtemp6,vtemp7,perm_4567);


            //vec_st
            
            store_multbuf1 = vec_add(store_multbuf1,vtemp1);
            store_multbuf2 = vec_add(store_multbuf2,vtemp2);

            vec_st(store_multbuf1,0,&cursumbuf_fd[k][0]);
            
            vec_st(store_multbuf2,0,&cursumbuf_fd[k][4]);
            
            
            /*
            cursumbuf_fd[k][0]
            +=
            (  x->input_fd[k][0] * x->irpart_fd[p][k][0]
               - x->input_fd[k][1] * x->irpart_fd[p][k][1]);

            cursumbuf_fd[k][1]
                +=
                (  x->input_fd[k][0] * x->irpart_fd[p][k][1]
                   + x->input_fd[k][1] * x->irpart_fd[p][k][0]);*/
        }
    }
    x->curpart = p;

    // The convolution of the fresh block of input with the first partition of the IR
    // is the last thing that gets summed into the current sumbuf before it gets IFFTed and starts being output.
    // This happens during the first call of every cycle.
    if (x->curcall == 0) {
        // current sumbuf has been filled, so transform it (TD to FD).
        // Output loop will begin to read it and sum it with the last one
        fftwf_execute(x->sumbuf->plan);
    }

    // we're summing and outputting the first half of the most recently IFFTed sumbuf
    // and the second half of the previous one
    sumbuf1ptr = &(x->sumbuf->td[x->sumbuf->readpos]);
    sumbuf2ptr = &(x->sumbuf->prev->td[x->sumbuf->prev->readpos]);
    for (i = 0; i < n; i++) {
        *(out++) = (*(sumbuf1ptr++) + *(sumbuf2ptr++)) * x->scale;
    }
    x->sumbuf->readpos += n;
    x->sumbuf->prev->readpos += n;

    x->curcall++;

    return (w+5);
}
