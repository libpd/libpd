/*
 *   Pure Data Packet module for cellular automata
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
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
 *
 */



#include "pdp_ca.h"
#include "pdp_internals.h"
#include <dlfcn.h>
#include <stdio.h>

t_class *pdp_ca_class;        // a cellular automaton processor: single input - single output
//t_class *pdp_ca2_class;     //                                 double input - single output
t_class *pdp_ca2image_class;   // converter from ca -> grey/yv12
t_class *pdp_image2ca_class;   // converter from grey/yv12 -> ca


// *********************** CA CLASS STUFF *********************


// this is defined in the makefile
// #define PDP_CA_RULES_LIB "/path/default.scafo"

#define PDP_CA_STACKSIZE 256
#define PDP_CA_MODE_1D 1
#define PDP_CA_MODE_2D 2

typedef struct pdp_ca_data_struct
{
    unsigned int env[2*4];
    unsigned int reg[2*4];
    unsigned int stack[2*PDP_CA_STACKSIZE];
    short int random_seed[4];
} t_pdp_ca_data;

typedef struct pdp_ca_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    int x_queue_id;

    /* double buffering data packets */
    int x_packet0;
    int x_packet1;

    /* some data on the ca_routine */
    void (*x_ca_routine)(void);
    void *x_ca_libhandle;
    char *x_ca_rulenames;
    int x_ca_nbrules;
    char ** x_ca_rulename;
    t_symbol *x_lastrule;
    
    /* nb of iterations */
    int x_iterations;

    /* shift ca on output */
    int x_horshift;
    int x_vershift;

    /* operation mode */
    int x_mode;
    int x_fullscreen1d;

    /* aligned vector data */
    t_pdp_ca_data *x_data;

    /* output packet type */
    t_symbol *x_packet_type;

} t_pdp_ca;


/* 1D: process from packet0 -> packet0 */
static void pdp_ca_process_ca_1D(t_pdp_ca *x)
{
    t_pdp *header = pdp_packet_header(x->x_packet0);
    unsigned int  *data   = (unsigned int *)pdp_packet_data  (x->x_packet0);

    int width  = pdp_type_ca_info(header)->width;
    int height = pdp_type_ca_info(header)->height;
    int i;

    unsigned int saved;

    /* load TOS in middle of buffer to limit the effect of stack errors */
    unsigned int *tos = &x->x_data->stack[2*(PDP_CA_STACKSIZE/2)];
    unsigned int *env = &x->x_data->env[0];
    unsigned int *reg = &x->x_data->reg[0];
    void *ca_routine = x->x_ca_routine;
    unsigned int rtos;

    /* double word width: number of unsigned ints per row  */
    int dwwidth = width >> 5;
    int currow = pdp_type_ca_info(header)->currow;

    unsigned long long result = 0;

    unsigned short temp;
    unsigned short *usdata;

    /* set destination row to 4th row from top (ca time horizon is 3 deep) */
    int dwrow0 = (((currow + height - 3) % height) * width) >> 5;
    int dwrow1 = (((currow + height - 2) % height) * width) >> 5;
    int dwrow2 = (((currow + height - 1) % height) * width) >> 5;
    int dwrow3 = (currow * width) >> 5;

    /* exit if there isn't a valid routine */
    if(!ca_routine) return;


    /* compute new row */
    for(i=0; i < (dwwidth-1) ; i+=1){
	env[0] = data[dwrow0 + i];
	env[1] = data[dwrow0 + i + 1];
	env[2] = data[dwrow1 + i];
	env[3] = data[dwrow1 + i + 1];
	env[4] = data[dwrow2 + i];
	env[5] = data[dwrow2 + i + 1];
	result = scaf_feeder(tos, reg, ca_routine, env);
	data[dwrow3 + i] = result & 0xffffffff;
    }
    // i == dwwidth-1
    
    /* compute last column in row */
    env[0] = data[dwrow0 + i];
    env[1] = data[dwrow0];
    env[2] = data[dwrow1 + i];
    env[3] = data[dwrow1];
    env[4] = data[dwrow2 + i];
    env[5] = data[dwrow2];
    result = scaf_feeder(tos, reg, ca_routine, env);
    data[dwrow3 + i] = result & 0xffffffff;


    /* undo the shift */
    usdata = (unsigned short *)(&data[dwrow3]);
    temp = usdata[(dwwidth*2)-1];
    for (i = (dwwidth*2 - 1); i > 0; i--){
	usdata[i] = usdata[i-1];
    }
    usdata[0] = temp;

    /* check data stack pointer */
    rtos = (unsigned int)tos;

    if (env[0] != rtos){
	if (env[0] > rtos) post("pdp_ca: ERROR: stack underflow detected in ca routine");
	if (env[0] < rtos) post("pdp_ca: ERROR: ca routine returned more than one item");
	x->x_ca_routine = 0;
	post("pdp_ca: rule disabled");
	
    }

    /* save current row */
    pdp_type_ca_info(header)->currow = (currow + 1) % height;

}


/* 2D: process from packet0 -> packet1 */
static void pdp_ca_process_ca_2D(t_pdp_ca *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    unsigned int  *data0   = (unsigned int *)pdp_packet_data  (x->x_packet0);
    unsigned int  *data1   = (unsigned int *)pdp_packet_data  (x->x_packet1);


    int width  = pdp_type_ca_info(header0)->width;
    int height = pdp_type_ca_info(header0)->height;
    int i,j;

    /* load TOS in middle of buffer to limit the effect of stack errors */
    unsigned int *tos = &x->x_data->stack[2*(PDP_CA_STACKSIZE/2)];
    unsigned int *env = &x->x_data->env[0];
    unsigned int *reg = &x->x_data->reg[0];
    void *ca_routine = x->x_ca_routine;
    unsigned int rtos;

    int offset = pdp_type_ca_info(header0)->offset;
    int xoffset = offset % width;
    int yoffset = offset / width;

    /* double word width: number of unsigned ints per row  */
    int dwwidth = width >> 5;

    unsigned long long result = 0;

    /* exit if there isn't a valid routine */
    if(!ca_routine) return;
    if(!header0) return;
    if(!header1) return;

    //post("pdp_ca: PRE offset: %d, xoffset: %d, yoffset: %d", offset, xoffset, yoffset);

    /* calculate new offset: lines shift up, rows shift left by 16 cells */
    xoffset = (xoffset + width - 16) % width;
    yoffset = (yoffset + height - 1) % height;

    offset =  yoffset * width + xoffset;

    //post("pdp_ca: PST offset: %d, xoffset: %d, yoffset: %d", offset, xoffset, yoffset);


    pdp_type_ca_info(header1)->offset = offset;


    for(j=0; j<dwwidth*(height - 2); j+=(dwwidth<<1)){
	for(i=0; i < (dwwidth-1) ; i+=1){
	    env[0] = data0[i + j];
	    env[1] = data0[i + j + 1];
	    env[2] = data0[i + j + dwwidth];
	    env[3] = data0[i + j + dwwidth + 1];
	    env[4] = data0[i + j + (dwwidth<<1)];
	    env[5] = data0[i + j + (dwwidth<<1) + 1];
	    env[6] = data0[i + j + (dwwidth<<1) + dwwidth];
	    env[7] = data0[i + j + (dwwidth<<1) + dwwidth + 1];
	    result = scaf_feeder(tos, reg, ca_routine, env);
	    data1[i + j] = result & 0xffffffff;
	    data1[i + j + dwwidth] = result >> 32;
	}
	// i == dwwidth-1

	env[0] = data0[i + j];
	env[1] = data0[j];
	env[2] = data0[i + j + dwwidth];
	env[3] = data0[j + dwwidth];
	env[4] = data0[i + j + (dwwidth<<1)];
	env[5] = data0[j + (dwwidth<<1)];
	env[6] = data0[i + j + (dwwidth<<1) + dwwidth];
	env[7] = data0[j + (dwwidth<<1) + dwwidth];
	result = scaf_feeder(tos, reg, ca_routine, env);
	data1[i + j] = result & 0xffffffff;
	data1[i + j + dwwidth] = result >> 32;
    }

    // j == dwwidth*(height - 2)
    for(i=0; i < (dwwidth-1) ; i+=1){
	env[0] = data0[i + j];
	env[1] = data0[i + j + 1];
	env[2] = data0[i + j + dwwidth];
	env[3] = data0[i + j + dwwidth + 1];
	env[4] = data0[i];
	env[5] = data0[i + 1];
	env[6] = data0[i + dwwidth];
	env[7] = data0[i + dwwidth + 1];
	result = scaf_feeder(tos, reg, ca_routine, env);
	data1[i + j] = result & 0xffffffff;
	data1[i + j + dwwidth] = result >> 32;
    }
    // j == dwwidth*(height - 2)
    // i == dwwidth-1
    env[0] = data0[i + j];
    env[1] = data0[j];
    env[2] = data0[i + j + dwwidth];
    env[3] = data0[j + dwwidth];
    env[4] = data0[i];
    env[5] = data0[0];
    env[6] = data0[i + dwwidth];
    env[7] = data0[dwwidth];
    result = scaf_feeder(tos, reg, ca_routine, env);
    data1[i + j] = result & 0xffffffff;
    data1[i + j + dwwidth] = result >> 32;



    /* check data stack pointer */
    rtos = (unsigned int)tos;

    if (env[0] != rtos){
	if (env[0] > rtos) post("pdp_ca: ERROR: stack underflow detected in ca routine");
	if (env[0] < rtos) post("pdp_ca: ERROR: ca routine returned more than one item");
	x->x_ca_routine = 0;
	post("pdp_ca: rule disabled");
	
    }

    return;
}


static void pdp_ca_swappackets(t_pdp_ca *x)
{
   /* swap packets */
   int packet = x->x_packet1;
   x->x_packet1 = x->x_packet0;
   x->x_packet0 = packet;
}





/* tick advance CA one timestep */
static void pdp_ca_bang_thread(t_pdp_ca *x)
{
   int encoding;
   int packet;
   int i;
   int iterations =  x->x_iterations;
  
   /* invariant: the two packets are allways valid and compatible 
      so a bang is allways possible. this means that in the pdp an 
      invalid packet needs to be converted to a valid one */

   if (-1 == x->x_packet0) pdp_post("warning: packet 0 invalid");
   if (-1 == x->x_packet1) pdp_post("warning: packet 1 invalid");

   if (PDP_CA_MODE_2D == x->x_mode){
       for(i=0; i < iterations; i++){
      
	   /* process form packet0 -> packet1 */
	   pdp_ca_process_ca_2D(x);

	   /* swap */
	   pdp_ca_swappackets(x);
       }
   }
   else if (PDP_CA_MODE_1D == x->x_mode){
       if (x->x_fullscreen1d){
	   t_pdp *header0 = pdp_packet_header(x->x_packet0);
	   pdp_type_ca_info(header0)->currow = 0;
	   pdp_type_ca_info(header0)->offset = 0;
	   iterations = pdp_type_ca_info(header0)->height;
       }
       for(i=0; i < iterations; i++){
 
	   pdp_ca_process_ca_1D(x);
       }
   }

}

static void pdp_ca_sendpacket(t_pdp_ca *x)
{

    /* adjust offset before sending */
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    int offset, width, height, xoffset, yoffset, horshift, vershift;

    if (!header0) return;

    offset = pdp_type_ca_info(header0)->offset;
    width  = pdp_type_ca_info(header0)->width;
    height = pdp_type_ca_info(header0)->height;
    xoffset = offset % width;
    yoffset = offset / width;
    horshift = x->x_horshift;
    vershift = x->x_vershift;

    horshift %= width;
    if (horshift < 0) horshift += width;
    vershift %= height;
    if (vershift < 0) vershift += height;

    xoffset = (xoffset + horshift) % width;
    yoffset = (yoffset + vershift) % height;
    offset =  yoffset * width + xoffset;

    pdp_type_ca_info(header0)->offset = offset;



    /* output the packet */
    outlet_pdp(x->x_outlet0, x->x_packet0);
}

static void pdp_ca_bang(t_pdp_ca *x)
{
    /* we don't use input packets for testing dropping here 
       but check the queue_id to see if processing is
       still going on */
    
    if (-1 == x->x_queue_id){
	pdp_queue_add(x, pdp_ca_bang_thread, pdp_ca_sendpacket, &x->x_queue_id);
    }

    else{
	pdp_control_notify_drop(-1);
    }
}


/* this method stores the packet into x->x_packet0 (the packet
   to be processed) if it is valid. x->x_packet1 is not compatible
   it is regenerated so that it is 
   
   in short, when this routine returns both packets are valid
   and compatible.
*/


static void pdp_ca_copy_rw_if_valid(t_pdp_ca *x, int packet)
{
    t_pdp *header  = pdp_packet_header(packet);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);


    int grabpacket;
    int convertedpacket;

    /* check if header is valid */
    if (!header) return;

    if (PDP_CA != header->type) return;
    if (PDP_CA_STANDARD != pdp_type_ca_info(header)->encoding) return;


    /* packet is a ca, register it */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = pdp_packet_copy_rw(packet);


    /* make sure we have the right header */
    header = pdp_packet_header(x->x_packet0);


    /* make sure that the other packet is compatible */
    if ((pdp_type_ca_info(header1)->width != pdp_type_ca_info(header)->width) ||
	 (pdp_type_ca_info(header1)->height != pdp_type_ca_info(header)->height)) {

	/* if not, throw away and clone the new one */
	pdp_packet_mark_unused(x->x_packet1);
	x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
    }

    if (-1 == x->x_packet0) pdp_post("warning: packet 0 invalid");
    if (-1 == x->x_packet1) pdp_post("warning: packet 1 invalid");


};

/* hot packet inlet */
static void pdp_ca_input_0(t_pdp_ca *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_rw")){
	pdp_ca_copy_rw_if_valid(x, (int)f);
    }
    else if (s == gensym("process")){
	pdp_ca_bang(x);
    }


}

/* cold packet inlet */
static void pdp_ca_input_1(t_pdp_ca *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_rw"))
    {
	pdp_ca_copy_rw_if_valid(x, (int)f);
    }

}


static void pdp_ca_rule_string(t_pdp_ca *x, char *c)
{
    char tmp[256];
    void (*ca_routine)(void);


    /* check if we can find string */
    sprintf(tmp, "rule_%s", c);
    if (!(ca_routine = dlsym(x->x_ca_libhandle, tmp))){
	post("pdp_ca: can't fine ca rule %s (symbol: %s)", c, tmp);
	return;
    }
    /* ok, so store routine address */
    else{
	x->x_ca_routine = ca_routine;
	x->x_lastrule = gensym(c);
    }
}


static void pdp_ca_rule(t_pdp_ca *x, t_symbol *s)
{
    /* make sure lib is loaded */
    if (!x->x_ca_libhandle) return;
    
    /* set rule by name */
    pdp_ca_rule_string(x, s->s_name);
}

static void pdp_ca_rule_index(t_pdp_ca *x, t_float f)
{
    int i = (int)f;

    /* make sure lib is loaded */
    if (!x->x_ca_libhandle) return;

    /* check index */
    if (i<0) return;
    if (i>=x->x_ca_nbrules) return;

    /* set rule by index */
    pdp_ca_rule_string(x, x->x_ca_rulename[i]);

}


static void pdp_ca_close(t_pdp_ca *x)
{
    if (x->x_ca_libhandle){
	dlclose(x->x_ca_libhandle);
	x->x_ca_libhandle = 0;
	x->x_ca_routine = 0;
	if (x->x_ca_rulename){
	    free (x->x_ca_rulename);
	    x->x_ca_rulename = 0;
	}
    

    }
}


static void pdp_ca_printrules(t_pdp_ca *x)
{
    int i;

    if (!(x->x_ca_libhandle)) return;
    post("pdp_ca: found %d rules: ", x->x_ca_nbrules);
    for(i=0;i<x->x_ca_nbrules; i++) post("%3d: %s ", i, x->x_ca_rulename[i]);


}

/* open code library */
static void pdp_ca_openlib(t_pdp_ca *x, t_symbol *s)
{

    char *c;
    int words;

    /* close current lib, if one */
    pdp_ca_close(x);

    /* try to open new lib */
    if (!(x->x_ca_libhandle = dlopen(s->s_name, RTLD_NOW))){
	post("pdp_ca: can't open ca library %s\n%s", s->s_name, dlerror());
	x->x_ca_libhandle = 0;
	return;
    }

    /* scan for valid rules */
    if (!(x->x_ca_rulenames = (char *)dlsym(x->x_ca_libhandle, "rulenames"))){
	post("pdp_ca: ERROR: %s does not contain a name table. closing.", s->s_name);
        pdp_ca_close(x);
	return;
    }

    /* count rules */
    words = 0;
    for(c = (char *)x->x_ca_rulenames; *c;){
	words++;
	while(*c++);
    }
    x->x_ca_nbrules = words;
    x->x_ca_rulename = (char **)malloc(sizeof(char *) * words);

    /* build name array */
    words = 0;
    for(c = (char *)x->x_ca_rulenames; *c;){
	x->x_ca_rulename[words] = c;
	words++;
	while(*c++);
    }

    /* ok, we're done */
    post("pdp_ca: opened rule library %s", s->s_name ,x->x_ca_nbrules);

    /* print rule names */
    //pdp_ca_printrules(x);

    /* set last selected rule */
    pdp_ca_rule(x, x->x_lastrule);
  
    
}

/* compile source file and open resulting code library */
static void pdp_ca_opensrc(t_pdp_ca *x, t_symbol *s)
{
    #define TMPSIZE 1024
    char commandline[TMPSIZE];
    char library[TMPSIZE];
    int status;

    /*  setup compiler args */
    snprintf(library, TMPSIZE, "%so", s->s_name);
    snprintf(commandline, TMPSIZE, "scafc %s %s", s->s_name, library);



    /* call compiler */
    if (system(commandline)) 
    {
	post ("pdp_ca: error compiling %s", s->s_name);
    }
    else 
    {
	post("pdp_ca: compiled %s", s->s_name);
	pdp_ca_openlib(x, gensym(library));
    }
}

/* open a source file or a library, depending on extension */
static void pdp_ca_open(t_pdp_ca *x, t_symbol *s)
{
    char *name = s->s_name;
    char *end = name;
    while(*end) end++;
    if (end == name){
	post("pdp_ca: invalid file name");
	return;
    }
    /* if the name ends with 'o' assume it is a library */
    if (end[-1] == 'o'){
	pdp_ca_openlib(x, s);
    }
    /* otherwize, assume it is a source file */
    else{
	pdp_ca_opensrc(x, s);
    }

}

/* init the current packet with random noise */
static void pdp_ca_rand(t_pdp_ca *x){

    t_pdp *header = pdp_packet_header(x->x_packet0);
    short int *data = (short int *) pdp_packet_data(x->x_packet0);
    int i;

    int nbshortints = (pdp_type_ca_info(header)->width >> 4) * pdp_type_ca_info(header)->height;

    for(i=0; i<nbshortints; i++) 
	data[i] = random();
    
}


static void pdp_ca_newca(t_pdp_ca *x, t_float width, t_float height)
{
    /* delete old packets */
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);


    /* create new packets */
    x->x_packet0 = pdp_packet_new_ca(PDP_CA, width, height);
    x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);

}


static void pdp_ca_iterations(t_pdp_ca *x, t_float f)
{
    int i = (int)f;

    if (i < 0) i = 0;

    x->x_iterations = i;
}

static void pdp_ca_horshift16(t_pdp_ca *x, t_float f)
{
    x->x_horshift = 16 * (int)f;
}

static void pdp_ca_vershift(t_pdp_ca *x, t_float f)
{
    x->x_vershift = (int)f;
}

static void pdp_ca_set1d(t_pdp_ca *x)
{
    x->x_mode = PDP_CA_MODE_1D;
}

static void pdp_ca_set2d(t_pdp_ca *x)
{
    x->x_mode = PDP_CA_MODE_2D;
}

static void pdp_ca_fullscreen1d(t_pdp_ca *x, t_floatarg f)
{
    if (f == 0.0f) x->x_fullscreen1d = 0;
    if (f == 1.0f) x->x_fullscreen1d = 1;
}

static void pdp_ca_free(t_pdp_ca *x)
{
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);
    pdp_ca_close(x);
    free(x->x_data);
}



void *pdp_ca_new(void)
{
    t_pdp_ca *x = (t_pdp_ca *)pd_new(pdp_ca_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pdp"), gensym("pdp1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("iterations"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;
    
    x->x_data = (t_pdp_ca_data *)malloc(sizeof(t_pdp_ca_data));
    x->x_ca_routine = 0;
    x->x_ca_libhandle = 0;
    x->x_ca_rulename = 0;

    x->x_horshift = 0;
    x->x_vershift = 0;

    pdp_ca_newca(x, 64, 64);
    pdp_ca_iterations(x, 1);
    pdp_ca_set2d(x);
    pdp_ca_fullscreen1d(x, 0);

    x->x_packet_type = gensym("grey");
    x->x_lastrule = gensym("gameoflife");
    pdp_ca_openlib(x, gensym(PDP_CA_RULES_LIB));

    return (void *)x;
}


// *********************** CA CONVERTER CLASSES STUFF *********************
// TODO: move this to a separate file later together with other converters (part of system?)

#define PDP_CA2IMAGE 1
#define PDP_IMAGE2CA 2

typedef struct pdp_ca_conv_struct
{
    t_object x_obj;
    t_float x_f;


    int x_threshold;

    int x_packet;

    t_outlet *x_outlet0;

    /* solve identity crisis */
    int x_whoami;

    /* output packet type */
    /* only greyscale for now */
    t_symbol *x_packet_type;

} t_pdp_ca_conv;

/* hot packet inlet */
static void pdp_ca_conv_input_0(t_pdp_ca_conv *x, t_symbol *s, t_floatarg f)
{
    int packet = -1;

    if (s == gensym("register_ro")){
	pdp_packet_mark_unused(x->x_packet);
	x->x_packet = pdp_packet_copy_ro((int)f);
	return;
    }
    else if (s == gensym("process")){
	switch(x->x_whoami){
	case PDP_CA2IMAGE:
	    packet = pdp_type_ca2grey(x->x_packet);
	    break;
	case PDP_IMAGE2CA:
	    packet = pdp_type_grey2ca(x->x_packet, x->x_threshold);
	    break;
	}
	
	/* throw away the original packet */
	pdp_packet_mark_unused(x->x_packet);
	x->x_packet = -1;


	/* pass the fresh packet */
	pdp_packet_pass_if_valid(x->x_outlet0, &packet);

	/* unregister the freshly created packet */
	//pdp_packet_mark_unused(packet);

	/* output if valid */
	//if (-1 != packet) outlet_pdp(x->x_outlet0, packet);
    }


}

void pdp_ca_conv_free(t_pdp_ca_conv *x) 
{
    pdp_packet_mark_unused(x->x_packet);
}


void pdp_image2ca_threshold(t_pdp_ca_conv *x, t_float f)
{
    f *= 0x8000;

    if (f < -0x7fff) f = -0x7fff;
    if (f > 0x7fff) f = 0x7fff;

    x->x_threshold = (short int)f;
}

void *pdp_ca2image_new(void)
{
    t_pdp_ca_conv *x = (t_pdp_ca_conv *)pd_new(pdp_ca2image_class);
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet_type = gensym("grey");
    x->x_packet = -1;
    x->x_whoami = PDP_CA2IMAGE;
    return (void *)x;
}

void *pdp_image2ca_new(void)
{
    t_pdp_ca_conv *x = (t_pdp_ca_conv *)pd_new(pdp_image2ca_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("threshold"));
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet_type = gensym("grey");
    x->x_packet = -1;
    x->x_whoami = PDP_IMAGE2CA;
    x->x_threshold = 0x4000;
    return (void *)x;
}


// *********************** CLASS SETUP FUNCTIONS *********************

#ifdef __cplusplus
extern "C"
{
#endif



void pdp_ca2image_setup(void)
{
    pdp_ca2image_class = class_new(gensym("pdp_ca2image"), (t_newmethod)pdp_ca2image_new,
    	(t_method)pdp_ca_conv_free, sizeof(t_pdp_ca), 0, A_NULL);
    class_addmethod(pdp_ca2image_class, (t_method)pdp_ca_conv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
}

void pdp_image2ca_setup(void)
{
    pdp_image2ca_class = class_new(gensym("pdp_image2ca"), (t_newmethod)pdp_image2ca_new,
    	(t_method)pdp_ca_conv_free, sizeof(t_pdp_ca), 0, A_NULL);
    class_addmethod(pdp_image2ca_class, (t_method)pdp_ca_conv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_image2ca_class, (t_method)pdp_image2ca_threshold, gensym("threshold"),  A_FLOAT, A_NULL);
}

void pdp_ca_setup(void)
{


    pdp_ca_class = class_new(gensym("pdp_ca"), (t_newmethod)pdp_ca_new,
    	(t_method)pdp_ca_free, sizeof(t_pdp_ca), 0, A_NULL);

   
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_iterations, gensym("iterations"), A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_printrules, gensym("rules"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_rand, gensym("random"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_newca, gensym("ca"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_newca, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_horshift16, gensym("hshift16"), A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_vershift, gensym("vshift"), A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_close, gensym("close"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_openlib, gensym("openlib"),  A_SYMBOL, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_opensrc, gensym("opensrc"),  A_SYMBOL, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_open, gensym("open"),  A_SYMBOL, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_rule, gensym("rule"),  A_SYMBOL, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_rule_index, gensym("ruleindex"),  A_FLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_input_1, gensym("pdp1"), A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_set1d, gensym("1D"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_set2d, gensym("2D"), A_NULL);
    class_addmethod(pdp_ca_class, (t_method)pdp_ca_fullscreen1d, gensym("fullscreen1D"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
