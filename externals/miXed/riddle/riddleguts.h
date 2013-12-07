/* Copyright (c) 2007 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* these declarations do not belong to the riddle API
   and should not be used by riddle externals */

#ifndef __RIDDLEGUTS_H__
#define __RIDDLEGUTS_H__

/* from riddle.c */

void riddle_setsourceblock(t_riddle *rd, int siginno, int newblock);
void riddle_setsourcelayout(t_riddle *rd, int siginno,
			    t_symbol *newpattern, int maxblock);
void riddle_setsourceflags(t_riddle *rd, int siginno, int flags);

t_canvas *riddle_nextgraph(t_riddle *rd);
t_canvas *riddle_firstgraph(t_riddle *rd, int outno);

t_rdfeedchain *riddle_getfeedchain(t_riddle *rd, int sigoutno);

/* from rdremote.c */

t_pd *riddle_getenvironment(void);
void riddle_updatedsp(void);

void rdfeedchain_free(t_rdfeedchain *ch);
t_rdfeedchain *rdfeedchain_new(int outno);

t_rdremote *rdremote_getwriter(t_rdremote *re);
t_rdremote *rdremote_nextreader(t_rdremote *re);
void rdremote_freeports(t_rdremote *re);

#endif
