/* OpenGL pixel buffer
 *
 * Copyright (C) 2003-2004, Alexander Zaprjagaev <frustum@frustum.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PBUFFER_H__
#define __PBUFFER_H__

struct PBuffer_data;

class PBuffer {
public:
	
	enum {
		GEM_PBUFLAG_RGB = 1 << 0,
		GEM_PBUFLAG_RGBA = 1 << 1,
		GEM_PBUFLAG_DEPTH = 1 << 2,
		GEM_PBUFLAG_STENCIL = 1 << 3,
		GEM_PBUFLAG_FLOAT = 1 << 4,
		GEM_PBUFLAG_MULTISAMPLE_2 = 1 << 5,
		GEM_PBUFLAG_MULTISAMPLE_4 = 1 << 6
	};
	
	PBuffer(int width,int height,int flags = GEM_PBUFLAG_RGBA | GEM_PBUFLAG_DEPTH | GEM_PBUFLAG_STENCIL);
	~PBuffer(void);
	
	void enable(void);
	void disable(void);
	
	int width;
	int height;

private:
	struct PBuffer_data *data;

  PBuffer(const PBuffer&);
  PBuffer&operator=(const PBuffer&);
};

#endif /* __PBUFFER_H__ */
