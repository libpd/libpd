/* ...this is a very IEM16 external ...
   it allows for 16bit-constructs where float would eat too much memory
	
   forum::für::umläute@IEM:2003
*/

#include "iem16.h"

/* do a little help thing */

typedef struct iem16 {
  t_object x_obj;
} t_iem16;

static t_class *iem16_class;


static void *iem16_new(void){
  t_iem16 *x = (t_iem16 *)pd_new(iem16_class);
  post("iem16: 16bit objects for low memory usage");
  return(x);
}

/* include some externals */
void del16read_tilde_setup();
void del16write_tilde_setup();
void tab16play_tilde_setup();
void tab16read4_setup();
void tab16read4_tilde_setup();
void tab16read_setup();
void tab16read_tilde_setup();
void tab16receive_tilde_setup();
void tab16send_tilde_setup();
void tab16write_setup();
void tab16write_tilde_setup();
void table16_setup();
void vd16_tilde_setup();

void iem16_setup(void) {
  static unsigned int setupcount=0;
  if(setupcount>0) {
    post("iem16:\tsetup called several times, skipping...");
    return;
  }
  setupcount++;

  del16read_tilde_setup();
  del16write_tilde_setup();
  tab16play_tilde_setup();
  tab16read4_setup();
  tab16read4_tilde_setup();
  tab16read_setup();
  tab16read_tilde_setup();
  tab16receive_tilde_setup();
  tab16send_tilde_setup();
  tab16write_setup();
  tab16write_tilde_setup();
  table16_setup();
  vd16_tilde_setup();
  
  /* ************************************** */
  post("iem16:\t16bit-objects for low memory usage");
  post("iem16:\t(l) forum::für::umläute\t\tIOhannes m zmölnig");
  post("iem16:\tInstitute of Electronic Music and Acoustics, Graz - iem");
  post("iem16:\tcompiled: "__DATE__);
#if defined __WIN32__ || defined __WIN32
  post("iem16:\ton W32 you cannot create the [iem16] object. nevermind...");
#else
  iem16_class = class_new(gensym("iem16"), 
			  iem16_new, 
			  0, 
			  sizeof(t_iem16), CLASS_NOINLET, 0);

  class_addcreator((t_newmethod)iem16_new, 
		   gensym("IEM16"), 0); 
#endif
}

void IEM16_setup(void){
  iem16_setup();
}
