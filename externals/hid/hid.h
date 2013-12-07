#ifndef _HID_H
#define _HID_H

#include <stdio.h>
#include <sys/syslog.h>

#ifdef __linux__
#include <linux/types.h>
#endif /* __linux__ */

#include <m_pd.h>

/* 
 * this is automatically generated from linux/input.h by
 * make-arrays-from-input.h.pl to be the cross-platform event types and codes 
 */
#include "input_arrays.h"

#define HID_MAJOR_VERSION 0
#define HID_MINOR_VERSION 7

/* static char *version = "$Revision: 1.29 $"; */

/*------------------------------------------------------------------------------
 * GLOBAL DEFINES
 */

#define DEFAULT_DELAY 5

/* this is set to simplify data structures (arrays instead of linked lists) */
#define MAX_DEVICES 128

/* think 64 is the limit per device as defined in the OS */
#define MAX_ELEMENTS 64

/* this is limited so that the object doesn't cause a click getting too many
 * events from the OS's event queue */
#define MAX_EVENTS_PER_POLL 64

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
typedef struct _hid 
{
	t_object            x_obj;
	t_int               x_fd;
	void                *x_ff_device;
	short               x_device_number;
	short               x_instance;
	t_int               x_has_ff;
	t_int               x_started;
	t_int               x_device_open;
	t_int               x_delay;
	t_clock             *x_clock;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_hid;



/*------------------------------------------------------------------------------
 *  GLOBAL VARIABLES
 */

/* count the number of instances of this object so that certain free()
 * functions can be called only after the final instance is detroyed.
 */
EXTERN t_int hid_instance_count;

/* this is used to test for the first instance to execute */
EXTERN double last_execute_time[MAX_DEVICES];

EXTERN unsigned short global_debug_level;

/* built up when the elements of an open device are enumerated */
typedef struct _hid_element
{
#ifdef __linux__
    /* GNU/Linux store type and code to compare against */
    __u16 linux_type;
    __u16 linux_code;
#else
    void *os_pointer;  // pRecElement on Mac OS X; ... on Windows
#endif /* __linux__ */
    t_symbol *type; // Linux "type"; HID "usagePage"
    t_symbol *name; // Linux "code"; HID "usage"
    unsigned char polled; // is it polled or queued? (maybe only on Mac OS X?)
    unsigned char relative; // relative data gets output everytime
    t_int min; // from device report
    t_int max; // from device report
    t_float instance; // usage page/usage instance # ([absolute throttle 2 163( 
    t_int value; // output the sum of events in a poll for relative axes
    t_int previous_value; //only output on change on abs and buttons
} t_hid_element;

/* mostly for status querying */
EXTERN unsigned short device_count;

/* store element structs to eliminate symbol table lookups, etc. */
EXTERN t_hid_element *element[MAX_DEVICES][MAX_ELEMENTS];
/* number of active elements per device */
EXTERN unsigned short element_count[MAX_DEVICES]; 

/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR DIFFERENT PLATFORMS
 */

/* support functions */
void debug_print(t_int debug_level, const char *fmt, ...);
void debug_error(t_hid *x, t_int debug_level, const char *fmt, ...);
void hid_output_event(t_hid *x, t_hid_element *output_data);
/* the old way
void hid_output_event(t_hid *x, t_symbol *type, t_symbol *code, t_float value);
*/

/* generic, cross-platform functions implemented in a separate file for each
 * platform 
 */
t_int hid_open_device(t_hid *x, short device_number);
t_int hid_close_device(t_hid *x);
void hid_build_device_list(void);
void hid_get_events(t_hid *x);
void hid_print(t_hid* x); /* print info to the console */
void hid_platform_specific_info(t_hid* x); /* device info on the status outlet */
void hid_platform_specific_free(t_hid *x);
short get_device_number_by_id(unsigned short vendor_id, unsigned short product_id);
short get_device_number_from_usage(short device_number, 
										unsigned short usage_page, 
										unsigned short usage);


/* cross-platform force feedback functions */
t_int hid_ff_autocenter(t_hid *x, t_float value);
t_int hid_ff_gain(t_hid *x, t_float value);
t_int hid_ff_motors(t_hid *x, t_float value);
t_int hid_ff_continue(t_hid *x);
t_int hid_ff_pause(t_hid *x);
t_int hid_ff_reset(t_hid *x);
t_int hid_ff_stopall(t_hid *x);

// these are just for testing...
t_int hid_ff_fftest (t_hid *x, t_float value);
void hid_ff_print(t_hid *x);





#endif  /* #ifndef _HID_H */
