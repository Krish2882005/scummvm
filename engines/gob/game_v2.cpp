/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/video.h"
#include "gob/dataio.h"
#include "gob/pack.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/parse.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/util.h"
#include "gob/goblin.h"
#include "gob/cdrom.h"
#include "gob/music.h"

namespace Gob {

Game_v2::Game_v2(GobEngine *vm) : Game_v1(vm) {
}

void Game_v2::playTot(int16 skipPlay) {
	char savedTotName[20];
	int16 *oldCaptureCounter;
	int16 *oldBreakFrom;
	int16 *oldNestLevel;
	int16 _captureCounter;
	int16 breakFrom;
	int16 nestLevel;
	char needTextFree;
	char needFreeResTable;
	char *curPtr;
	int32 variablesCount;
	char *filePtr;
	char *savedIP;
	int16 i;

	oldNestLevel = _vm->_inter->_nestLevel;
	oldBreakFrom = _vm->_inter->_breakFromLevel;
	oldCaptureCounter = _vm->_scenery->_pCaptureCounter;
	savedIP = _vm->_global->_inter_execPtr;

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->_pCaptureCounter = &_captureCounter;
	strcpy(savedTotName, _curTotFile);

	if (skipPlay == 0) {
		while (1) {
			for (i = 0; i < 4; i++) {
				_vm->_draw->_fontToSprite[i].sprite = -1;
				_vm->_draw->_fontToSprite[i].base = -1;
				_vm->_draw->_fontToSprite[i].width = -1;
				_vm->_draw->_fontToSprite[i].height = -1;
			}

			if(_vm->_features & GF_MAC)
				_vm->_music->stopPlay();
			else
				_vm->_cdrom->stopPlaying();
			_vm->_draw->animateCursor(4);
			_vm->_inter->initControlVars();
			_vm->_mult->initAll();
			_vm->_mult->zeroMultData();

			for (i = 0; i < 20; i++)
				_vm->_draw->_spritesArray[i] = 0;

			_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
			_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;
			_vm->_draw->_spritesArray[23] = _vm->_draw->_cursorSprites;

			for (i = 0; i < 20; i++)
				_soundSamples[i] = 0;

			_totTextData = 0;
			_totResourceTable = 0;
			_imFileData = 0;
			_extTable = 0;
			_extHandle = -1;

			needFreeResTable = 1;
			needTextFree = 1;

			_totToLoad[0] = 0;

			if (_curTotFile[0] == 0 && _totFileData == 0)
				break;

			loadTotFile(_curTotFile);
			if (_totFileData == 0) {
				_vm->_draw->blitCursor();
				break;
			}

			strcpy(_curImaFile, _curTotFile);
			strcpy(_curExtFile, _curTotFile);

			_curImaFile[strlen(_curImaFile) - 4] = 0;
			strcat(_curImaFile, ".ima");

			_curExtFile[strlen(_curExtFile) - 4] = 0;
			strcat(_curExtFile, ".ext");

			debugC(4, DEBUG_FILEIO, "IMA: %s", _curImaFile);
			debugC(4, DEBUG_FILEIO, "EXT: %s", _curExtFile);

			filePtr = (char *)_totFileData + 0x30;

			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = _totFileData;
				if (READ_LE_UINT32(filePtr) == 0)
					_totTextData = (TotTextTable *) loadLocTexts();
				else
					_totTextData =
							(TotTextTable *) (curPtr +
							READ_LE_UINT32((char *)_totFileData + 0x30));

				if (_totTextData != 0) {
					_totTextData->itemsCount = (int16)READ_LE_UINT16(&_totTextData->itemsCount);

					for (i = 0; i < _totTextData->itemsCount; ++i) {
						_totTextData->items[i].offset = (int16)READ_LE_UINT16(&_totTextData->items[i].offset);
						_totTextData->items[i].size = (int16)READ_LE_UINT16(&_totTextData->items[i].size);
					}
				}

				needTextFree = 0;
			}

			filePtr = (char *)_totFileData + 0x34;
			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = _totFileData;

				_totResourceTable =
					(TotResTable *)(curPtr +
				    READ_LE_UINT32((char *)_totFileData + 0x34));

				_totResourceTable->itemsCount = (int16)READ_LE_UINT16(&_totResourceTable->itemsCount);

				for (i = 0; i < _totResourceTable->itemsCount; ++i) {
					_totResourceTable->items[i].offset = (int32)READ_LE_UINT32(&_totResourceTable->items[i].offset);
					_totResourceTable->items[i].size = (int16)READ_LE_UINT16(&_totResourceTable->items[i].size);
					_totResourceTable->items[i].width = (int16)READ_LE_UINT16(&_totResourceTable->items[i].width);
					_totResourceTable->items[i].height = (int16)READ_LE_UINT16(&_totResourceTable->items[i].height);
				}

				needFreeResTable = 0;
			}

			loadImFile();
			loadExtTable();

			_vm->_global->_inter_animDataSize = READ_LE_UINT16((char *)_totFileData + 0x38);
			if (_vm->_global->_inter_variables == 0) {
				variablesCount = READ_LE_UINT32((char *)_totFileData + 0x2c);
				_vm->_global->_inter_variables = new char[variablesCount * 4];
				for (i = 0; i < variablesCount; i++)
					WRITE_VAR(i, 0);
			}

			_vm->_global->_inter_execPtr = (char *)_totFileData;
			_vm->_global->_inter_execPtr += READ_LE_UINT32((char *)_totFileData + 0x64);

			_vm->_inter->renewTimeInVars();

			WRITE_VAR(13, _vm->_global->_useMouse);
			WRITE_VAR(14, _vm->_global->_soundFlags);
			WRITE_VAR(15, _vm->_global->_videoMode);
			WRITE_VAR(16, _vm->_global->_language);

			_vm->_inter->callSub(2);

			if (_totToLoad[0] != 0)
				_vm->_inter->_terminate = false;

			variablesCount = READ_LE_UINT32((char *)_totFileData + 0x2c);
			_vm->_draw->blitInvalidated();
			delete[] _totFileData;
			_totFileData = 0;

			if (needTextFree)
				delete[] _totTextData;
			_totTextData = 0;

			if (needFreeResTable)
				delete[] _totResourceTable;
			_totResourceTable = 0;

			delete[] _imFileData;
			_imFileData = 0;

			if (_extTable)
				delete[] _extTable->items;
			delete _extTable;
			_extTable = 0;

			if (_extHandle >= 0)
				_vm->_dataio->closeData(_extHandle);

			_extHandle = -1;

			for (i = 0; i < *_vm->_scenery->_pCaptureCounter; i++)
				capturePop(0);

			_vm->_mult->checkFreeMult();
			_vm->_mult->freeAll();

			for (i = 0; i < 20; i++) {
				if (_vm->_draw->_spritesArray[i] != 0)
					_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[i]);
				_vm->_draw->_spritesArray[i] = 0;
			}
			_vm->_snd->stopSound(0);

			for (i = 0; i < 20; i++)
				freeSoundSlot(i);

			if (_totToLoad[0] == 0)
				break;

			strcpy(_curTotFile, _totToLoad);
		}
	}

	strcpy(_curTotFile, savedTotName);

	_vm->_inter->_nestLevel = oldNestLevel;
	_vm->_inter->_breakFromLevel = oldBreakFrom;
	_vm->_scenery->_pCaptureCounter = oldCaptureCounter;
	_vm->_global->_inter_execPtr = savedIP;
}

void Game_v2::clearCollisions() {
	int16 i;

	_lastCollKey = 0;

	for (i = 0; i < 250; i++) {
		_collisionAreas[i].id = 0;
		_collisionAreas[i].left = -1;
	}
}

void Game_v2::addNewCollision(int16 id, int16 left, int16 top, int16 right, int16 bottom,
	    int16 flags, int16 key, int16 funcEnter, int16 funcLeave) {
	int16 i;
	Collision *ptr;

	debugC(5, DEBUG_COLLISIONS, "addNewCollision");
	debugC(5, DEBUG_COLLISIONS, "id = %x", id);
	debugC(5, DEBUG_COLLISIONS, "left = %d, top = %d, right = %d, bottom = %d", left, top, right, bottom);
	debugC(5, DEBUG_COLLISIONS, "flags = %x, key = %x", flags, key);
	debugC(5, DEBUG_COLLISIONS, "funcEnter = %d, funcLeave = %d", funcEnter, funcLeave);

	for (i = 0; i < 250; i++) {
		if ((_collisionAreas[i].left != -1) && (_collisionAreas[i].id != id))
			continue;

		ptr = &_collisionAreas[i];
		ptr->id = id;
		ptr->left = left;
		ptr->top = top;
		ptr->right = right;
		ptr->bottom = bottom;
		ptr->flags = flags;
		ptr->key = key;
		ptr->funcEnter = funcEnter;
		ptr->funcLeave = funcLeave;
		ptr->field_12 = 0;
		return;
	}
	error("addNewCollision: Collision array full!\n");
}

} // End of namespace Gob
