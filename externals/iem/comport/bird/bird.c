/*

 bird.c   - PD externals, that controls and parses one flock of bird out of a comport
         
 Date:  16.01.97          
 Author: Winfried Ritsch (see LICENCE.txt)
 
 Institute for Electronic Music - Graz

 Desc.:  put the object in a correct parse state and send commands

  first input: where data from bird is thrown in (eg.from comport)
  first output: a list of data which size is dependen on the parse mode
  second output: to control the bird eg connect to a comport in

*/


#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <stdlib.h>
#include <stdio.h>          /* general I/O */
#include <string.h>         /* for string commands */
#include "m_pd.h"
 
#define B_MAX_DATA 32    /* Maximal awaited data per record */
#define B_MAX_CMDDATA 6  /* Maximum of awaited cmd arguments */

typedef struct {

  t_object x_obj;
  t_outlet *x_out2;
  t_int x_n;
  t_atom *x_vec;

  int databytes;    /* expected databytes */
  int datacount;    /* count bytes in record */
  int phase_wait;   /* wait for phasebit */

  int datamode;     /* data mode is data or examine*/
/*  int flowmode;      stream or point mode */
  int phase_error;

  void (*writefun)(void *this,unsigned char c);

  unsigned char data[B_MAX_DATA]; /* maximal record length */
  char *argname;
  int argc;
  int argv[B_MAX_DATA];
  
  int verbose;

} bird_t;

bird_t *bird_init( bird_t *this);
int bird_data( bird_t *this, unsigned char data );
void bird_setwritefun(bird_t *this,void (*newwritefun)(void *bird,unsigned char c));
void bird_send(bird_t *this,unsigned char chr);
void bird_bang(bird_t *this);
void bird_set(bird_t *this,char *cmdname,long *cmddata);



typedef struct {

  char *name;
  unsigned char cmd;
  int cmdbytes;      /*  number of cmd arguments */
  int cmdsize;       /* size of arguments in bytes (most 2) */
  int databytes;     /*  number of awaited data */
  int datamode;     /* data mode is ignore, point flow or examine*/

} bird_cmd;


/* defines Modes for data receiving */
#define B_MODE_IGNORE 0 
#define B_MODE_POINT  1
#define B_MODE_STREAM 2 
#define B_MODE_EXAM   3

/*#define B_STREAM_ON 1
 #define B_STREAM_OFF 0
*/
#define B_WAIT_PHASE  1
#define B_FOUND_PHASE 0


/* definitions */

/* cmds accepted by the flock */
static bird_cmd cmds[]= {
 
  /* cmd , value, nr of cmdatabytes, cmddatasize, nr datainbytes 
                          data modes, if change always point */
  {"ANGLES",      'W', 0, 0,  6, B_MODE_POINT}, 
  {"MATRIX",      'X', 0, 0, 18, B_MODE_POINT},
  {"POSITION",    'V', 0, 0,  6, B_MODE_POINT},
  {"QUATER",     0x5C, 0, 0,  8, B_MODE_POINT},
  {"POSANG",      'Y', 0, 0, 12, B_MODE_POINT},
  {"POSMAT",      'Z', 0, 0, 24, B_MODE_POINT},
  {"POSQUATER",   ']', 0, 0, 14, B_MODE_POINT},
                          /* output cmd */
  {"POINT",       'B', 0, 0,  0, B_MODE_POINT},
  {"STREAM",       64, 0, 0,  0, B_MODE_STREAM},
  {"RUN",         'F', 0, 0,  0, B_MODE_IGNORE},
  {"SLEEP",       'G', 0, 0,  0, B_MODE_IGNORE},
                          /* set cmds */
  {"AngleAlign1", 'J', 6, 2,  0, B_MODE_IGNORE},
  {"AngleAlign2", 'q', 3, 2,  0, B_MODE_IGNORE},
  {"Hemisphere",  'L', 2, 1,  0, B_MODE_IGNORE},
  {"RefFrame1",   'H', 6, 2,  0, B_MODE_IGNORE},
  {"RefFrame2",   'r', 3, 2,  0, B_MODE_IGNORE},
  {"RepRate1",    'Q', 0, 0,  0, B_MODE_IGNORE},
  {"RepRate2",    'R', 0, 0,  0, B_MODE_IGNORE},
  {"RepRate8",    'S', 0, 0,  0, B_MODE_IGNORE},
  {"RepRate32",   'T', 0, 0,  0, B_MODE_IGNORE},
  { NULL,        '\0', 0, 0,  0, B_MODE_IGNORE}
};



/* -------------------- the serial object methods -------------------- */
bird_t *bird_init( bird_t *this)
{
  if(this == NULL){
	 this = malloc(sizeof(bird_t));
  }
  if(this == NULL){
	 post("Could not allocate data for bird_t");
  }

  this->databytes = 0;
  this->datacount = 0;
  this->phase_wait = B_WAIT_PHASE;
  this->datamode = B_MODE_IGNORE;
  this->phase_error = 0;
  this->writefun = NULL;

  this->argname = "STARTUP_MODE";
  this->argc = 0;


  return this;
}

int bird_data( bird_t *this, unsigned char data )
{
  int i;

  if(this->datamode !=  B_MODE_IGNORE){

    /* STREAM or POINT Mode */

    /* Phase was detected */
    if(this->phase_wait == B_FOUND_PHASE && data < 0x80){ 

      this->data[this->datacount] = data; /* store data */
      this->datacount++;                  /* increment counter */
      
      if(this->databytes <= this->datacount){ /* last byte of record */
	this->datacount = 0;
	this->phase_wait = B_WAIT_PHASE;

	/* interpret and output */
	this->argc = this->databytes / 2;
	for(i=0;i<this->databytes;i+=2){

	  this->argv[i/2] = (this->data[i]<<2)+(this->data[i+1]<<9);

	  /*			 printf("list[%2d]=%7d (%3d,%3d) ",i,
				 ((this->data[i]<<2)+(this->data[i+1]<<9)),
				 this->data[i],this->data[i+1]);
	  */
	}
	//		  printf("\n");
	return this->argc;
      };
    }
    else{ /* Phase wait */
      
      if( (data & 0x80) == 0x00 ){ /* phase bit not found */ 
	if(this->phase_error == 0)
	  if(this->verbose)post("phase error:%x",data);
	this->phase_error++;
      }
      else{
	this->phase_wait = B_FOUND_PHASE; /* phase found */
	this->data[0] = data & 0x7F;      /* store first data */
	this->datacount = 1;              /* wait for next */
	this->phase_error = 0;            /* phase error reset */
      };
      
    };
  }; /* stream or point mode */
  return 0;
}


void bird_setwritefun(bird_t *bird,void (*newwritefun)(void *this,unsigned char c))
{
  //if(bird == NULL) return; better segfault and you find the error...
  bird->writefun = newwritefun;
}

void bird_send(bird_t *this,unsigned char chr)
{
  //  if(this == NULL)return; better segfault and you find the error...
  if(this->writefun)this->writefun(this,chr);
}

/* with bang to trigger a data output (POINT) */

void bird_bang(bird_t *this)
{
    if(this->datamode == B_MODE_POINT)
		bird_send(this,'B');
}

/* set the modes for the bird */
void bird_set(bird_t *this,char *cmdname,long *cmddata)
{
  int i,j;
  long data;
  bird_cmd *cmd = cmds;

  /* search for cmd */
  while(cmd->name != (char *) 0l && strcmp(cmd->name,cmdname) != 0)cmd++;

  if(cmd->name == (char *) 0l){
	 post("bird:Dont know how to set %s",cmdname);
	 return;
  }

  /* CMD found */
  if(cmd->databytes > 0){  /* if databytes awaited, else dont change */

	 this->databytes = cmd->databytes; /* expected databytes per record */
	 this->datacount = 0;              /* start with first */
	 this->argname = cmd->name;

	 if( cmd->datamode == B_MODE_EXAM)
		this->phase_wait = B_FOUND_PHASE;  /* wait for phase-bit */
	 else
		this->phase_wait = B_WAIT_PHASE;  /* wait for phase-bit */
  }

  if( cmd->datamode != B_MODE_IGNORE) /* go into data mode */
	 this->datamode = cmd->datamode;


  if(cmd->cmdbytes >= 0){        /* is a real cmd for bird */

	 bird_send(this,cmd->cmd);

	 for(i=0; i < cmd->cmdbytes;i++){ 

		data = cmddata[i];
		  
		for(j=0; j < cmd->cmdsize;j++){      /* send it bytewise */
		  bird_send(this, (unsigned char) (data&0xFF));
		  data >>= 8;
		};
	 };
	 
  }

  if(this->verbose)post("command %s (%c): databytes=%d, mode=%d, phase=%d",
			cmd->name,cmd->cmd,
			this->databytes,
			this->datamode, this->phase_wait);

}


/* ---------------- pd object bird ----------------- */

/* code for bird pd class */


void bird_float(bird_t *x, t_floatarg f)
{
  int n,i;

  if((n=bird_data(x,(unsigned char) f)) > 0){
		  
    /* make list and output */
  			 
    for(i=0; i < x->argc ; i++){
      x->x_vec[i].a_type = A_FLOAT;
      x->x_vec[i].a_w.w_float  =  x->argv[i];
    }
    outlet_list(x->x_obj.ob_outlet, &s_list, x->argc, x->x_vec);
  }
}

void bird_setting(bird_t *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  char *cmdnam;
  long buffer[ B_MAX_CMDDATA ];

  if(argc < 1) return;
  cmdnam = argv[0].a_w.w_symbol->s_name;

  if(argc > (B_MAX_CMDDATA +1))
    argc = B_MAX_CMDDATA +1;

  for(i=1;i< argc;i++)
    if(argv[i].a_type != A_FLOAT)
      post("bird set arg %d no float",i);
    else
      buffer[i-1] = argv[i].a_w.w_float;

  bird_set(x,cmdnam,buffer);
}

void bird_verbose(bird_t *x, t_floatarg f)
{
  if(f) x->verbose = 1;
  else  x->verbose = 0;
}

void bird_free(bird_t *x)
{
  freebytes(x->x_vec, x->x_n * sizeof(*x->x_vec));
}

t_class *bird_class;

void bird_output(void *x,unsigned char c)
{
  outlet_float(((bird_t *)x)->x_out2, (t_float) c);
}

void *bird_new(void)
{
  bird_t *x;

  x = (bird_t *)pd_new(bird_class);
  
  outlet_new(&x->x_obj, &s_list);
  x->x_out2 = outlet_new(&x->x_obj, &s_float);

  
  x->x_vec = (t_atom *)getbytes((x->x_n=B_MAX_DATA)  * sizeof(*x->x_vec));

  bird_init(x);
  bird_setwritefun(x,bird_output);
  
  bird_set(x,"RUN",NULL);
  
  bird_set(x,"POSANG",NULL);
  //  out_byte('W');
  
  bird_set(x,"POINT",NULL);
  //  out_byte(64);
  
  x->verbose = 0;
 
  return (void *)x;
}

void bird_setup(void)
{
    bird_class = class_new(gensym("bird"), (t_newmethod)bird_new,
    	(t_method)bird_free, sizeof(bird_t), 0, 0);

    /* maximum commandatasize is 6*/
    class_addmethod(bird_class, (t_method)bird_setting, gensym("set"), A_GIMME, 0); 
    class_addmethod(bird_class, (t_method)bird_verbose, gensym("verbose"), A_FLOAT, 0); 

    class_addbang(bird_class,bird_bang);

    class_addfloat(bird_class, bird_float);
}

