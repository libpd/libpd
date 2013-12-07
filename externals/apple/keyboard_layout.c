/* Copyright 2006 Fredrik Olofsson
 * Copyright 2007 Free Software Foundation
 *   ported to Pd by Hans-Christoph Steiner <hans@eds.org> from f0.keyboard_layout.c
 */

#include "m_pd.h"
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

static t_class *keyboard_layout_class;

typedef struct _keyboard_layout {
	t_object    x_obj;
    t_outlet*   x_data_outlet;
    t_outlet*   x_status_outlet;
} t_keyboard_layout;

#ifdef __APPLE__

//----------------------------------------------------------------------------------------------
void keyboard_layout_bang(t_keyboard_layout *x) {
	//OSStatus err;
	KeyboardLayoutRef currentLayoutRef;
	const void *keyboardName;
	char cKeyboardName[100];
	
	KLGetCurrentKeyboardLayout(&currentLayoutRef);
	KLGetKeyboardLayoutProperty(currentLayoutRef, kKLName, (const void **)&keyboardName);
	CFStringGetCString((CFStringRef)keyboardName, cKeyboardName, 100, kCFStringEncodingASCII);
	
    outlet_symbol(x->x_data_outlet, gensym(cKeyboardName));
}

void keyboard_layout_menu(t_keyboard_layout *x) {
	//OSStatus err;
	KeyboardLayoutRef currentLayoutRef;
	const void *keyboardName;
	char cKeyboardName[100];
	CFIndex countOfLayouts;
	CFIndex i;
	t_atom name;
	
// TODO this should probably output [menu clear( so other messages work too
    outlet_anything(x->x_status_outlet, gensym("clear"), 0, NULL);
	
	KLGetKeyboardLayoutCount(&countOfLayouts);
	for(i= 0; i<countOfLayouts; i++) {
		KLGetKeyboardLayoutAtIndex(i, &currentLayoutRef);
		KLGetKeyboardLayoutProperty(currentLayoutRef, kKLName, (const void **)&keyboardName);
		CFStringGetCString((CFStringRef)keyboardName, cKeyboardName, 100, kCFStringEncodingASCII);
		
		SETSYMBOL(&name, gensym(cKeyboardName));
// TODO this should probably output [menu append( so other messages work too
        outlet_anything(x->x_status_outlet, gensym("append"), 1, &name);
	}
}

void keyboard_layout_anything(t_keyboard_layout *x, t_symbol *s, short argc, t_atom *argv) {
	//OSStatus err;
	KeyboardLayoutRef currentLayoutRef;
	const void *keyboardName;
	char cKeyboardName[100];
	
	keyboardName= CFStringCreateWithCString(NULL, s->s_name, kCFStringEncodingASCII);
	KLGetKeyboardLayoutWithName(keyboardName, &currentLayoutRef);
	KLGetKeyboardLayoutProperty(currentLayoutRef, kKLName, (const void **)&keyboardName);
	CFStringGetCString((CFStringRef)keyboardName, cKeyboardName, 100, kCFStringEncodingASCII);
	KLSetCurrentKeyboardLayout(currentLayoutRef);
	//outlet_anything(x->t_out, s, 0, NULL);
	keyboard_layout_bang(x);
}

void *keyboard_layout_new(void) {
    t_keyboard_layout *x = (t_keyboard_layout *)pd_new(keyboard_layout_class);

    x->x_data_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_status_outlet = outlet_new(&x->x_obj, &s_symbol);

    return (x);
}

//----------------------------------------------------------------------------------------------
void keyboard_layout_setup(void) {
    keyboard_layout_class = class_new(gensym("keyboard_layout"), 
                                      (t_newmethod)keyboard_layout_new, 
                                      NULL,
                                      sizeof(t_keyboard_layout),
                                      0, A_GIMME, 0);
	
	class_addbang(keyboard_layout_class, (t_method)keyboard_layout_bang);
	class_addanything(keyboard_layout_class, (t_method)keyboard_layout_anything);

	class_addmethod(keyboard_layout_class, (t_method)keyboard_layout_menu, 
                    gensym("menu"), 0);
	
	post("f0.keyboard_layout v1.1-ub; distributed under GNU GPL license");
}


#else /* GNU/Linux and Windows */


void keyboard_layout_new(void)
{
	post("f0.keyboard_layout v1.1-ub; distributed under GNU GPL license");
    post("ERROR: this objectclass is currently only for Mac OS X");
}

void keyboard_layout_setup(void)
{
    keyboard_layout_class = class_new(gensym("text"), (t_method)keyboard_layout_new, 
                                 NULL, sizeof(t_keyboard_layout), 0, 0);
}

#endif /* __APPLE__ */
