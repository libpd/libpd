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
// HList.h

#ifndef _h_list_h__
#define _h_list_h__


#include "include/SimpleBase.h"
#include <list>

using std::list;


//---------------------------------------------------
/* this is the class of the list
 */
class HList : 
public SimpleBase< list<Element>, list<Element>::iterator >
{
 protected:
  /* this integer holds the current position of the
   * iterator, because accessing the iterator directly
   * from PD is very buggy...
   * (maybe I'll change this !)
   */
  int i_pos_;

  /* the internal iterator, you can navigate
   * through the container with it
   */
  list<Element>::iterator iter_;

 private:

  /* Copy Construction is not allowed
   */
  HList(const HList &src)
    { }

  /* assignement operator is not allowed
   */
  const HList& operator = (const HList&)
    { return *this; }

 public:

  /* Standard Constructor
   * no namespace
   */
  HList()
    { 
      dataname_ = "h_list";
      i_pos_=0;
    }

  /* Constructor
   * with a namespace
   */
  HList(string h_namespace)
    {
      dataname_ = "h_list";
      setNamespace(h_namespace);
      i_pos_=0;
    }

  /* Destructor
   */
  virtual ~HList() { }

  /* inserts an element at the end of 
   * the container
   */
  virtual void pushBack(Element value)
    {  data_[h_namespace_].push_back(value);  }

  /* removes the element from the end of 
   * the container
   */
  virtual void popBack()
    {  data_[h_namespace_].pop_back();  }

  /* inserts an element at the end of 
   * the container
   */
  virtual void pushFront(Element value)
    {  data_[h_namespace_].push_front(value);  }

  /* removes the element from the end of 
   * the container
   */
  virtual void popFront()
    {  data_[h_namespace_].pop_front();  }

  /* returns the last element
   */
  virtual Element &back() const 
    {  return data_[h_namespace_].back(); }

  /* returns the first element
   */
  virtual Element &front() const 
    {  return data_[h_namespace_].front(); }

  /* inserts an element at the current
   * iterator position
   */
  virtual void insert(Element value)
    {
      makeIterator();
      data_[h_namespace_].insert(iter_, value);
      i_pos_++;
    }
  
  /* overrides the element at the current
   * iterator position
   */
  virtual void modify(Element value)
  {
    makeIterator();
    *iter_=value;
  }
    
    
  /* gives back the element at the current
   * iterator position
   */
  virtual Element get()
    {
      makeIterator();
 
      // key was not found if iterator is pointing to the end
      if(iter_ == data_[h_namespace_].end())
	throw "h_list, get: Element not found !";

      return *iter_;
    }

  /* removes all elements with that value
   */
  virtual void remove(Element value)
    {  data_[h_namespace_].remove(value);  }

  /* removes an element at the current
   * iterator position
   */
  virtual void del()
    {
      makeIterator();

      if(data_[h_namespace_].size() == 0)
	return;

      if(iter_ ==  data_[h_namespace_].end())
	{
	  post("h_list, delete: not possible, go back by 1 (iterator points to the element after the end) !!!");
	  return;
	}

      data_[h_namespace_].erase(iter_);
    }

  /* get the size of the sequence
   */
  virtual int getSize() const
    {  return data_[h_namespace_].size();  }

  /* set current iterator position
   */
  virtual void setIterPos(int position)
  {  i_pos_ = position;  }

  /* get the current iterator position
   */
  virtual int getIterPos()
    {
      makeIterator(); 
      return i_pos_;
    }

  /* sets the iterator position the the begin
   * of the list
   */
  virtual void begin()
    {  i_pos_ = 0;  }

  /* sets the iterator position the the end
   * of the list
   */
  virtual void end()
    {  i_pos_ = data_[h_namespace_].size();  }

  /* increases the iterator position by one
   */
  virtual void next()
    {  i_pos_++; }

  /* decreases the iterator position by one
   */
  virtual void last()
    {  i_pos_--;  }

  /* removes all but the first element in every 
   * consecutive group of equal elements
   */
  virtual void unique()
    {  data_[h_namespace_].unique();  }

  /* reverses the order of elements in the list
   */
  virtual void reverse()
    {  data_[h_namespace_].reverse();  }

  /* sorts the list according to operator<
   */
  virtual void sort()
    {  data_[h_namespace_].sort();  }
  
  /* reads from file
   */
  virtual bool readFromFile(string filename);

 private:
  /* generates the current iterator position
   */
  void makeIterator()
    {
      if(i_pos_<0)
	i_pos_=0;

      if((unsigned)i_pos_ > data_[h_namespace_].size())
	i_pos_=data_[h_namespace_].size();

      // this is a hack to make the iterator, I'll change this !
      iter_=data_[h_namespace_].begin();
      for(int i = 0; i<i_pos_; i++) iter_++;
    }
};



#endif //_h_list_h__
