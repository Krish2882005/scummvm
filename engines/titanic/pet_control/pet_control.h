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

#ifndef TITANIC_PET_CONTROL_H
#define TITANIC_PET_CONTROL_H

#include "titanic/core/game_object.h"
#include "titanic/core/node_item.h"
#include "titanic/core/room_item.h"
#include "titanic/messages/messages.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/pet_control/pet_conversations.h"
#include "titanic/pet_control/pet_frame.h"
#include "titanic/pet_control/pet_inventory.h"
#include "titanic/pet_control/pet_real_life.h"
#include "titanic/pet_control/pet_remote.h"
#include "titanic/pet_control/pet_rooms.h"
#include "titanic/pet_control/pet_control_sub5.h"
#include "titanic/pet_control/pet_control_sub7.h"

namespace Titanic {

enum SummonResult { SUMMON_CANT = 0, SUMMON_PRESENT = 1, SUMMON_CAN = 2 };

class CPetControl : public CGameObject {
	DECLARE_MESSAGE_MAP
	struct PetEventInfo {
		int _id;
		void *_target;
		PetEventInfo() : _id(0), _target(nullptr) {}
	};
private:
	int _fieldC0;
	int _locked;
	int _fieldC8;
	CPetSection *_sections[7];
	CPetConversations _conversations;
	CPetInventory _inventory;
	CPetRemote _remote;
	CPetRooms _rooms;
	CPetRealLife _realLife;
	CPetControlSub5 _sub5;
	CPetControlSub7 _sub7;
	CPetFrame _frame;
	CString _activeNPCName;
	CTreeItem *_treeItem2;
	CString _string2;
	CRoomItem *_hiddenRoom;
	Rect _drawBounds;
	PetEventInfo _timers[2];
private:
	/**
	 * Returns true if the control is in a valid state
	 */
	bool isValid();

	/**
	 * Loads data for the individual areas
	 */
	void loadAreas(SimpleFile *file, int param);

	/**
	 * Saves data for the individual areas
	 */
	void saveAreas(SimpleFile *file, int indent) const;

	/**
	 * Called at the end of the post game-load handling
	 */
	void loaded();

	/**
	 * Returns true if the draw bounds contains the specified point
	 */
	bool containsPt(const Common::Point &pt) const;

	bool getC0() const;

	/**
	 * Checks whether a designated NPC in present in the current view
	 */
	bool isNPCInView(const CString &name) const;

	void setTimer44(int id, int val);
protected:
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	bool MouseDragMoveMsg(CMouseDragMoveMsg *msg);
	bool MouseDragEndMsg(CMouseDragEndMsg *msg);
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	bool KeyCharMsg(CKeyCharMsg *msg);
	bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);
	bool TimerMsg(CTimerMsg *msg);
public:
	PetArea _currentArea;
	CTreeItem *_activeNPC;
public:
	CLASSDEF
	CPetControl();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Allows the item to draw itself
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	 * Gets the bounds occupied by the item
	 */
	virtual Rect getBounds();

	/**
	 * Setups the sections within the PET
	 */
	void setup();

	/**
	 * Called after loading a game has finished
	 */
	void postLoad();

	/**
	 * Called when a new node is entered
	 */
	void enterNode(CNodeItem *node);

	/**
	 * Called when a new room is entered
	 */
	void enterRoom(CRoomItem *room);

	/**
	 * Called to clear the PET display
	 */
	void clear();

	bool fn1(int val);

	void fn2(int val);

	void fn3(CTreeItem *item);

	void fn4();

	/**
	 * Sets the currently viewed area within the PET
	 */
	PetArea setArea(PetArea newSection);

	/**
	 * Returns true if the PET is currently unlocked
	 */
	bool isUnlocked() const { return _locked == 0; }

	/**
	 * Returns a game object used by the PET by name from within the
	 * special hidden room container
	 */
	CGameObject *getHiddenObject(const CString &name);

	/**
	 * Returns a reference to the special hidden room container
	 */
	CRoomItem *getHiddenRoom();

	/**
	 * Draws squares for showing glyphs inside
	 */
	void drawSquares(CScreenManager *screenManager, int count);

	/**
	 * Returns true if the point is within the PET's draw bounds
	 */
	bool contains(const Point &pt) const {
		return _drawBounds.contains(pt);
	}

	/**
	 * Handles drag ends within the PET
	 */
	CGameObject *dragEnd(const Point &pt) const {
		return _currentArea == PET_INVENTORY ? _inventory.dragEnd(pt) : nullptr;
	}

	/**
	 * Display a message
	 */
	void displayMessage(const CString &msg);

	/**
	 * Get the first game object stored in the PET
	 */
	CGameObject *getFirstObject() const;

	/**
	 * Get the next game object stored in the PET following
	 * the passed game object
	 */
	CGameObject *getNextObject(CGameObject *prior) const;

	/**
	 * Adds an item to the PET inventory
	 */
	void addToInventory(CCarry *item);

	/**
	 * Remove an item from the inventory
	 */
	void removeFromInventory(CCarry *item, CTreeItem *newParent,
		bool refreshUI = true, bool sendMsg = true);

	/**
	 * Remove an item from the inventory
	 */
	void removeFromInventory(CCarry *item, bool refreshUI = true, bool sendMsg = true);

	/**
	 * Called when the status of an item in the inventory has changed
	 */
	void invChange(CCarry *item);

	/**
	 * Moves a tree item from it's original position to be under the hidden room
	 */
	void moveToHiddenRoom(CTreeItem *item);

	void setC8(int val) { _fieldC8 = val; }

	/**
	 * Play a sound
	 */
	void playSound(int soundNum);

	/**
	 * Get the room name
	 */
	CString getRoomName() const;

	/**
	 * Check whether an NPC can be summoned
	 */
	int canSummonNPC(const CString &name);

	/**
	 * Summon an NPC to the player
	 */
	void summonNPC(const CString &name, int val);

	/**
	 * Start a timer
	 */
	void startPetTimer(uint timerIndex, uint firstDuration, uint duration, void *target);

	/**
	 * Stop a timer
	 */
	void stopPetTimer(uint timerIndex);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_H */
