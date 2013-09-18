// *********************(c)*2004*********************>
// -holzilib--holzilib--holzilib--holzilib--holzilib->
// ++++PD-External++by+Georg+Holzmann++grh@gmx.at++++>
//
// PDContainer: 
// this is a port of the containers from the C++ STL
// (Standard Template Library)
// for usage see the documentation and PD help files
// for license see readme.txt
//
// PDContainer.cpp 


#include "include/GlobalStuff.h"


typedef struct PDContainer 
{
  t_object x_obj;
} t_PDContainer;

t_class *PDContainer_class;

static void PDContainer_help(void)
{
  post("\nPD-Container, Version: "PDC_VERSION"");
  post("------------------------------------------");
  post("this is an implementation of the container");
  post("objects from the Standard Template");
  post("Library (STL) of C++");
  post("for documentation see the help patches");
  post("(by Georg Holzmann <grh@mur.at>, 2004)");
  post("------------------------------------------\n");
}

static void *PDContainer_new(void)
{
  t_PDContainer *x = (t_PDContainer *)pd_new(PDContainer_class);
  return (void *)x;
}

//-----------------------------------------------------
// declaration of the setup functions:
void h_map_setup();
void h_multimap_setup();
void h_set_setup();
void h_multiset_setup();
void h_vector_setup();
void h_deque_setup();
void h_queue_setup();
void h_prioqueue_setup();
void h_stack_setup();
void h_list_setup();
//-end-of-declaration----------------------------------

extern "C" void PDContainer_setup(void) 
{
  //---------------------------------------------------
  // call all the setup functions:
  h_map_setup();
  h_multimap_setup();
  h_set_setup();
  h_multiset_setup();
  h_vector_setup();
  h_deque_setup();
  h_queue_setup();
  h_prioqueue_setup();
  h_stack_setup();
  h_list_setup();
  //-end-----------------------------------------------

  post("\nPD-Container, Version: "PDC_VERSION", by Georg Holzmann <grh@mur.at>, 2004-2005");
  
  // without an argument the following two methods wont work ??? why?? because of c++?  
  PDContainer_class = class_new(gensym("PDContainer"), (t_newmethod)PDContainer_new,
                                       0, sizeof(t_PDContainer), CLASS_DEFAULT, A_DEFFLOAT,  0);
  class_addmethod(PDContainer_class, (t_method)PDContainer_help, gensym("help"), A_DEFFLOAT, 0);
}
