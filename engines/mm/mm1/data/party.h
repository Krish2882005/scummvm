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

#ifndef MM1_DATA_PARTY_H
#define MM1_DATA_PARTY_H

#include "common/array.h"
#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {

struct Party : public Common::Array<Character> {
	/**
	 * Share food, gold, gems between entire party
	 */
	static void share(TransferKind shareType);

	/**
	 * Get the party gold combined
	 */
	uint getPartyGold() const;

	/**
	 * Reset entire party's gold to zero
	 */
	void clearPartyGold();

	/**
	 * Reset entire party's gems to zero
	 */
	void clearPartyGems();

	/**
	 * Reset entire party's food to zero
	 */
	void clearPartyFood();

	/**
	 * Update the entire party AC
	 */
	void updateAC();
};

} // namespace MM1
} // namespace MM

#endif
