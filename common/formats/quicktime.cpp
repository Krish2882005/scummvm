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

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "common/debug.h"
#include "common/endian.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/formats/quicktime.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/compression/deflate.h"

namespace Common {

////////////////////////////////////////////
// QuickTimeParser
////////////////////////////////////////////

QuickTimeParser::QuickTimeParser() {
	_beginOffset = 0;
	_fd = nullptr;
	_scaleFactorX = 1;
	_scaleFactorY = 1;
	_resFork = new MacResManager();
	_disposeFileHandle = DisposeAfterUse::YES;
	_timeScale = 1;
	_qtvrType = QTVRType::OTHER;

	initParseTable();
}

QuickTimeParser::~QuickTimeParser() {
	close();
	delete _resFork;
}

bool QuickTimeParser::parseFile(const Path &filename) {
	if (!_resFork->open(filename))
		return false;

	_foundMOOV = false;
	_disposeFileHandle = DisposeAfterUse::YES;

	Atom atom = { 0, 0, 0 };

	if (_resFork->hasResFork()) {
		// Search for a 'moov' resource
		MacResIDArray idArray = _resFork->getResIDArray(MKTAG('m', 'o', 'o', 'v'));

		if (!idArray.empty())
			_fd = _resFork->getResource(MKTAG('m', 'o', 'o', 'v'), idArray[0]);

		if (_fd) {
			atom.size = _fd->size();
			if (readDefault(atom) < 0 || !_foundMOOV)
				return false;
		}

		delete _fd;
	}

	_fd = Common::MacResManager::openFileOrDataFork(filename);
	if (!_fd)
		return false;
	atom.size = _fd->size();

	if (readDefault(atom) < 0 || !_foundMOOV)
		return false;

	init();
	return true;
}

bool QuickTimeParser::parseStream(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	_fd = stream;
	_foundMOOV = false;
	_disposeFileHandle = disposeFileHandle;

	Atom atom = { 0, 0, 0xffffffff };

	if (readDefault(atom) < 0 || !_foundMOOV) {
		close();
		return false;
	}

	init();
	return true;
}

void QuickTimeParser::init() {
	for (uint32 i = 0; i < _tracks.size(); i++) {
		// Remove unknown/unhandled tracks
		if (_tracks[i]->codecType == CODEC_TYPE_MOV_OTHER) {
			delete _tracks[i];
			_tracks.remove_at(i);
			i--;
		} else {
			// If this track doesn't have a declared scale, use the movie scale
			if (_tracks[i]->timeScale == 0)
				_tracks[i]->timeScale = _timeScale;

			// If this track doesn't have an edit list (like in MPEG-4 files),
			// fake an entry of one edit that takes up the entire sample
			if (_tracks[i]->editList.size() == 0) {
				_tracks[i]->editList.resize(1);
				_tracks[i]->editList[0].trackDuration = _tracks[i]->duration;
				_tracks[i]->editList[0].timeOffset = 0;
				_tracks[i]->editList[0].mediaTime = 0;
				_tracks[i]->editList[0].mediaRate = 1;
			}
		}
	}
}

void QuickTimeParser::initParseTable() {
	static const ParseTable p[] = {
		{ &QuickTimeParser::readDefault, MKTAG('d', 'i', 'n', 'f') },
		{ &QuickTimeParser::readDREF,    MKTAG('d', 'r', 'e', 'f') },
		{ &QuickTimeParser::readDefault, MKTAG('e', 'd', 't', 's') },
		{ &QuickTimeParser::readELST,    MKTAG('e', 'l', 's', 't') },
		{ &QuickTimeParser::readHDLR,    MKTAG('h', 'd', 'l', 'r') },
		{ &QuickTimeParser::readLeaf,    MKTAG('m', 'd', 'a', 't') },
		{ &QuickTimeParser::readMDHD,    MKTAG('m', 'd', 'h', 'd') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'd', 'i', 'a') },
		{ &QuickTimeParser::readDefault, MKTAG('m', 'i', 'n', 'f') },
		{ &QuickTimeParser::readMOOV,    MKTAG('m', 'o', 'o', 'v') },
		{ &QuickTimeParser::readMVHD,    MKTAG('m', 'v', 'h', 'd') },
		{ &QuickTimeParser::readLeaf,    MKTAG('s', 'm', 'h', 'd') },
		{ &QuickTimeParser::readDefault, MKTAG('s', 't', 'b', 'l') },
		{ &QuickTimeParser::readSTCO,    MKTAG('s', 't', 'c', 'o') },
		{ &QuickTimeParser::readSTSC,    MKTAG('s', 't', 's', 'c') },
		{ &QuickTimeParser::readSTSD,    MKTAG('s', 't', 's', 'd') },
		{ &QuickTimeParser::readSTSS,    MKTAG('s', 't', 's', 's') },
		{ &QuickTimeParser::readSTSZ,    MKTAG('s', 't', 's', 'z') },
		{ &QuickTimeParser::readSTTS,    MKTAG('s', 't', 't', 's') },
		{ &QuickTimeParser::readTKHD,    MKTAG('t', 'k', 'h', 'd') },
		{ &QuickTimeParser::readTRAK,    MKTAG('t', 'r', 'a', 'k') },
		{ &QuickTimeParser::readDefault, MKTAG('u', 'd', 't', 'a') },
		{ &QuickTimeParser::readCTYP,    MKTAG('c', 't', 'y', 'p') },
		{ &QuickTimeParser::readNAVG,    MKTAG('N', 'A', 'V', 'G') },
		{ &QuickTimeParser::readVMHD,    MKTAG('v', 'm', 'h', 'd') },
		{ &QuickTimeParser::readCMOV,    MKTAG('c', 'm', 'o', 'v') },
		{ &QuickTimeParser::readWAVE,    MKTAG('w', 'a', 'v', 'e') },
		{ &QuickTimeParser::readESDS,    MKTAG('e', 's', 'd', 's') },
		{ &QuickTimeParser::readSMI,     MKTAG('S', 'M', 'I', ' ') },
		{ &QuickTimeParser::readDefault, MKTAG('g', 'm', 'h', 'd') },
		{ &QuickTimeParser::readLeaf,    MKTAG('g', 'm', 'i', 'n') },
		{ &QuickTimeParser::readDefault, MKTAG('S', 'T', 'p', 'n') },
		{ &QuickTimeParser::readPINF,    MKTAG('p', 'I', 'n', 'f') },
		{ nullptr, 0 }
	};

	_parseTable = p;
}

int QuickTimeParser::readDefault(Atom atom) {
	uint32 total_size = 0;
	Atom a;
	int err = 0;

	a.offset = atom.offset;

	if (_fd->eos() || _fd->err() || (_fd->pos() == _fd->size()))
		return -1;

	while(((total_size + 8) < atom.size) && !_fd->eos() && _fd->pos() < _fd->size() && !err) {
		a.size = atom.size;
		a.type = 0;

		if (atom.size >= 8) {
			a.size = _fd->readUint32BE();
			a.type = _fd->readUint32BE();

			// Some QuickTime videos with resource forks have mdat chunks
			// that are of size 0. Adjust it so it's the correct size.
			if (a.type == MKTAG('m', 'd', 'a', 't') && a.size == 0)
				a.size = _fd->size();
		}

		total_size += 8;
		a.offset += 8;
		debug(4, "type: %08x  %.4s  sz: %x %x %x", a.type, tag2str(a.type), a.size, atom.size, total_size);

		if (a.size == 1) { // 64 bit extended size
			warning("64 bit extended size is not supported in QuickTime");
			return -1;
		}

		if (a.size == 0) {
			a.size = atom.size - total_size;
			if (a.size <= 8)
				break;
		}

		uint32 i = 0;

		for (; _parseTable[i].type != 0 && _parseTable[i].type != a.type; i++)
			; // Empty

		if (a.size < 8)
			break;

		a.size -= 8;

		if (a.size + (uint32)_fd->pos() > (uint32)_fd->size()) {
			_fd->seek(_fd->size());
			debug(0, "Skipping junk found at the end of the QuickTime file");
			return 0;
		} else if (_parseTable[i].type == 0) { // skip leaf atom data
			debug(0, ">>> Skipped [%s]", tag2str(a.type));

			_fd->seek(a.size, SEEK_CUR);
		} else {
			uint32 start_pos = _fd->pos();
			err = (this->*_parseTable[i].func)(a);

			if (!err && (_fd->eos() || _fd->err()))
				err = -1;

			uint32 left = a.size - _fd->pos() + start_pos;

			if (left > 0) // skip garbage at atom end
				_fd->seek(left, SEEK_CUR);
		}

		a.offset += a.size;
		total_size += a.size;
	}

	if (!err && total_size < atom.size)
		_fd->seek(atom.size - total_size, SEEK_SET);

	return err;
}

int QuickTimeParser::readLeaf(Atom atom) {
	if (atom.size > 1)
		_fd->seek(atom.size, SEEK_SET);

	return 0;
}

int QuickTimeParser::readMOOV(Atom atom) {
	if (readDefault(atom) < 0)
		return -1;

	// We parsed the 'moov' atom, so we don't need anything else
	_foundMOOV = true;
	return 1;
}

int QuickTimeParser::readCMOV(Atom atom) {
	// Read in the dcom atom
	_fd->readUint32BE();
	if (_fd->readUint32BE() != MKTAG('d', 'c', 'o', 'm'))
		return -1;
	if (_fd->readUint32BE() != MKTAG('z', 'l', 'i', 'b')) {
		warning("Unknown cmov compression type");
		return -1;
	}

	// Read in the cmvd atom
	uint32 compressedSize = _fd->readUint32BE() - 12;
	if (_fd->readUint32BE() != MKTAG('c', 'm', 'v', 'd'))
		return -1;
	uint32 uncompressedSize = _fd->readUint32BE();

	// Read in data
	byte *compressedData = (byte *)malloc(compressedSize);
	_fd->read(compressedData, compressedSize);

	// Create uncompressed stream
	byte *uncompressedData = (byte *)malloc(uncompressedSize);

	// Uncompress the data
	unsigned long dstLen = uncompressedSize;
	if (!inflateZlib(uncompressedData, &dstLen, compressedData, compressedSize)) {
		warning ("Could not uncompress cmov chunk");
		free(compressedData);
		free(uncompressedData);
		return -1;
	}

	// Load data into a new MemoryReadStream and assign _fd to be that
	SeekableReadStream *oldStream = _fd;
	_fd = new MemoryReadStream(uncompressedData, uncompressedSize, DisposeAfterUse::YES);

	// Read the contents of the uncompressed data
	Atom a = { MKTAG('m', 'o', 'o', 'v'), 0, uncompressedSize };
	int err = readDefault(a);

	// Assign the file handle back to the original handle
	free(compressedData);
	delete _fd;
	_fd = oldStream;

	return err;
}

int QuickTimeParser::readMVHD(Atom atom) {
	byte version = _fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	if (version == 1) {
		warning("QuickTime version 1");
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	_timeScale = _fd->readUint32BE(); // time scale
	debug(0, "time scale = %i\n", _timeScale);

	// duration
	_duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE();
	_fd->readUint32BE(); // preferred scale

	_fd->readUint16BE(); // preferred volume

	_fd->seek(10, SEEK_CUR); // reserved

	// We only need two values from the movie display matrix. Most of the values are just
	// skipped. xMod and yMod are 16:16 fixed point numbers, the last part of the 3x3 matrix
	// is 2:30.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	_scaleFactorX = Rational(0x10000, xMod);
	_scaleFactorY = Rational(0x10000, yMod);

	_scaleFactorX.debugPrint(1, "readMVHD(): scaleFactorX =");
	_scaleFactorY.debugPrint(1, "readMVHD(): scaleFactorY =");

	_fd->readUint32BE(); // preview time
	_fd->readUint32BE(); // preview duration
	_fd->readUint32BE(); // poster time
	_fd->readUint32BE(); // selection time
	_fd->readUint32BE(); // selection duration
	_fd->readUint32BE(); // current time
	_fd->readUint32BE(); // next track ID

	return 0;
}

int QuickTimeParser::readTRAK(Atom atom) {
	Track *track = new Track();

	track->codecType = CODEC_TYPE_MOV_OTHER;
	_tracks.push_back(track);

	return readDefault(atom);
}

int QuickTimeParser::readTKHD(Atom atom) {
	Track *track = _tracks.back();
	byte version = _fd->readByte();

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags
	//
	//MOV_TRACK_ENABLED 0x0001
	//MOV_TRACK_IN_MOVIE 0x0002
	//MOV_TRACK_IN_PREVIEW 0x0004
	//MOV_TRACK_IN_POSTER 0x0008
	//

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	/* track->id = */_fd->readUint32BE(); // track id (NOT 0 !)
	_fd->readUint32BE(); // reserved
	track->duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // highlevel (considering edits) duration in movie timebase
	_fd->readUint32BE(); // reserved
	_fd->readUint32BE(); // reserved

	_fd->readUint16BE(); // layer
	_fd->readUint16BE(); // alternate group
	_fd->readUint16BE(); // volume
	_fd->readUint16BE(); // reserved

	// We only need the two values from the displacement matrix for a track.
	// See readMVHD() for more information.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	track->scaleFactorX = Rational(0x10000, xMod);
	track->scaleFactorY = Rational(0x10000, yMod);

	track->scaleFactorX.debugPrint(1, "readTKHD(): scaleFactorX =");
	track->scaleFactorY.debugPrint(1, "readTKHD(): scaleFactorY =");

	// these are fixed-point, 16:16
	//_fd->readUint32BE() >> 16; // track width
	//_fd->readUint32BE() >> 16; // track height

	return 0;
}

// edit list atom
int QuickTimeParser::readELST(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	uint32 editCount = _fd->readUint32BE();
	track->editList.resize(editCount);

	debug(2, "Track %d edit list count: %d", _tracks.size() - 1, editCount);

	uint32 offset = 0;

	for (uint32 i = 0; i < editCount; i++) {
		track->editList[i].trackDuration = _fd->readUint32BE();
		track->editList[i].mediaTime = _fd->readSint32BE();
		track->editList[i].mediaRate = Rational(_fd->readUint32BE(), 0x10000);
		track->editList[i].timeOffset = offset;
		debugN(3, "\tDuration = %d (Offset = %d), Media Time = %d, ", track->editList[i].trackDuration, offset, track->editList[i].mediaTime);
		track->editList[i].mediaRate.debugPrint(3, "Media Rate =");
		offset += track->editList[i].trackDuration;
	}

	return 0;
}

int QuickTimeParser::readHDLR(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	// component type
	uint32 ctype = _fd->readUint32BE();
	uint32 type = _fd->readUint32BE(); // component subtype

	debug(0, "ctype= %s (0x%08lx)", tag2str(ctype), (long)ctype);
	debug(0, "stype= %s", tag2str(type));

	if (ctype == MKTAG('m', 'h', 'l', 'r')) // MOV
		debug(0, "MOV detected");
	else if (ctype == 0)
		debug(0, "MPEG-4 detected");

	if (type == MKTAG('v', 'i', 'd', 'e'))
		track->codecType = CODEC_TYPE_VIDEO;
	else if (type == MKTAG('s', 'o', 'u', 'n'))
		track->codecType = CODEC_TYPE_AUDIO;
	else if (type == MKTAG('m', 'u', 's', 'i'))
		track->codecType = CODEC_TYPE_MIDI;

	_fd->readUint32BE(); // component manufacture
	_fd->readUint32BE(); // component flags
	_fd->readUint32BE(); // component flags mask

	if (atom.size <= 24)
		return 0; // nothing left to read

	// .mov: PASCAL string
	byte len = _fd->readByte();
	_fd->seek(len, SEEK_CUR);

	_fd->seek(atom.size - (_fd->pos() - atom.offset), SEEK_CUR);

	return 0;
}

int QuickTimeParser::readMDHD(Atom atom) {
	Track *track = _tracks.back();
	byte version = _fd->readByte();

	if (version > 1)
		return 1; // unsupported

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	track->timeScale = _fd->readUint32BE();
	track->mediaDuration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // duration

	_fd->readUint16BE(); // language
	_fd->readUint16BE(); // quality

	return 0;
}

int QuickTimeParser::readSTSD(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	uint32 entryCount = _fd->readUint32BE();
	track->sampleDescs.reserve(entryCount);

	for (uint32 i = 0; i < entryCount; i++) { // Parsing Sample description table
		Atom a = { 0, 0, 0 };
		uint32 start_pos = _fd->pos();
		int size = _fd->readUint32BE(); // size
		uint32 format = _fd->readUint32BE(); // data format

		_fd->readUint32BE(); // reserved
		_fd->readUint16BE(); // reserved
		_fd->readUint16BE(); // index

		track->sampleDescs.push_back(readSampleDesc(track, format, size - 16));

		debug(0, "size=%d 4CC= %s codec_type=%d", size, tag2str(format), track->codecType);

		if (!track->sampleDescs[i]) {
			// other codec type, just skip (rtp, mp4s, tmcd ...)
			_fd->seek(size - (_fd->pos() - start_pos), SEEK_CUR);
		}

		// this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...)
		a.size = size - (_fd->pos() - start_pos);
		if (a.size > 8)
			readDefault(a);
		else if (a.size > 0)
			_fd->seek(a.size, SEEK_CUR);
	}

	return 0;
}

int QuickTimeParser::readSTSC(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->sampleToChunkCount = _fd->readUint32BE();

	debug(0, "track[%i].stsc.entries = %i", _tracks.size() - 1, track->sampleToChunkCount);

	track->sampleToChunk = new SampleToChunkEntry[track->sampleToChunkCount];

	if (!track->sampleToChunk)
		return -1;

	for (uint32 i = 0; i < track->sampleToChunkCount; i++) {
		track->sampleToChunk[i].first = _fd->readUint32BE() - 1;
		track->sampleToChunk[i].count = _fd->readUint32BE();
		track->sampleToChunk[i].id = _fd->readUint32BE();
		//warning("Sample to Chunk[%d]: First = %d, Count = %d", i, track->sampleToChunk[i].first, track->sampleToChunk[i].count);
	}

	return 0;
}

int QuickTimeParser::readSTSS(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->keyframeCount = _fd->readUint32BE();

	debug(0, "keyframeCount = %d", track->keyframeCount);

	track->keyframes = new uint32[track->keyframeCount];

	if (!track->keyframes)
		return -1;

	for (uint32 i = 0; i < track->keyframeCount; i++) {
		track->keyframes[i] = _fd->readUint32BE() - 1; // Adjust here, the frames are based on 1
		debug(6, "keyframes[%d] = %d", i, track->keyframes[i]);

	}
	return 0;
}

int QuickTimeParser::readSTSZ(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->sampleSize = _fd->readUint32BE();
	track->sampleCount = _fd->readUint32BE();

	debug(5, "sampleSize = %d sampleCount = %d", track->sampleSize, track->sampleCount);

	if (track->sampleSize)
		return 0; // there isn't any table following

	track->sampleSizes = new uint32[track->sampleCount];

	if (!track->sampleSizes)
		return -1;

	for(uint32 i = 0; i < track->sampleCount; i++) {
		track->sampleSizes[i] = _fd->readUint32BE();
		debug(6, "sampleSizes[%d] = %d", i, track->sampleSizes[i]);
	}

	return 0;
}

int QuickTimeParser::readSTTS(Atom atom) {
	Track *track = _tracks.back();
	uint32 totalSampleCount = 0;

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->timeToSampleCount = _fd->readUint32BE();
	track->timeToSample = new TimeToSampleEntry[track->timeToSampleCount];

	debug(0, "track[%d].stts.entries = %d", _tracks.size() - 1, track->timeToSampleCount);

	for (int32 i = 0; i < track->timeToSampleCount; i++) {
		track->timeToSample[i].count = _fd->readUint32BE();
		track->timeToSample[i].duration = _fd->readUint32BE();

		debug(1, "\tCount = %d, Duration = %d", track->timeToSample[i].count, track->timeToSample[i].duration);

		totalSampleCount += track->timeToSample[i].count;
	}

	track->frameCount = totalSampleCount;
	return 0;
}

int QuickTimeParser::readVMHD(Atom atom) {
	Track *track = _tracks.back();

	_fd->readUint32BE(); // version + flags
	track->graphicsMode = (GraphicsMode)_fd->readUint16BE();
	_fd->readMultipleBE(track->opcolor);

	return 0;
}

int QuickTimeParser::readSTCO(Atom atom) {
	Track *track = _tracks.back();

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	track->chunkCount = _fd->readUint32BE();
	track->chunkOffsets = new uint32[track->chunkCount];

	if (!track->chunkOffsets)
		return -1;

	for (uint32 i = 0; i < track->chunkCount; i++) {
		// WORKAROUND/HACK: The offsets in Riven videos (ones inside the Mohawk archives themselves)
		// have offsets relative to the archive and not the video. This is quite nasty. We subtract
		// the initial offset of the stream to get the correct value inside of the stream.
		track->chunkOffsets[i] = _fd->readUint32BE() - _beginOffset;
	}

	return 0;
}

int QuickTimeParser::readWAVE(Atom atom) {
	if (_tracks.empty())
		return 0;

	Track *track = _tracks.back();

	if (atom.size > (1 << 30))
		return -1;

	// We should only get here within an stsd atom
	if (track->sampleDescs.empty())
		return -1;

	SampleDesc *sampleDesc = track->sampleDescs.back();

	if (sampleDesc->getCodecTag() == MKTAG('Q', 'D', 'M', '2')) // Read extra data for QDM2
		sampleDesc->_extraData = _fd->readStream(atom.size);
	else if (atom.size > 8)
		return readDefault(atom);
	else
		_fd->skip(atom.size);

	return 0;
}

enum {
	kMP4IODescTag          = 2,
	kMP4ESDescTag          = 3,
	kMP4DecConfigDescTag   = 4,
	kMP4DecSpecificDescTag = 5
};

static int readMP4DescLength(SeekableReadStream *stream) {
	int length = 0;
	int count = 4;

	while (count--) {
		byte c = stream->readByte();
		length = (length << 7) | (c & 0x7f);

		if (!(c & 0x80))
			break;
	}

	return length;
}

static void readMP4Desc(SeekableReadStream *stream, byte &tag, int &length) {
	tag = stream->readByte();
	length = readMP4DescLength(stream);
}

int QuickTimeParser::readESDS(Atom atom) {
	if (_tracks.empty())
		return 0;

	Track *track = _tracks.back();

	// We should only get here within an stsd atom
	if (track->sampleDescs.empty())
		return -1;

	SampleDesc *sampleDesc = track->sampleDescs.back();

	_fd->readUint32BE(); // version + flags

	byte tag;
	int length;

	readMP4Desc(_fd, tag, length);
	_fd->readUint16BE(); // id
	if (tag == kMP4ESDescTag)
		_fd->readByte(); // priority

	// Check if we've got the Config MPEG-4 header
	readMP4Desc(_fd, tag, length);
	if (tag != kMP4DecConfigDescTag)
		return 0;

	sampleDesc->_objectTypeMP4 = _fd->readByte();
	_fd->readByte();                      // stream type
	_fd->readUint16BE(); _fd->readByte(); // buffer size
	_fd->readUint32BE();                  // max bitrate
	_fd->readUint32BE();                  // avg bitrate

	// Check if we've got the Specific MPEG-4 header
	readMP4Desc(_fd, tag, length);
	if (tag != kMP4DecSpecificDescTag)
		return 0;

	sampleDesc->_extraData = _fd->readStream(length);

	debug(0, "MPEG-4 object type = %02x", sampleDesc->_objectTypeMP4);
	return 0;
}

int QuickTimeParser::readSMI(Atom atom) {
	if (_tracks.empty())
		return 0;

	Track *track = _tracks.back();

	// We should only get here within an stsd atom
	if (track->sampleDescs.empty())
		return -1;

	SampleDesc *sampleDesc = track->sampleDescs.back();

	// This atom just contains SVQ3 extra data
	sampleDesc->_extraData = _fd->readStream(atom.size);

	return 0;
}

int QuickTimeParser::readCTYP(Atom atom) {
	uint32 ctype = _fd->readUint32BE();

	switch (ctype) {
	case MKTAG('s', 't', 'n', 'a'):
		_qtvrType = QTVRType::OBJECT;
		break;

	case MKTAG('S', 'T', 'p', 'n'):
	case MKTAG('s', 't', 'p', 'n'):
		_qtvrType = QTVRType::PANORAMA;
		break;

	default:
		_qtvrType = QTVRType::OTHER;
		warning("QuickTimeParser::readCTYP(): Unknown QTVR Type ('%s')", tag2str(ctype));
		break;
	}

	return 0;
}

static float readAppleFloatField(SeekableReadStream *stream) {
	int16 a = stream->readSint16BE();
	uint16 b = stream->readUint16BE();

	float value = (float)a + (float)b / 65536.0f;

	return value;
}

int QuickTimeParser::readNAVG(Atom atom) {
	_fd->readUint16BE(); // version
	_nav.columns = _fd->readUint16BE();
	_nav.rows = _fd->readUint16BE();
	_fd->readUint16BE(); // reserved
	_nav.loop_size = _fd->readUint16BE();
	_nav.frame_duration = _fd->readUint16BE();
	_nav.movie_type = (MovieType)_fd->readUint16BE();
	_nav.loop_ticks = _fd->readUint16BE();
	_nav.field_of_view = readAppleFloatField(_fd);
	_nav.startHPan = readAppleFloatField(_fd);
	_nav.endHPan = readAppleFloatField(_fd);
	_nav.endVPan = readAppleFloatField(_fd);
	_nav.startVPan = readAppleFloatField(_fd);
	_nav.initialHPan = readAppleFloatField(_fd);
	_nav.initialVPan = readAppleFloatField(_fd);
	_fd->readUint32BE(); // reserved2

	return 0;
}

int QuickTimeParser::readPINF(Atom atom) {
	Track *track = _tracks.back();

	track->panoInfo.name = _fd->readPascalString();
	_fd->seek((int64)atom.offset + 32);
	track->panoInfo.defNodeID = _fd->readUint32BE();
	track->panoInfo.defZoom = readAppleFloatField(_fd);
	_fd->readUint32BE(); // reserved
	_fd->readSint16BE(); // padding
	int16 numEntries = _fd->readSint16BE();
	track->panoInfo.nodes.resize(numEntries);
	for (int16 i = 0; i < numEntries; i++) {
		track->panoInfo.nodes[i].nodeID = _fd->readUint32BE();
		track->panoInfo.nodes[i].timestamp = _fd->readUint32BE();
	}

	return 0;
}

int QuickTimeParser::readDREF(Atom atom) {
	if (atom.size > 1) {
		Track *track = _tracks.back();

		uint32 endPos = _fd->pos() + atom.size;
		_fd->readUint32BE(); // version + flags
		uint32 entries = _fd->readUint32BE();
		for (uint32 i = 0; i < entries && _fd->pos() < endPos; i++) {
			uint32 size = _fd->readUint32BE();
			uint32 next = _fd->pos() + size - 4;
			if (next > endPos) {
				warning("DREF chunk overflows atom bounds");
				return 1;
			}
			uint32 type = _fd->readUint32BE();
			_fd->readUint32BE(); // version + flags
			if (type == MKTAG('a', 'l', 'i', 's')) {
				if (size < 150) {
					_fd->seek(next, SEEK_SET);
					continue;
				}

				// Macintosh alias record
				_fd->seek(10, SEEK_CUR);

				uint8 volumeSize = MIN((uint8)27, _fd->readByte());
				track->volume = _fd->readString('\0', volumeSize);
				_fd->seek(27 - volumeSize, SEEK_CUR);
				_fd->seek(12, SEEK_CUR);

				uint8 filenameSize = MIN((uint8)63, _fd->readByte());
				track->filename = _fd->readString('\0', filenameSize);
				_fd->seek(63 - filenameSize, SEEK_CUR);
				_fd->seek(16, SEEK_CUR);
				debug(5, "volume: %s, filename: %s", track->volume.c_str(), track->filename.c_str());

				track->nlvlFrom = _fd->readSint16BE();
				track->nlvlTo = _fd->readSint16BE();
				_fd->seek(16, SEEK_CUR);
				debug(5, "nlvlFrom: %d, nlvlTo: %d", track->nlvlFrom, track->nlvlTo);

				for (int16 subType = 0; subType != -1 && _fd->pos() < endPos;) {
					subType = _fd->readSint16BE();
					uint16 subTypeSize = _fd->readUint16BE();
					subTypeSize += subTypeSize & 1 ? 1 : 0;
					if (subType == 2) { // Absolute path
						track->path = _fd->readString('\0', subTypeSize);
						if (track->path.substr(0, volumeSize) == track->volume) {
							track->path = track->path.substr(volumeSize);
						}
						debug(5, "path: %s", track->path.c_str());
					} else if (subType == 0) {
						track->directory = _fd->readString('\0', subTypeSize);
						debug(5, "directory: %s", track->directory.c_str());
					} else {
						_fd->seek(subTypeSize, SEEK_CUR);
					}
				}
			} else {
				warning("Unknown DREF type %s", tag2str(type));
				_fd->seek(next, SEEK_SET);
			}
		}

		_fd->seek(endPos, SEEK_SET);
	}

	return 0;
}

void QuickTimeParser::close() {
	for (uint32 i = 0; i < _tracks.size(); i++)
		delete _tracks[i];

	_tracks.clear();

	if (_disposeFileHandle == DisposeAfterUse::YES)
		delete _fd;

	_fd = nullptr;
}

void QuickTimeParser::flattenEditLists() {
	// This flattens the movie edit list, collapsing everything into a single edit.
	// This is necessary to work around sound popping in Obsidian on certain movies:
	//
	// For some reason, numerous movies have audio tracks with edit lists consisting
	// off numerous 0.5-second duration chunks, which is 22050 audio examples at the
	// 44100Hz media rate, but the edit media times are spaced out 22080 apart.
	//
	// The QuickTime File Format reference seems to suggest that this means the audio
	// would skip ahead 30 samples every half-second, which is what the playback code
	// currently does, and that causes audible popping in some movies (such as the
	// cube maze greeter vidbot when she says "Take take take, that's all you ever do!"
	// after repeatedly visiting her.)
	//
	// Other players seem to just play the audio track chunks consecutively without the
	// 30-sample skips, which produces the correct results, not sure why.
	//
	//
	// We also need to account for mixed silent and non-silent tracks.  In Obsidian's
	// Japanese localization, the vidbot that you talk to at the end of the maze (asset
	// 4375) has a brief silent edit followed by the actual audio track.  If we
	// collapse the audio track into the silent edit, then it causes the entire track
	// to be silent.
	for (Track *track : _tracks) {
		if (track->editList.size() >= 2) {
			Common::Array<EditListEntry> newEdits;

			for (const EditListEntry &curEdit : track->editList) {
				if (newEdits.size() == 0) {
					newEdits.push_back(curEdit);
					continue;
				}

				EditListEntry &prevEdit = newEdits.back();
				bool prevIsSilent = (prevEdit.mediaTime == -1);
				bool currentIsSilent = (curEdit.mediaTime == -1);

				if (prevIsSilent != currentIsSilent) {
					newEdits.push_back(curEdit);
					continue;
				} else
					prevEdit.trackDuration += curEdit.trackDuration;
			}

			track->editList = Common::move(newEdits);
		}
	}
}

QuickTimeParser::SampleDesc::SampleDesc(Track *parentTrack, uint32 codecTag) {
	_parentTrack = parentTrack;
	_codecTag = codecTag;
	_extraData = nullptr;
	_objectTypeMP4 = 0;
}

QuickTimeParser::SampleDesc::~SampleDesc() {
	delete _extraData;
}

QuickTimeParser::Track::Track() {
	chunkCount = 0;
	chunkOffsets = nullptr;
	timeToSampleCount = 0;
	timeToSample = nullptr;
	sampleToChunkCount = 0;
	sampleToChunk = nullptr;
	sampleSize = 0;
	sampleCount = 0;
	sampleSizes = nullptr;
	keyframeCount = 0;
	keyframes = nullptr;
	timeScale = 0;
	width = 0;
	height = 0;
	codecType = CODEC_TYPE_MOV_OTHER;
	frameCount = 0;
	duration = 0;
	mediaDuration = 0;
	nlvlFrom = -1;
	nlvlTo = -1;
	graphicsMode = GraphicsMode::COPY;
	opcolor[0] = opcolor[1] = opcolor[2] = 0;
}

QuickTimeParser::Track::~Track() {
	delete[] chunkOffsets;
	delete[] timeToSample;
	delete[] sampleToChunk;
	delete[] sampleSizes;
	delete[] keyframes;

	for (uint32 i = 0; i < sampleDescs.size(); i++)
		delete sampleDescs[i];
}

} // End of namespace Video
