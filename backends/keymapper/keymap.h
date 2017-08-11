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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_KEYMAP_H
#define COMMON_KEYMAP_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "common/config-manager.h"
#include "common/func.h"
#include "common/hashmap.h"
#include "common/hash-ptr.h"
#include "common/list.h"

namespace Common {

class Action;
class HardwareInput;
class HardwareInputSet;

class Keymap {
public:
	Keymap(const String& name) : _name(name) {}
	Keymap(const Keymap& km);
	~Keymap();

public:
	/**
	* Registers a HardwareInput to the given Action
	* @param action Action in this Keymap
	* @param key pointer to HardwareInput to map
	* @see Action::mapKey
	*/
	void registerMapping(Action *action, const HardwareInput *input);

	/**
	* Unregisters a HardwareInput from the given Action (if one is mapped)
	* @param action Action in this Keymap
	* @see Action::mapKey
	*/
	void unregisterMapping(Action *action);

	/**
	 * Find the hardware input an action is mapped to, if any
	 */
	const HardwareInput *getActionMapping(Action *action) const;

	/**
	 * Find the Action that a hardware input is mapped to
	 * @param hardwareInput	the input that is mapped to the required Action
	 * @return		a pointer to the Action or 0 if no
	 */
	Action *getMappedAction(const HardwareInput *hardwareInput) const;

	/**
	 * Get the list of all the Actions contained in this Keymap
	 */
	List<Action *>& getActions() { return _actions; }

	void setConfigDomain(ConfigManager::Domain *dom);

	/**
	 * Load this keymap's mappings from the config manager.
	 * @param hwInputs	the set to retrieve hardware input pointers from
	 */
	void loadMappings(const HardwareInputSet *hwInputs);

	/**
	 * Save this keymap's mappings to the config manager
	 * @note Changes are *not* flushed to disk, to do so call ConfMan.flushToDisk()
	 * @note Changes are *not* flushed to disk, to do so call ConfMan.flushToDisk()
	 */
	void saveMappings();

	const String& getName() { return _name; }

private:
	friend struct Action;

	/**
	 * Adds a new Action to this Map,
	 * adding it at the back of the internal array
	 * @param action the Action to add
	 */
	void addAction(Action *action);

	const Action *findAction(const char *id) const;

	typedef List<Action *> ActionList;
	typedef HashMap<const HardwareInput *, Action *> HardwareActionMap;

	String _name;
	ActionList _actions;
	HardwareActionMap _hwActionMap;
	ConfigManager::Domain *_configDomain;

};


} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_KEYMAP_H
