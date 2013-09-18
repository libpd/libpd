
/*
 *   Pure Data Packet header file. pdp logging.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _PDP_POST_H_
#define _PDP_POST_H_

#include <stdarg.h>
#include <m_pd.h>

/* write a message to log (console) */

#include <m_pd.h>

// old
//void pdp_post_n(char *fmt, ...);
//void pdp_post(char *fmt, ...);

// new
//void startpost(const char *fmt, ...);
//void post(const char *fmt, ...);

#define pdp_post_n startpost
#define pdp_post   post

#endif
