/*
 * Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
 *
 * map that stores "any"thing
 * 
 * USAGE:
 * 
 *   Properties am;                                                     // create a new Properties
 *   am["a"] = 42;                                                      // assign value "42" to key "a"
 *   int i=0;
 *   try { i=any_cast<int>am["a"]; } catch(bad_any_cast ex) { ; }       // retrieve value from key "a"; might throw a bad_any_cast exception
 *   am.get("a", i);                                                    // retrieve value at key "a"; if there was an error (e.g. typeof(i) does not match, then i is left untouched and "false" is returned
 *
 * NOTE:
 *   this simplilstic approach has some problems with type-conversion
 *   e.g. this will fail:
 *      am["a"]=12.0f; any_cast<int>am["a"];
 */

#ifndef GEM_PROPERTIES_H
#define GEM_PROPERTIES_H

#include "Utils/any.h"
#include <vector>
#include <string>

namespace gem 
{
  class Properties {
  private:
    class PIMPL;
    PIMPL*pimpl;

  public:
    enum PropertyType {
      UNSET=-1, /* not set, in-existant */
      NONE,   /* "bang" */
      DOUBLE, /* double */
      STRING, /* std::string */
      UNKNOWN /* all the rest */
    };

    Properties(void);
    Properties(const gem::Properties&); /* copy constructor */

    virtual ~Properties(void);

#if 0
    /* array/hashmap like access:
     * e.g.: prop["width"]=640;
     */
    virtual gem::any&operator[](const std::string&key);
#endif

    /* get the value of a property
     *  e.g.: double w=any_cast<double>prop.at("width")
     */
    virtual gem::any get(const std::string&key) const;

    /* check whether the given key exists
     * if the key was in the property-map, return the type of the property
     * if no key of the given value exists, return <code>PropertyType::UNSET</code>
     */
    virtual enum PropertyType type(const std::string) const;

    /* set a property
     *  e.g.: double w=640; prop.set("width", w);
     */
    virtual void set(const std::string&key, gem::any value);

    /* get a property
     *  e.g.: double w=320; prop.get("width", w);
     * NOTE: if no property of the given name exists or the existing property 
     *       is of a different (incompatible) type, "value" will not be changed
     */
    template<class Class>
      bool get(const std::string&key, Class&value) const { 
       try {
	 value=gem::any_cast<Class>(get(key));
      } catch (gem::bad_any_cast e) {
	return false;
      }
      return true;
    };

    /* get all keys
     */
    virtual std::vector<std::string>keys(void) const;

    /* 
     * delete a given key from the Properties
     */
    virtual void erase(const std::string);
    /* 
     * delete all keys from the Properties
     */
    virtual void clear(void);


    /* 
     * assign Properties from another set
     */
    virtual gem::Properties& assign(const gem::Properties&);
    gem::Properties& operator= (const gem::Properties&org) { return assign(org); }

  };
};
#endif /* GEM_PROPERTIES_H */
