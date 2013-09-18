/*
** Copyright (C) 2000 Albert L. Faber
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED

// #include "common.h"
#include "interface.h"

BOOL InitMP3(PMPSTR mp);
int	 decodeMP3(PMPSTR mp,unsigned char *inmemory,int inmemsize,char *outmemory,int outmemsize,int *done);
void ExitMP3(PMPSTR mp);

/* added remove_buf to support mpglib seeking */
void remove_buf(PMPSTR mp);

#endif
