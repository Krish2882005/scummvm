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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/imgui/IconsMaterialSymbols.h"
#include "director/director.h"
#include "director/debugger/dt-internal.h"

#include "director/cast.h"
#include "director/castmember/castmember.h"
#include "director/castmember/text.h"

namespace Director {
namespace DT {

const char *toIcon(CastType castType) {
	static const char *castTypes[] = {
		"",                           // Empty
		ICON_MS_BACKGROUND_DOT_LARGE, // Bitmap
		ICON_MS_THEATERS,             // FilmLoop
		ICON_MS_MATCH_CASE,           // Text
		ICON_MS_PALETTE,              // Palette
		ICON_MS_IMAGESMODE,           // Picture
		ICON_MS_VOLUME_UP,            // Sound
		ICON_MS_SLAB_SERIF,           // Button
		ICON_MS_SHAPES,               // Shape
		ICON_MS_MOVIE,                // Movie
		ICON_MS_ANIMATED_IMAGES,      // DigitalVideo
		ICON_MS_FORMS_APPS_SCRIPT,    // Script
		ICON_MS_BRAND_FAMILY,         // RTE
		"?",                          // ???
		ICON_MS_TRANSITION_FADE};     // Transition
	if (castType < 0 || castType > kCastTransition)
		return "";
	return castTypes[(int)castType];
}

const char *toString(CastType castType) {
	static const char *castTypes[] = {
		"Empty",
		"Bitmap",
		"FilmLoop",
		"Text",
		"Palette",
		"Picture",
		"Sound",
		"Button",
		"Shape",
		"Movie",
		"DigitalVideo",
		"Script",
		"RTE",
		"???",
		"Transition"};
	if (castType < 0 || castType > kCastTransition)
		return "???";
	return castTypes[(int)castType];
}

Common::String getDisplayName(CastMember *castMember) {
	const CastMemberInfo *castMemberInfo = castMember->getInfo();
	Common::String name(castMemberInfo ? castMemberInfo->name : "");
	if (!name.empty())
		return name;
	if (castMember->_type == kCastText) {
		TextCastMember *textCastMember = (TextCastMember *)castMember;
		return textCastMember->getText();
	}
	return Common::String::format("%u", castMember->getID());
}

} // namespace DT
} // namespace Director
