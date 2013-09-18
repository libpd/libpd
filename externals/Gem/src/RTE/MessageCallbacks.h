/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  helper-functions&macros for message callbacks to objects

  Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

// these utility macros warp the callback functions from the RTE, no need add them to your headers!

// NOTE: this is very experimental; you should not include this header unless you know what you are doing
// once this has proven to work (test on M$VC!), it is likely to be included automatically

/* usage:
 *    void myclass::obj_setupCallback(t_class*classPtr) { 
 *      CPPEXTERN_MSG (classPtr, "foo", gimmeMess); // A_GIMME
 *      CPPEXTERN_MSG0(classPtr, "doit", bangMess); // no args
 *      CPPEXTERN_MSG1(classPtr, "name", nameMess, t_symbol*);  // 1 arg (A_SYMBOL)
 *      CPPEXTERN_MSG1(classPtr, "title", titleMess, std::string); // 1 arg (A_SYMBOL)
 *      CPPEXTERN_MSG3(classPtr, "values", tripletMess, t_float, t_float, t_float);  // 3 args (A_FLOAT)
 *    }
 *    void myclass::gimmeMess  (t_symbol*s, int argc, t_atom*argv) {;}
 *    void myclass::bangMess   (void)                              {;}
 *    void myclass::nameMess   (t_symbol*s)                        {;}
 *    void myclass::titleMess  (std::string s)                     {;}
 *    void myclass::tripletMess(t_float a, t_float b, t_float c)   {;}
 */
#ifndef _INCLUDE__GEM_RTE_MESSAGECALLBACKS_H_
#define _INCLUDE__GEM_RTE_MESSAGECALLBACKS_H_

namespace gem {
  namespace RteMess {
    class NoneType {}; // just a dummy class
    template<class T=NoneType, class T1=T>
      struct TypeTemplateCore{
        static t_atomtype atomtype_id(void) { return A_NULL; }
        static T1 cast(T value) { return static_cast<T1>(value); }
        typedef T proxyType;
        virtual ~TypeTemplateCore(void) { }
      };
    template<class T>
      struct TypeTemplate : TypeTemplateCore<T, T> {
      };
    template<>
      struct TypeTemplate<t_float> : TypeTemplateCore<t_float> {
        static t_atomtype atomtype_id(void) { return A_FLOAT; }
      };
    template<>
      struct TypeTemplate<t_int> : TypeTemplateCore<t_float, t_int> {
        static t_atomtype atomtype_id(void) { return A_FLOAT; }
      };
    template<>
      struct TypeTemplate<int> : TypeTemplateCore<t_float, int> {
        static t_atomtype atomtype_id(void) { return A_FLOAT; }
      };
    template<>
      struct TypeTemplate<unsigned int> : TypeTemplateCore<t_float, unsigned int> {
        static t_atomtype atomtype_id(void) { return A_FLOAT; }
      };
    template<>
      struct TypeTemplate<bool> : TypeTemplateCore<t_float, bool> {
        static t_atomtype atomtype_id(void) { return A_FLOAT; }
        static bool cast(t_float f) {return (f>0.5); }
      };
    template<>
      struct TypeTemplate<t_symbol*> : TypeTemplateCore<t_symbol*> {
        static t_atomtype atomtype_id(void) { return A_DEFSYMBOL; }
      };
    template<>
      struct TypeTemplate<std::string> : TypeTemplateCore<t_symbol*, std::string> {
        static t_atomtype atomtype_id(void) { return A_DEFSYMBOL; }
        static std::string cast(t_symbol*s) {return std::string(s->s_name); }
      };
  }; };
#define MSG_CONCAT3(a, b, c) _##a##_##b##_##c




#define CPPEXTERN_MSG_(line, cp, selector, fun)                 \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data, t_symbol*s, int argc, t_atom*argv) { GetMyClass(data)->fun(s, argc, argv); } \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), A_GIMME, A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)
#define CPPEXTERN_MSG0_(line, cp, selector, fun)                \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data) { GetMyClass(data)->fun(); }        \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)
#define CPPEXTERN_MSG1_(line, cp, selector, fun, typ0)          \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data, gem::RteMess::TypeTemplate<typ0>::proxyType v0) \
      { GetMyClass(data)->fun(gem::RteMess::TypeTemplate<typ0>::cast(v0)); } \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), gem::RteMess::TypeTemplate<typ0>::atomtype_id(),  A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)
#define CPPEXTERN_MSG2_(line, cp, selector, fun, typ0, typ1)    \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data, gem::RteMess::TypeTemplate<typ0>::proxyType v0, gem::RteMess::TypeTemplate<typ1>::proxyType v1) \
      { GetMyClass(data)->fun(gem::RteMess::TypeTemplate<typ0>::cast(v0),  gem::RteMess::TypeTemplate<typ1>::cast(v1)); } \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), gem::RteMess::TypeTemplate<typ0>::atomtype_id(),  gem::RteMess::TypeTemplate<typ1>::atomtype_id(),  A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)

#define CPPEXTERN_MSG3_(line, cp, selector, fun, typ0, typ1, typ2) \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data, gem::RteMess::TypeTemplate<typ0>::proxyType v0, gem::RteMess::TypeTemplate<typ1>::proxyType v1, gem::RteMess::TypeTemplate<typ2>::proxyType v2) \
      { GetMyClass(data)->fun(gem::RteMess::TypeTemplate<typ0>::cast(v0),  gem::RteMess::TypeTemplate<typ1>::cast(v1),  gem::RteMess::TypeTemplate<typ2>::cast(v2)); } \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), gem::RteMess::TypeTemplate<typ0>::atomtype_id(),  gem::RteMess::TypeTemplate<typ1>::atomtype_id(),  gem::RteMess::TypeTemplate<typ2>::atomtype_id(), A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)

#define CPPEXTERN_MSG4_(line, cp, selector, fun, typ0, typ1, typ2, typ3) \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data, gem::RteMess::TypeTemplate<typ0>::proxyType v0, gem::RteMess::TypeTemplate<typ1>::proxyType v1, gem::RteMess::TypeTemplate<typ2>::proxyType v2, gem::RteMess::TypeTemplate<typ3>::proxyType v3) \
      { GetMyClass(data)->fun(gem::RteMess::TypeTemplate<typ0>::cast(v0),  gem::RteMess::TypeTemplate<typ1>::cast(v1),  gem::RteMess::TypeTemplate<typ2>::cast(v2),  gem::RteMess::TypeTemplate<typ3>::cast(v3)); } \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), gem::RteMess::TypeTemplate<typ0>::atomtype_id(),  gem::RteMess::TypeTemplate<typ1>::atomtype_id(),  gem::RteMess::TypeTemplate<typ2>::atomtype_id(),  gem::RteMess::TypeTemplate<typ3>::atomtype_id(), A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)
#define CPPEXTERN_MSG5_(line, cp, selector, fun, typ0, typ1, typ2, typ3, typ4) \
  struct MSG_CONCAT3(CallbackClass, fun, line) {                \
    static void callback(void*data, gem::RteMess::TypeTemplate<typ0>::proxyType v0, gem::RteMess::TypeTemplate<typ1>::proxyType v1, gem::RteMess::TypeTemplate<typ2>::proxyType v2, gem::RteMess::TypeTemplate<typ3>::proxyType v3, gem::RteMess::TypeTemplate<typ4>::proxyType v4) \
      { GetMyClass(data)->fun(gem::RteMess::TypeTemplate<typ0>::cast(v0),  gem::RteMess::TypeTemplate<typ1>::cast(v1),  gem::RteMess::TypeTemplate<typ2>::cast(v2),  gem::RteMess::TypeTemplate<typ3>::cast(v3),  gem::RteMess::TypeTemplate<typ4>::cast(v4)); } \
    MSG_CONCAT3(CallbackClass, fun, line) (t_class*c, std::string s) { class_addmethod(c, reinterpret_cast<t_method>(callback), gensym(s.c_str()), gem::RteMess::TypeTemplate<typ0>::atomtype_id(),  gem::RteMess::TypeTemplate<typ1>::atomtype_id(),  gem::RteMess::TypeTemplate<typ2>::atomtype_id(),  gem::RteMess::TypeTemplate<typ3>::atomtype_id(),  gem::RteMess::TypeTemplate<typ4>::atomtype_id(), A_NULL); } \
  };                                                                    \
  MSG_CONCAT3(CallbackClass, fun, line)  MSG_CONCAT3(CallbackClassInstance, fun, line) (cp, selector)


#define CPPEXTERN_MSG(cp, selector, fun)                                CPPEXTERN_MSG_  ( __LINE__ , cp, selector, fun)
#define CPPEXTERN_MSG0(cp, selector, fun)                               CPPEXTERN_MSG0_ ( __LINE__ , cp, selector, fun)
#define CPPEXTERN_MSG1(cp, selector, fun, typ)                          CPPEXTERN_MSG1_ ( __LINE__ , cp, selector, fun, typ)
#define CPPEXTERN_MSG2(cp, selector, fun, typ0, typ1)                   CPPEXTERN_MSG2_ ( __LINE__ , cp, selector, fun, typ0, typ1)
#define CPPEXTERN_MSG3(cp, selector, fun, typ0, typ1, typ2)             CPPEXTERN_MSG3_ ( __LINE__ , cp, selector, fun, typ0, typ1, typ2)
#define CPPEXTERN_MSG4(cp, selector, fun, typ0, typ1, typ2, typ3)       CPPEXTERN_MSG4_ ( __LINE__ , cp, selector, fun, typ0, typ1, typ2, typ3)
#define CPPEXTERN_MSG5(cp, selector, fun, typ0, typ1, typ2, typ3, typ4) CPPEXTERN_MSG5_ ( __LINE__ , cp, selector, fun, typ0, typ1, typ2, typ3, typ4)




#endif /* _INCLUDE__GEM_RTE_MESSAGECALLBACKS_H_ */
