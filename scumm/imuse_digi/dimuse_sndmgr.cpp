/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "sound/voc.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

ImuseDigiSndMgr::ImuseDigiSndMgr(ScummEngine *scumm) {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		memset(&_sounds[l], 0, sizeof(soundStruct));
	}
	_vm = scumm;
	_disk = 0;
	_cacheBundleDir = new BundleDirCache();
	BundleCodecs::initializeImcTables();
}

ImuseDigiSndMgr::~ImuseDigiSndMgr() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		closeSound(&_sounds[l]);
	}
#ifdef __PALM_OS__
	BundleCodecs::releaseImcTables();
#endif
}

void ImuseDigiSndMgr::countElements(byte *ptr, int &numRegions, int &numJumps, int &numSyncs) {
	uint32 tag;
	int32 size = 0;

	do {
		tag = READ_BE_UINT32(ptr); ptr += 4;
		switch(tag) {
		case MKID_BE('TEXT'):
		case MKID_BE('STOP'):
		case MKID_BE('FRMT'):
		case MKID_BE('DATA'):
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKID_BE('REGN'):
			numRegions++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKID_BE('JUMP'):
			numJumps++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		case MKID_BE('SYNC'):
			numSyncs++;
			size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;
		default:
			error("ImuseDigiSndMgr::countElements() Unknown sfx header '%s'", tag2str(tag));
		}
	} while (tag != MKID_BE('DATA'));
}

void ImuseDigiSndMgr::prepareSound(byte *ptr, soundStruct *sound) {
	if (READ_UINT32(ptr) == MKID('Crea')) {
		bool quit = false;
		int len;

		int32 offset = READ_LE_UINT16(ptr + 20);
		int16 code = READ_LE_UINT16(ptr + 24);

		sound->region = (_region *)malloc(sizeof(_region) * 70);
		sound->jump = (_jump *)malloc(sizeof(_jump));
		sound->resPtr = ptr;
		sound->bits = 8;
		sound->channels = 1;

		while (!quit) {
			len = READ_LE_UINT32(ptr + offset);
			code = len & 0xFF;
			if ((code != 0) && (code != 1) && (code != 6) && (code != 7)) {
				// try again with 2 bytes forward (workaround for some FT sounds (ex.362, 363)
				offset += 2;
				len = READ_LE_UINT32(ptr + offset);
				code = len & 0xFF;
				if ((code != 0) && (code != 1) && (code != 6) && (code != 7)) {
					error("Invalid code in VOC file : %d", code);
				}
			}
			offset += 4;
			len >>= 8;
			switch(code) {
			case 0:
				quit = true;
				break;
			case 1:
				{
					int time_constant = ptr[offset];
					offset += 2;
					len -= 2;
					sound->freq = getSampleRateFromVOCRate(time_constant);
					sound->region[sound->numRegions].offset = offset;
					sound->region[sound->numRegions].length = len;
					sound->numRegions++;
				}
				break;
			case 6:	// begin of loop
				sound->jump[0].dest = offset + 8;
				sound->jump[0].hookId = 0;
				sound->jump[0].fadeDelay = 0;
				break;
			case 7:	// end of loop
				sound->jump[0].offset = offset - 4;
				sound->numJumps++;
				sound->region[sound->numRegions].offset = offset - 4;
				sound->region[sound->numRegions].length = 0;
				sound->numRegions++;
				break;
			default:
				error("Invalid code in VOC file : %d", code);
				quit = true;
				break;
			}
			offset += len;
		}
	} else if (READ_UINT32(ptr) == MKID('iMUS')) {
		uint32 tag;
		int32 size = 0;
		byte *s_ptr = ptr;
		ptr += 16;

		int curIndexRegion = 0;
		int curIndexJump = 0;
		int curIndexSync = 0;

		sound->numRegions = 0;
		sound->numJumps = 0;
		sound->numSyncs = 0;
		countElements(ptr, sound->numRegions, sound->numJumps, sound->numSyncs);
		sound->region = (_region *)malloc(sizeof(_region) * sound->numRegions);
		sound->jump = (_jump *)malloc(sizeof(_jump) * sound->numJumps);
		sound->sync = (_sync *)malloc(sizeof(_sync) * sound->numSyncs);

		do {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			switch(tag) {
			case MKID_BE('FRMT'):
				ptr += 12;
				sound->bits = READ_BE_UINT32(ptr); ptr += 4;
				sound->freq = READ_BE_UINT32(ptr); ptr += 4;
				sound->channels = READ_BE_UINT32(ptr); ptr += 4;
				break;
			case MKID_BE('TEXT'):
			case MKID_BE('STOP'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
			case MKID_BE('REGN'):
				ptr += 4;
				sound->region[curIndexRegion].offset = READ_BE_UINT32(ptr); ptr += 4;
				sound->region[curIndexRegion].length = READ_BE_UINT32(ptr); ptr += 4;
				curIndexRegion++;
				break;
			case MKID_BE('JUMP'):
				ptr += 4;
				sound->jump[curIndexJump].offset = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].dest = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].hookId = READ_BE_UINT32(ptr); ptr += 4;
				sound->jump[curIndexJump].fadeDelay = READ_BE_UINT32(ptr); ptr += 4;
				curIndexJump++;
				break;
			case MKID_BE('SYNC'):
				size = READ_BE_UINT32(ptr); ptr += 4;
				sound->sync[curIndexSync].size = size;
				sound->sync[curIndexSync].ptr = (byte *)malloc(size);
				memcpy(sound->sync[curIndexSync].ptr, ptr, size);
				curIndexSync++;
				ptr += size;
				break;
			case MKID_BE('DATA'):
				ptr += 4;
				break;
			default:
				error("ImuseDigiSndMgr::prepareSound(%d/%s) Unknown sfx header '%s'", sound->soundId, sound->name, tag2str(tag));
			}
		} while (tag != MKID_BE('DATA'));
		sound->offsetData =  ptr - s_ptr;
	} else {
		error("ImuseDigiSndMgr::prepareSound(): Unknown sound format");
	}
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::allocSlot() {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (!_sounds[l].inUse) {
			_sounds[l].inUse = true;
			return &_sounds[l];
		}
	}

	return NULL;
}

bool ImuseDigiSndMgr::openMusicBundle(soundStruct *sound, int disk) {
	bool result = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	if (_vm->_gameId == GID_CMI) {
		if (_vm->_features & GF_DEMO) {
			result = sound->bundle->openFile("music.bun");
		} else {
			char musicfile[20];
			if (disk == -1)
				sprintf(musicfile, "musdisk%d.bun", _vm->VAR(_vm->VAR_CURRENTDISK));
			else
				sprintf(musicfile, "musdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_imuseDigital->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->openFile(musicfile);

			if (result == false)
				result = sound->bundle->openFile("music.bun");
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_gameId == GID_DIG)
		result = sound->bundle->openFile("digmusic.bun");
	else
		error("ImuseDigiSndMgr::openMusicBundle() Don't know which bundle file to load");

	if (result)
		_vm->VAR(_vm->VAR_MUSIC_BUNDLE_LOADED) = 1;
	else
		_vm->VAR(_vm->VAR_MUSIC_BUNDLE_LOADED) = 0;

	return result;
}

bool ImuseDigiSndMgr::openVoiceBundle(soundStruct *sound, int disk) {
	bool result = false;

	sound->bundle = new BundleMgr(_cacheBundleDir);
	if (_vm->_gameId == GID_CMI) {
		if (_vm->_features & GF_DEMO) {
			result = sound->bundle->openFile("voice.bun");
		} else {
			char voxfile[20];
			if (disk == -1)
				sprintf(voxfile, "voxdisk%d.bun", _vm->VAR(_vm->VAR_CURRENTDISK));
			else
				sprintf(voxfile, "voxdisk%d.bun", disk);
//			if (_disk != _vm->VAR(_vm->VAR_CURRENTDISK)) {
//				_vm->_imuseDigital->parseScriptCmds(0x1000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->parseScriptCmds(0x2000, 0, 0, 0, 0, 0, 0, 0);
//				_vm->_imuseDigital->stopAllSounds();
//				sound->bundle->closeFile();
//			}

			result = sound->bundle->openFile(voxfile);

			if (result == false)
				result = sound->bundle->openFile("voice.bun");
			_disk = (byte)_vm->VAR(_vm->VAR_CURRENTDISK);
		}
	} else if (_vm->_gameId == GID_DIG)
		result = sound->bundle->openFile("digvoice.bun");
	else
		error("ImuseDigiSndMgr::openVoiceBundle() Don't know which bundle file to load");

	if (result)
		_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = 1;
	else
		_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = 0;

	return result;
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::openSound(int32 soundId, const char *soundName, int soundType, int volGroupId, int disk) {
	assert(soundId >= 0);
	assert(soundType);

	soundStruct *sound = allocSlot();
	if (!sound) {
		error("ImuseDigiSndMgr::openSound() can't alloc free sound slot");
	}

	const bool header_outside = ((_vm->_gameId == GID_CMI) && !(_vm->_features & GF_DEMO));
	bool result = false;
	byte *ptr = NULL;
	
	switch (soundType) {
	case IMUSE_RESOURCE:
		assert(soundName[0] == 0);	// Paranoia check

		_vm->ensureResourceLoaded(rtSound, soundId);
		_vm->lock(rtSound, soundId);
		ptr = _vm->getResourceAddress(rtSound, soundId);
		if (ptr == NULL) {
			closeSound(sound);
			return NULL;
		}
		sound->resPtr = ptr;
		break;
	case IMUSE_BUNDLE:
		if (volGroupId == IMUSE_VOLGRP_VOICE)
			result = openVoiceBundle(sound, disk);
		else if (volGroupId == IMUSE_VOLGRP_MUSIC)
			result = openMusicBundle(sound, disk);
		else
			error("ImuseDigiSndMgr::openSound() Don't know how load sound: %d", soundId);
		if (!result) {
			closeSound(sound);
			return NULL;
		}
		if (soundName[0] == 0) {
			if (sound->bundle->decompressSampleByIndex(soundId, 0, 0x2000, &ptr, 0, header_outside) == 0 || ptr == NULL) {
				closeSound(sound);
				return NULL;
			}
		} else {
			if (sound->bundle->decompressSampleByName(soundName, 0, 0x2000, &ptr, header_outside) == 0 || ptr == NULL) {
				closeSound(sound);
				return NULL;
			}
		}
		sound->resPtr = 0;
		break;
	default:
		error("ImuseDigiSndMgr::openSound() Unknown soundType %d (trying to load sound %d)", soundType, soundId);
	}

	strcpy(sound->name, soundName);
	sound->soundId = soundId;
	sound->type = soundType;
	sound->volGroupId = volGroupId;
	sound->disk = _disk;
	prepareSound(ptr, sound);
	return sound;
}

void ImuseDigiSndMgr::closeSound(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));

	if (soundHandle->resPtr) {
		bool found = false;
		for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
			if ((_sounds[l].soundId == soundHandle->soundId) && (&_sounds[l] != soundHandle))
				found = true;
		}
		if (!found)
			_vm->unlock(rtSound, soundHandle->soundId);
	}

	delete soundHandle->bundle;
	for (int r = 0; r < soundHandle->numSyncs; r++)
		free(soundHandle->sync[r].ptr);
	free(soundHandle->region);
	free(soundHandle->jump);
	free(soundHandle->sync);
	memset(soundHandle, 0, sizeof(soundStruct));
}

ImuseDigiSndMgr::soundStruct *ImuseDigiSndMgr::cloneSound(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));

	return openSound(soundHandle->soundId, soundHandle->name, soundHandle->type, soundHandle->volGroupId, soundHandle->disk);
}

bool ImuseDigiSndMgr::checkForProperHandle(soundStruct *soundHandle) {
	for (int l = 0; l < MAX_IMUSE_SOUNDS; l++) {
		if (soundHandle == &_sounds[l])
			return true;
	}
	return false;
}

int ImuseDigiSndMgr::getFreq(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->freq;
}

int ImuseDigiSndMgr::getBits(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->bits;
}

int ImuseDigiSndMgr::getChannels(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->channels;
}

bool ImuseDigiSndMgr::isEndOfRegion(soundStruct *soundHandle, int region) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->endFlag;
}

int ImuseDigiSndMgr::getNumRegions(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numRegions;
}

int ImuseDigiSndMgr::getNumJumps(soundStruct *soundHandle) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	return soundHandle->numJumps;
}

int ImuseDigiSndMgr::getRegionOffset(soundStruct *soundHandle, int region) {
	debug(5, "getRegionOffset() region:%d");
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	return soundHandle->region[region].offset;
}

int ImuseDigiSndMgr::getJumpIdByRegionAndHookId(soundStruct *soundHandle, int region, int hookId) {
	debug(5, "getJumpIdByRegionAndHookId() region:%d, hookId:%d", region, hookId);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(region >= 0 && region < soundHandle->numRegions);
	int32 offset = soundHandle->region[region].offset;
	for (int l = 0; l < soundHandle->numJumps; l++) {
		if (offset == soundHandle->jump[l].offset) {
			if (soundHandle->jump[l].hookId == hookId)
				return l;
		}
	}
	
	return -1;
}

void ImuseDigiSndMgr::getSyncSizeAndPtrById(soundStruct *soundHandle, int number, int32 &sync_size, byte **sync_ptr) {
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0);
	if (number < soundHandle->numSyncs) {
		sync_size = soundHandle->sync[number].size;
		*sync_ptr = soundHandle->sync[number].ptr;
	} else {
		sync_size = 0;
		*sync_ptr = NULL;
	}
}

int ImuseDigiSndMgr::getRegionIdByJumpId(soundStruct *soundHandle, int jumpId) {
	debug(5, "getRegionIdByJumpId() jumpId:%d", jumpId);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(jumpId >= 0 && jumpId < soundHandle->numJumps);
	int32 dest = soundHandle->jump[jumpId].dest;
	for (int l = 0; l < soundHandle->numRegions; l++) {
		if (dest == soundHandle->region[l].offset) {
			return l;
		}
	}

	return -1;
}

int ImuseDigiSndMgr::getJumpHookId(soundStruct *soundHandle, int number) {
	debug(5, "getJumpHookId() number:%d", number);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].hookId;
}

int ImuseDigiSndMgr::getJumpFade(soundStruct *soundHandle, int number) {
	debug(5, "getJumpFade() number:%d", number);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(number >= 0 && number < soundHandle->numJumps);
	return soundHandle->jump[number].fadeDelay;
}

int32 ImuseDigiSndMgr::getDataFromRegion(soundStruct *soundHandle, int region, byte **buf, int32 offset, int32 size) {
	debug(5, "getDataFromRegion() region:%d, offset:%d, size:%d, numRegions:%d", region, offset, size, soundHandle->numRegions);
	assert(soundHandle && checkForProperHandle(soundHandle));
	assert(buf && offset >= 0 && size >= 0);
	assert(region >= 0 && region < soundHandle->numRegions);

	int32 region_offset = soundHandle->region[region].offset;
	int32 region_length = soundHandle->region[region].length;
	int32 offset_data = soundHandle->offsetData;
	int32 start = region_offset - offset_data;

	if (offset + size + offset_data > region_length) {
		size = region_length - offset;
		soundHandle->endFlag = true;
	} else {
		soundHandle->endFlag = false;
	}

	int header_size = soundHandle->offsetData;
	bool header_outside = ((_vm->_gameId == GID_CMI) && !(_vm->_features & GF_DEMO));
	if (soundHandle->bundle) {
		size = soundHandle->bundle->decompressSampleByCurIndex(start + offset, size, buf, header_size, header_outside);
	} else if (soundHandle->resPtr) {
		*buf = (byte *)malloc(size);
		memcpy(*buf, soundHandle->resPtr + start + offset + header_size, size);
	}
	
	return size;
}

} // End of namespace Scumm
