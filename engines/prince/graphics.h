/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_GRAPHICS_H
#define PRINCE_GRAPHICS_H

#include "graphics/surface.h"


namespace Prince {

class PrinceEngine;

class GraphicsMan
{
public:
	GraphicsMan(PrinceEngine *vm);
    ~GraphicsMan();

	void update();

	void change();

	void setPalette(const byte *palette);

	void draw(uint16 x, uint16 y, const Graphics::Surface *s);
	void drawTransparent(const Graphics::Surface *s);

	Graphics::Surface *_frontScreen;
	Graphics::Surface *_backScreen;
	const Graphics::Surface *_roomBackground;

private:

	PrinceEngine *_vm;

	bool _changed;
};

}

#endif
