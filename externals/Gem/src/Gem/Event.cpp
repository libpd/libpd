////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Event.h"

#include <stdlib.h>
#include "m_pd.h"

/////////////////////////////////////////////////////////
// The callbacks
//
/////////////////////////////////////////////////////////
struct CallbackList{
    CallbackList() : data(NULL), func(NULL), next(NULL) {}
    void *data;
    void *func;
    CallbackList *next;
};

static CallbackList *s_motionList = NULL;
static CallbackList *s_buttonList = NULL;
static CallbackList *s_wheelList = NULL;
static CallbackList *s_keyboardList = NULL;
static CallbackList *s_resizeList = NULL;

/////////////////////////////////////////////////////////
// Motion callbacks
//
/////////////////////////////////////////////////////////
GEM_EXTERN void setMotionCallback(MOTION_CB callback, void *data)
{
    CallbackList *newCallback = new CallbackList;
    newCallback->data = data;
    newCallback->func = (void *)callback;

    if (!s_motionList)
        s_motionList = newCallback;
    else
    {
        CallbackList *theList = s_motionList;
        while(theList->next)
            theList = theList->next;
        theList->next = newCallback;
    }
}
GEM_EXTERN void removeMotionCallback(MOTION_CB callback, void *data)
{
    CallbackList *theList = s_motionList;
    if (!theList)
        return;
    else if (theList->func == (void *)callback &&
             theList->data == data)
    {
        s_motionList = theList->next;
        delete theList;
    }
    else
    {
        while(theList->next)
        {
            if (theList->next->func == (void *)callback &&
                theList->next->data == data)
            {
                CallbackList *holder = theList->next;
                theList->next = holder->next;
                delete holder;
                return;
            }
            theList = theList->next;
        }
    }
}
/////////////////////////////////////////////////////////
// Button callbacks
//
/////////////////////////////////////////////////////////
GEM_EXTERN void setButtonCallback(BUTTON_CB callback, void *data)
{
    CallbackList *newCallback = new CallbackList;

    newCallback->data = data;
    newCallback->func = (void *)callback;

    if (!s_buttonList)
        s_buttonList = newCallback;
    else
    {
        CallbackList *theList = s_buttonList;
        while(theList->next)
            theList = theList->next;
        theList->next = newCallback;
    }
}
GEM_EXTERN void removeButtonCallback(BUTTON_CB callback, void *data)
{
    CallbackList *theList = s_buttonList;
    if (!theList)
        return;
    else if (theList->func == (void *)callback &&
             theList->data == data)
    {
        s_buttonList = theList->next;
        delete theList;
    }
    else
    {
        while(theList->next)
        {
            if (theList->next->func == (void *)callback &&
                theList->next->data == data)
            {
                CallbackList *holder = theList->next;
                theList->next = holder->next;
                delete holder;
                return;
            }
            theList = theList->next;
        }
    }
}
/////////////////////////////////////////////////////////
// Wheel callbacks
//
/////////////////////////////////////////////////////////
GEM_EXTERN void setWheelCallback(WHEEL_CB callback, void *data)
{
    CallbackList *newCallback = new CallbackList;
    newCallback->data = data;
    newCallback->func = (void *)callback;

    if (!s_wheelList)
        s_wheelList = newCallback;
    else
    {
        CallbackList *theList = s_wheelList;
        while(theList->next)
            theList = theList->next;
        theList->next = newCallback;
    }
}
GEM_EXTERN void removeWheelCallback(WHEEL_CB callback, void *data)
{
    CallbackList *theList = s_wheelList;
    if (!theList)
        return;
    else if (theList->func == (void *)callback &&
             theList->data == data)
    {
        s_wheelList = theList->next;
        delete theList;
    }
    else
    {
        while(theList->next)
        {
            if (theList->next->func == (void *)callback &&
                theList->next->data == data)
            {
                CallbackList *holder = theList->next;
                theList->next = holder->next;
                delete holder;
                return;
            }
            theList = theList->next;
        }
    }
}
/////////////////////////////////////////////////////////
// Keyboard callbacks
//
/////////////////////////////////////////////////////////
GEM_EXTERN void setKeyboardCallback(KEYBOARD_CB callback, void *data)
{
    CallbackList *newCallback = new CallbackList;
    newCallback->data = data;
    newCallback->func = (void *)callback;

    if (!s_keyboardList)
        s_keyboardList = newCallback;
    else
    {
        CallbackList *theList = s_keyboardList;
        while(theList->next)
            theList = theList->next;
        theList->next = newCallback;
    }
}
GEM_EXTERN void removeKeyboardCallback(KEYBOARD_CB callback, void *data)
{
    CallbackList *theList = s_keyboardList;
    if (!theList)
        return;
    else if (theList->func == (void *)callback &&
             theList->data == data)
    {
        s_keyboardList = theList->next;
        delete theList;
    }
    else
    {
        while(theList->next)
        {
            if (theList->next->func == (void *)callback &&
                theList->next->data == data)
            {
                CallbackList *holder = theList->next;
                theList->next = holder->next;
                delete holder;
                return;
            }
            theList = theList->next;
        }
    }
}
/////////////////////////////////////////////////////////
// Resize callbacks
//
/////////////////////////////////////////////////////////
GEM_EXTERN void setResizeCallback(RESIZE_CB callback, void *data)
{
    CallbackList *newCallback = new CallbackList;
    newCallback->data = data;
    newCallback->func = (void *)callback;

    if (!s_resizeList)
        s_resizeList = newCallback;
    else
    {
        CallbackList *theList = s_resizeList;
        while(theList->next)
            theList = theList->next;
        theList->next = newCallback;
    }
}
GEM_EXTERN void removeResizeCallback(RESIZE_CB callback, void *data)
{
    CallbackList *theList = s_resizeList;
    if (!theList)
        return;
    else if (theList->func == (void *)callback &&
             theList->data == data)
    {
        s_resizeList = theList->next;
        delete theList;
    }
    else
    {
        while(theList->next)
        {
            if (theList->next->func == (void *)callback &&
                theList->next->data == data)
            {
                CallbackList *holder = theList->next;
                theList->next = holder->next;
                delete holder;
                return;
            }
            theList = theList->next;
        }
    }
}

/////////////////////////////////////////////////////////
// Trigger queue
//
/////////////////////////////////////////////////////////
typedef enum {
  NONE,
  MOTION,
  BUTTON,
  WHEEL,
  KEYBOARD,
  RESIZE
} gem_event_t;

typedef struct _event_queue_item {
  gem_event_t type;
  struct _event_queue_item*next;
  char*string;
  int x;
  int y;
  int state;
  int axis;
  int value;
  int which;
} gem_event_queue_item_t;

typedef struct _gem_event_queue_t{
  gem_event_queue_item_t*first;
  gem_event_queue_item_t*last;
  t_clock *clock;

}  gem_event_queue_t;

gem_event_queue_t*event_queue = NULL;

static gem_event_queue_item_t* createEvent(gem_event_t type, char*string, int x, int y, int state, int axis, int value, int which)
{
  gem_event_queue_item_t*ret=new gem_event_queue_item_t;
  ret->type=type;
  ret->next=NULL;
  ret->string=string;
  ret->x=x;
  ret->y=y;
  ret->state=state;
  ret->axis=axis;
  ret->value=value;
  ret->which=which;

  return ret;
}
static void deleteEvent( gem_event_queue_item_t* event) {
  if(event == event_queue->first) {
    event_queue->first=event->next;
  }

  if(event == event_queue->last) {
    event_queue->last=NULL;
  }

  event->type=NONE;
  event->next=NULL;
  event->string=0;
  event->x=0;
  event->y=0;
  event->state=0;
  event->axis=0;
  event->value=0;
  event->which=0;

  delete event;
}

static void eventClock(void *x);

static void addEvent(gem_event_t type, char*string, int x, int y, int state, int axis, int value, int which) {
  if (NULL==event_queue) {
    event_queue=new gem_event_queue_t;
    event_queue->first=NULL;
    event_queue->last =NULL;
    event_queue->clock=clock_new(NULL, reinterpret_cast<t_method>(eventClock));
  }
  gem_event_queue_item_t*item=createEvent(type, string, x, y, state, axis, value, which);
  if(NULL==event_queue->first) {
    event_queue->first=item;
  }
  if(event_queue->last) {
    event_queue->last->next=item;
  }
  event_queue->last=item;

  clock_delay(event_queue->clock, 0);
}

static void dequeueEvents(void) {
  CallbackList *theList=NULL;
  if (NULL==event_queue) {
    error("dequeue NULL queue");
    return;
  }
  gem_event_queue_item_t*events = event_queue->first;
  if(NULL==events) {
    error("dequeue empty queue");
    return;
  }
  while(events) {

    switch(events->type) {
    case( MOTION): 
      theList = s_motionList;
      while(theList)
        {
          MOTION_CB callback = (MOTION_CB)theList->func;
          (*callback)(events->x, events->y, theList->data);
          theList = theList->next;
        }
      break;
    case( BUTTON):
      theList = s_buttonList;
      while(theList)
        {
          BUTTON_CB callback = (BUTTON_CB)theList->func;
          (*callback)(events->which, events->state, events->x, events->y, theList->data);
          theList = theList->next;
        }
      break;
    case( WHEEL):
      theList = s_wheelList;
      while(theList)
        {
          WHEEL_CB callback = (WHEEL_CB)theList->func;
          (*callback)(events->axis, events->value, theList->data);
          theList = theList->next;
        }
      break;
    case( KEYBOARD):
      theList = s_keyboardList;
      while(theList)
        {
          KEYBOARD_CB callback = (KEYBOARD_CB)theList->func;
          (*callback)(events->string, events->value, events->state, theList->data);
          theList = theList->next;
        } 
      break;
    case( RESIZE):
      theList = s_resizeList;
      while(theList)
        {
          RESIZE_CB callback = (RESIZE_CB)theList->func;
          (*callback)(events->x, events->y, theList->data);
          theList = theList->next;
        }
      break;
	default: break;
    }

    gem_event_queue_item_t*old = events;
    events=events->next;

    deleteEvent(old);
  }
}

static void eventClock(void *x)
{
  dequeueEvents();
}

/////////////////////////////////////////////////////////
// Trigger events
//
/////////////////////////////////////////////////////////
GEM_EXTERN void triggerMotionEvent(int x, int y)
{
  addEvent(MOTION, NULL, x, y, 0, 0, 0, 0);
}
GEM_EXTERN void triggerButtonEvent(int which, int state, int x, int y)
{
  addEvent(BUTTON, NULL, x, y, state, 0, 0, which);
}
GEM_EXTERN void triggerWheelEvent(int axis, int value)
{
  addEvent(WHEEL, NULL, 0, 0, 0, axis, value, 0);
}
GEM_EXTERN void triggerKeyboardEvent(char *string, int value, int state)
{
  addEvent(KEYBOARD, gensym(string)->s_name, 0, 0, state, 0, value, 0);
}
GEM_EXTERN void triggerResizeEvent(int xSize, int ySize)
{
  addEvent(RESIZE, NULL, xSize, ySize, 0, 0, 0, 0);
}
