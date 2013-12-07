/*
 * (C) Copyright Christopher Diggins 2005
 * (C) Copyright Pablo Aguilar 2005
 * (C) Copyright Kevlin Henney 2001
 *
 * Copyright (C) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
 *       downloaded this code from http://www.codeproject.com/KB/cpp/dynamic_typing.aspx
 *         changed namespace/defines "cdiggins" to something "gem"
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef GEM_ANY_HPP
#define GEM_ANY_HPP

#include "Gem/ExportDef.h"

#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4275 )
#endif

#include <stdexcept>
#include <typeinfo>
#include <algorithm>


namespace gem
{
  struct GEM_EXTERN bad_any_cast : std::bad_cast {
    bad_any_cast(const std::type_info& src, const std::type_info& dest)
      : from(src.name()), to(dest.name())
    { }
    virtual ~bad_any_cast(void) throw() 
    { }
    virtual const std::string what(void) {
      std::string result = std::string("bad cast(");
      result+= from;
      result+= std::string("->");
      result+= to;
      result+= std::string(")");
      return result;
    }
    const std::string from;
    const std::string to;
  };

  namespace any_detail {
    // function pointer table

    struct fxn_ptr_table {
      const std::type_info& (*get_type)(void);
      void (*static_delete)(void**);
      void (*clone)(void* const*, void**);
      void (*move)(void* const*,void**);
    };

    // static functions for small value-types

    template<bool is_small>
    struct fxns
    {
      template<typename T>
      struct type {
        static const std::type_info& get_type(void) {
          return typeid(T);
        }
        static void static_delete(void** x) {
          reinterpret_cast<T*>(x)->~T();
        }
        static void clone(void* const* src, void** dest) {
          new(dest) T(*reinterpret_cast<T const*>(src));
        }
        static void move(void* const* src, void** dest) {
          reinterpret_cast<T*>(dest)->~T();
          *reinterpret_cast<T*>(dest) = *reinterpret_cast<T const*>(src);
         }
      };
    };

    // static functions for big value-types (bigger than a void*)

    template<>
    struct fxns<false>
    {
      template<typename T>
      struct type {
        static const std::type_info& get_type(void) {
          return typeid(T);
        }
        static void static_delete(void** x) {
          delete(*reinterpret_cast<T**>(x));
        }
        static void clone(void* const* src, void** dest) {
          *dest = new T(**reinterpret_cast<T* const*>(src));
        }
        static void move(void* const* src, void** dest) {
          (*reinterpret_cast<T**>(dest))->~T();
          **reinterpret_cast<T**>(dest) = **reinterpret_cast<T* const*>(src);
        }
      };
    };

    template<typename T>
    struct get_table
    {
      static const bool is_small = sizeof(T) <= sizeof(void*);

      static fxn_ptr_table* get(void)
      {
        static fxn_ptr_table static_table = {
          fxns<is_small>::template type<T>::get_type
        , fxns<is_small>::template type<T>::static_delete
        , fxns<is_small>::template type<T>::clone
        , fxns<is_small>::template type<T>::move
        };
        return &static_table;
      }
    };

    struct empty {
    };
  } // namespace any_detail


  struct GEM_EXTERN any
  {
    // structors

    template <typename T>
    any(const T& x) : table(NULL), object(NULL) {
      table = any_detail::get_table<T>::get();
      if (sizeof(T) <= sizeof(void*)) {
        new(&object) T(x);
      }
      else {
        object = new T(x);
      }
    }

    any(void) : table(NULL), object(NULL) {
      table = any_detail::get_table<any_detail::empty>::get();
      object = NULL;
    }

    any(const any& x) : table(NULL), object(NULL) {
      table = any_detail::get_table<any_detail::empty>::get();
      assign(x);
    }

    virtual ~any(void) {
      table->static_delete(&object);
    }

    // assignment

    any& assign(const any& x) {
      // are we copying between the same type?

      if (table == x.table) {
        // if so, we can avoid reallocation

        table->move(&x.object, &object);
      }
      else {
        reset();
        x.table->clone(&x.object, &object);
        table = x.table;
      }
      return *this;
    }

    template <typename T>
    any& assign(const T& x)
    {
      // are we copying between the same type?

      any_detail::fxn_ptr_table* x_table = any_detail::get_table<T>::get();
      if (table == x_table) {
        // if so, we can avoid deallocating and resuse memory

        if (sizeof(T) <= sizeof(void*)) {
          // create copy on-top of object pointer itself

          new(&object) T(x);
        }
        else {
          // create copy on-top of old version

          new(object) T(x);
        }
      }
      else {
        reset();
        if (sizeof(T) <= sizeof(void*)) {
          // create copy on-top of object pointer itself

          new(&object) T(x);
          // update table pointer

          table = x_table;
        }
        else {
          object = new T(x);
          table = x_table;
        }
      }
      return *this;
    }

    // assignment operator

    template<typename T>
    any& operator=(T const& x) {
      return assign(x);
    }
    any& operator=(const any& x) {
      return assign(x);
    }

    // utility functions

    any& swap(any& x) {
      std::swap(table, x.table);
      std::swap(object, x.object);
      return *this;
    }

    const std::type_info& get_type(void) const {
      return table->get_type();
    }

    template<typename T>
    const T& cast(void) const {
      if (get_type() != typeid(T)) {
        throw bad_any_cast(get_type(), typeid(T));
      }
      if (sizeof(T) <= sizeof(void*)) {
        return *reinterpret_cast<T const*>(&object);
      }
      else {
        return *reinterpret_cast<T const*>(object);
      }
    }

  // implicit casting is disabled by default 

  #ifdef ANY_IMPLICIT_CASTING
    // automatic casting operator

    template<typename T>
    operator T(void) const {
      return cast<T>();
    }
  #endif // implicit casting


    bool empty(void) const {
      return table == any_detail::get_table<any_detail::empty>::get();
    }

    void reset(void)
    {
      if (empty()) return;
      table->static_delete(&object);
      table = any_detail::get_table<any_detail::empty>::get();
      object = NULL;
    }

    // fields

    any_detail::fxn_ptr_table* table;
    void* object;
  };

  // boost::any-like casting

  template<typename T>
  T* any_cast(any* this_) {
    if (this_->get_type() != typeid(T)) {
      throw bad_any_cast(this_->get_type(), typeid(T));
    }
    if (sizeof(T) <= sizeof(void*)) {
      return reinterpret_cast<T*>(&this_->object);
    }
    else {
      return reinterpret_cast<T*>(this_->object);
    }
  }

  template<typename T>
  T const* any_cast(any const* this_) {
    return any_cast<T>(const_cast<any*>(this_));
  }

  template<typename T>
  T const& any_cast(any const& this_){
    return *any_cast<T>(const_cast<any*>(&this_));
  }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif

#endif // GEM_ANY_HPP
