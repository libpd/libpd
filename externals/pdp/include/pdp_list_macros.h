#ifndef __PDP_LIST_MACROS__
#define __PDP_LIST_MACROS__

/* some additional (short named) list macros mainly for manipulationg
   argument lists. needs to be included locally. */

/* reading a list */
#define FIRST(l)  ((l)->first)
#define SIZE(l)   ((l)->elements)

#define NEXT(a)   ((a)->next)
#define N(a)      (a = a->next)

#define FLOAT(a)  ((a)->t == a_float ? (a)->w.w_float : 0.0f)
#define PACKET(a) ((a)->t == a_packet ? (a)->w.w_packet : -1)
#define INT(a)    ((a)->t == a_int ? (a)->w.w_packet : 0)


/* creating a list, and adding stuff to the end (queueing) */
#define LIST(n)       pdp_list_new(n)
#define LFREE(l)      pdp_list_free(l)
#define QFLOAT(l, x)  pdp_list_add_back(l, a_float, ((t_pdp_word)(float)(x)))
#define QINT(l, x)    pdp_list_add_back(l, a_int,   ((t_pdp_word)(int)(x)))
#define QPACKET(l, x) pdp_list_add_back(l, a_packet,((t_pdp_word)(int)(x)))


#endif
