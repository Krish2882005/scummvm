/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "director/lingo/xtra.h"
#include "director/director.h"

namespace Director {

XtraObject::XtraObject() : Graphics::MacWidget((MacWidget *)g_director->getCurrentWindow(), 0, 0, 0, 0, g_director->_wm, false) {
}

XtraObject::XtraObject(const XtraObject &other) : Graphics::MacWidget((MacWidget *)g_director->getCurrentWindow(), 0, 0, 0, 0, g_director->_wm, false) {
}

} // End of namespace Director
