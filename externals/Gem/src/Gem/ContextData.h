/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  data specific to a rendering context

  Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_CONTEXTDATA_H_
#define _INCLUDE__GEM_GEM_CONTEXTDATA_H_

#include "Gem/ExportDef.h"
#include <vector>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  ContextData
    
  rendering context specific data
  this is heavily inspired by VrJuggler

  DESCRIPTION

  several things in openGL like display-lists are context dependent
  if we have multiple contexts, such values most be generated for each context
  ContextData provides a generic (templated) datatype for this

  LATER (SOONER) think about splitting the render() into a context-specific section that 
  set's up display-lists,... and a draw() function that just calls the pre-generated values
    
  -----------------------------------------------------------------*/

namespace gem {

class GEM_EXTERN ContextDataBase {
 protected:
  static const int INVALID_CONTEXT;
  virtual int getCurContext(void);
  virtual ~ContextDataBase(void);
};


template<class ContextDataType = int>
  class GEM_EXTERN ContextData : ContextDataBase
  {
    private:
    public:
   
    //////////
    // Constructor
    ContextData(void) : m_haveDefaultValue(false) {;}

    ContextData(ContextDataType v) : m_haveDefaultValue(true), m_defaultValue(v) {;}

    virtual ~ContextData() {
      m_ContextDataVector.clear();
    }
    	
    /**
     * returns the context-specific value
     *
     * @usage ContextData<GLenum>m_fun; m_fun=GL_FUNC_ADD;
     *
     * @pre We are in a draw process.
     * @note Should only be called from the draw function.
     *        Results are un-defined for other functions.
     */
    virtual operator ContextDataType()
    {
      return (*getPtrToCur());
    }

    /**
     * assigns a value to the correct context
     *
     * @pre We are in a draw process.
     * @note Should only be called from the draw function.
     *       Results are un-defined for other functions.
     */
    virtual ContextDataType&operator = (ContextDataType value)
    {
      /* simplistic approach to handle out-of-context assignments:
       *  assign the value to all context instances
       */
      if(INVALID_CONTEXT==getCurContext()) {
        doSetAll(value);
      }

      return (*getPtrToCur()=value);
    }

    private:
    bool m_haveDefaultValue;
    ContextDataType m_defaultValue;
    std::vector<ContextDataType*>  m_ContextDataVector;


    /* Makes sure that the vector is at least requiredSize large */
    void checkSize(unsigned int requiredSize)
    {
      if(requiredSize > m_ContextDataVector.size())
        {
          m_ContextDataVector.reserve(requiredSize);          // Resize smartly
          while(m_ContextDataVector.size() < requiredSize)    // Add any new items needed
            {
                if(m_haveDefaultValue) {
                    m_ContextDataVector.push_back(new ContextDataType(m_defaultValue));
                } else {
                    m_ContextDataVector.push_back(new ContextDataType);
                }
            }
        }
    }

    /**
     * Returns a pointer to the correct data element in the current context.
     *
     * @pre We are in the draw function.
     * @post Synchronized.
     * @note ASSERT: Same context is rendered by same thread each time.
     */
    ContextDataType* getPtrToCur(void)
    {
      // Get current context
      int context_id = getCurContext();
      // Cache ref for better performance
      checkSize(context_id+1);     // Make sure we are large enough (+1 since we have index)

      return m_ContextDataVector[context_id];
    }

    void doSetAll(ContextDataType v)
    {
      unsigned int i=0;
      for(i=0; i< m_ContextDataVector.size(); i++) {
        *m_ContextDataVector[i]=v;
      }
    }
  };
};



#endif	// for header file
