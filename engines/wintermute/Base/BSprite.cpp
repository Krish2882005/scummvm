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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBSprite, false)

//////////////////////////////////////////////////////////////////////
CBSprite::CBSprite(CBGame *inGame, CBObject *Owner): CBScriptHolder(inGame) {
	_editorAllFrames = false;
	_owner = Owner;
	setDefaults();
}


//////////////////////////////////////////////////////////////////////
CBSprite::~CBSprite() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::setDefaults() {
	_currentFrame = -1;
	_looping = false;
	_lastFrameTime = 0;
	_filename = NULL;
	_finished = false;
	_changed = false;
	_paused = false;
	_continuous = false;
	_moveX = _moveY = 0;

	_editorMuted = false;
	_editorBgFile = NULL;
	_editorBgOffsetX = _editorBgOffsetY = 0;
	_editorBgAlpha = 0xFF;
	_streamed = false;
	_streamedKeepLoaded = false;

	setName("");

	_precise = true;
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::cleanup() {
	CBScriptHolder::cleanup();

	for (int i = 0; i < _frames.getSize(); i++)
		delete _frames[i];
	_frames.removeAll();

	delete[] _editorBgFile;
	_editorBgFile = NULL;

	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::draw(int x, int y, CBObject *registerOwner, float zoomX, float zoomY, uint32 alpha) {
	GetCurrentFrame(zoomX, zoomY);
	if (_currentFrame < 0 || _currentFrame >= _frames.getSize()) return STATUS_OK;

	// move owner if allowed to
	if (_changed && _owner && _owner->_movable) {
		_owner->_posX += _moveX;
		_owner->_posY += _moveY;
		_owner->afterMove();

		x = _owner->_posX;
		y = _owner->_posY;
	}

	// draw frame
	return display(x, y, registerOwner, zoomX, zoomY, alpha);
}


//////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::loadFile(const char *filename, int lifeTime, TSpriteCacheType cacheType) {
	Common::SeekableReadStream *file = Game->_fileManager->openFile(filename);
	if (!file) {
		Game->LOG(0, "CBSprite::LoadFile failed for file '%s'", filename);
		if (Game->_dEBUG_DebugMode) return loadFile("invalid_debug.bmp", lifeTime, cacheType);
		else return loadFile("invalid.bmp", lifeTime, cacheType);
	} else {
		Game->_fileManager->closeFile(file);
		file = NULL;
	}

	ERRORCODE ret;

	AnsiString ext = PathUtil::getExtension(filename);
	if (StringUtil::startsWith(filename, "savegame:", true) || StringUtil::compareNoCase(ext, "bmp") || StringUtil::compareNoCase(ext, "tga") || StringUtil::compareNoCase(ext, "png") || StringUtil::compareNoCase(ext, "jpg")) {
		CBFrame *frame = new CBFrame(Game);
		CBSubFrame *subframe = new CBSubFrame(Game);
		subframe->setSurface(filename, true, 0, 0, 0, lifeTime, true);
		if (subframe->_surface == NULL) {
			Game->LOG(0, "Error loading simple sprite '%s'", filename);
			ret = STATUS_FAILED;
			delete frame;
			delete subframe;
		} else {
			CBPlatform::setRect(&subframe->_rect, 0, 0, subframe->_surface->getWidth(), subframe->_surface->getHeight());
			frame->_subframes.add(subframe);
			_frames.add(frame);
			_currentFrame = 0;
			ret = STATUS_OK;
		}
	} else {
		byte *buffer = Game->_fileManager->readWholeFile(filename);
		if (buffer) {
			if (DID_FAIL(ret = loadBuffer(buffer, true, lifeTime, cacheType))) Game->LOG(0, "Error parsing SPRITE file '%s'", filename);
			delete [] buffer;
		}
	}

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);


	return ret;
}



TOKEN_DEF_START
TOKEN_DEF(CONTINUOUS)
TOKEN_DEF(SPRITE)
TOKEN_DEF(LOOPING)
TOKEN_DEF(FRAME)
TOKEN_DEF(NAME)
TOKEN_DEF(PRECISE)
TOKEN_DEF(EDITOR_MUTED)
TOKEN_DEF(STREAMED_KEEP_LOADED)
TOKEN_DEF(STREAMED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_BG_FILE)
TOKEN_DEF(EDITOR_BG_OFFSET_X)
TOKEN_DEF(EDITOR_BG_OFFSET_Y)
TOKEN_DEF(EDITOR_BG_ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::loadBuffer(byte *buffer, bool complete, int lifeTime, TSpriteCacheType cacheType) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(CONTINUOUS)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(LOOPING)
	TOKEN_TABLE(FRAME)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(PRECISE)
	TOKEN_TABLE(EDITOR_MUTED)
	TOKEN_TABLE(STREAMED_KEEP_LOADED)
	TOKEN_TABLE(STREAMED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_BG_FILE)
	TOKEN_TABLE(EDITOR_BG_OFFSET_X)
	TOKEN_TABLE(EDITOR_BG_OFFSET_Y)
	TOKEN_TABLE(EDITOR_BG_ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	cleanup();


	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SPRITE) {
			Game->LOG(0, "'SPRITE' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int frameCount = 1;
	CBFrame *frame;
	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_CONTINUOUS:
			parser.scanStr((char *)params, "%b", &_continuous);
			break;

		case TOKEN_EDITOR_MUTED:
			parser.scanStr((char *)params, "%b", &_editorMuted);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_LOOPING:
			parser.scanStr((char *)params, "%b", &_looping);
			break;

		case TOKEN_PRECISE:
			parser.scanStr((char *)params, "%b", &_precise);
			break;

		case TOKEN_STREAMED:
			parser.scanStr((char *)params, "%b", &_streamed);
			if (_streamed && lifeTime == -1) {
				lifeTime = 500;
				cacheType = CACHE_ALL;
			}
			break;

		case TOKEN_STREAMED_KEEP_LOADED:
			parser.scanStr((char *)params, "%b", &_streamedKeepLoaded);
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_EDITOR_BG_FILE:
			if (Game->_editorMode) {
				delete[] _editorBgFile;
				_editorBgFile = new char[strlen((char *)params) + 1];
				if (_editorBgFile) strcpy(_editorBgFile, (char *)params);
			}
			break;

		case TOKEN_EDITOR_BG_OFFSET_X:
			parser.scanStr((char *)params, "%d", &_editorBgOffsetX);
			break;

		case TOKEN_EDITOR_BG_OFFSET_Y:
			parser.scanStr((char *)params, "%d", &_editorBgOffsetY);
			break;

		case TOKEN_EDITOR_BG_ALPHA:
			parser.scanStr((char *)params, "%d", &_editorBgAlpha);
			_editorBgAlpha = MIN(_editorBgAlpha, 255);
			_editorBgAlpha = MAX(_editorBgAlpha, 0);
			break;

		case TOKEN_FRAME: {
			int FrameLifeTime = lifeTime;
			if (cacheType == CACHE_HALF && frameCount % 2 != 1) FrameLifeTime = -1;

			frame = new CBFrame(Game);

			if (DID_FAIL(frame->loadBuffer(params, FrameLifeTime, _streamedKeepLoaded))) {
				delete frame;
				Game->LOG(0, "Error parsing frame %d", frameCount);
				return STATUS_FAILED;
			}

			_frames.add(frame);
			frameCount++;
			if (_currentFrame == -1) _currentFrame = 0;
		}
		break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SPRITE definition");
		return STATUS_FAILED;
	}
	_canBreak = !_continuous;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
void CBSprite::reset() {
	if (_frames.getSize() > 0) _currentFrame = 0;
	else _currentFrame = -1;

	killAllSounds();

	_lastFrameTime = 0;
	_finished = false;
	_moveX = _moveY = 0;
}


//////////////////////////////////////////////////////////////////////
bool CBSprite::GetCurrentFrame(float zoomX, float zoomY) {
	//if(_owner && _owner->_freezable && Game->_state == GAME_FROZEN) return true;

	if (_currentFrame == -1) return false;

	uint32 timer;
	if (_owner && _owner->_freezable) timer = Game->_timer;
	else timer = Game->_liveTimer;

	int lastFrame = _currentFrame;

	// get current frame
	if (!_paused && !_finished && timer >= _lastFrameTime + _frames[_currentFrame]->_delay && _lastFrameTime != 0) {
		if (_currentFrame < _frames.getSize() - 1) {
			_currentFrame++;
			if (_continuous) _canBreak = (_currentFrame == _frames.getSize() - 1);
		} else {
			if (_looping) {
				_currentFrame = 0;
				_canBreak = true;
			} else {
				_finished = true;
				_canBreak = true;
			}
		}

		_lastFrameTime = timer;
	}

	_changed = (lastFrame != _currentFrame || (_looping && _frames.getSize() == 1));

	if (_lastFrameTime == 0) {
		_lastFrameTime = timer;
		_changed = true;
		if (_continuous) _canBreak = (_currentFrame == _frames.getSize() - 1);
	}

	if (_changed) {
		_moveX = _frames[_currentFrame]->_moveX;
		_moveY = _frames[_currentFrame]->_moveY;

		if (zoomX != 100 || zoomY != 100) {
			_moveX = (int)((float)_moveX * (float)(zoomX / 100.0f));
			_moveY = (int)((float)_moveY * (float)(zoomY / 100.0f));
		}
	}

	return _changed;
}


//////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::display(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode) {
	if (_currentFrame < 0 || _currentFrame >= _frames.getSize()) return STATUS_OK;

	// on change...
	if (_changed) {
		if (_frames[_currentFrame]->_killSound) {
			killAllSounds();
		}
		applyEvent("FrameChanged");
		_frames[_currentFrame]->oneTimeDisplay(_owner, Game->_editorMode && _editorMuted);
	}

	// draw frame
	return _frames[_currentFrame]->draw(X - Game->_offsetX, Y - Game->_offsetY, Register, ZoomX, ZoomY, _precise, Alpha, _editorAllFrames, Rotate, BlendMode);
}


//////////////////////////////////////////////////////////////////////////
CBSurface *CBSprite::getSurface() {
	// only used for animated textures for 3D models
	if (_currentFrame < 0 || _currentFrame >= _frames.getSize()) return NULL;
	CBFrame *Frame = _frames[_currentFrame];
	if (Frame && Frame->_subframes.getSize() > 0) {
		CBSubFrame *Subframe = Frame->_subframes[0];
		if (Subframe) return Subframe->_surface;
		else return NULL;
	} else return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBSprite::getBoundingRect(Common::Rect *rect, int x, int y, float scaleX, float scaleY) {
	if (!rect) return false;

	CBPlatform::setRectEmpty(rect);
	for (int i = 0; i < _frames.getSize(); i++) {
		Common::Rect frame;
		Common::Rect temp;
		CBPlatform::copyRect(&temp, rect);
		_frames[i]->getBoundingRect(&frame, x, y, scaleX, scaleY);
		CBPlatform::unionRect(rect, &temp, &frame);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "SPRITE {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "LOOPING=%s\n", _looping ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "CONTINUOUS=%s\n", _continuous ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PRECISE=%s\n", _precise ? "TRUE" : "FALSE");
	if (_streamed) {
		buffer->putTextIndent(indent + 2, "STREAMED=%s\n", _streamed ? "TRUE" : "FALSE");

		if (_streamedKeepLoaded)
			buffer->putTextIndent(indent + 2, "STREAMED_KEEP_LOADED=%s\n", _streamedKeepLoaded ? "TRUE" : "FALSE");
	}

	if (_editorMuted)
		buffer->putTextIndent(indent + 2, "EDITOR_MUTED=%s\n", _editorMuted ? "TRUE" : "FALSE");

	if (_editorBgFile) {
		buffer->putTextIndent(indent + 2, "EDITOR_BG_FILE=\"%s\"\n", _editorBgFile);
		buffer->putTextIndent(indent + 2, "EDITOR_BG_OFFSET_X=%d\n", _editorBgOffsetX);
		buffer->putTextIndent(indent + 2, "EDITOR_BG_OFFSET_Y=%d\n", _editorBgOffsetY);
		buffer->putTextIndent(indent + 2, "EDITOR_BG_ALPHA=%d\n", _editorBgAlpha);
	}

	CBScriptHolder::saveAsText(buffer, indent + 2);

	int i;

	// scripts
	for (i = 0; i < _scripts.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}


	for (i = 0; i < _frames.getSize(); i++) {
		_frames[i]->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::persist(CBPersistMgr *persistMgr) {
	CBScriptHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_canBreak));
	persistMgr->transfer(TMEMBER(_changed));
	persistMgr->transfer(TMEMBER(_paused));
	persistMgr->transfer(TMEMBER(_continuous));
	persistMgr->transfer(TMEMBER(_currentFrame));
	persistMgr->transfer(TMEMBER(_editorAllFrames));
	persistMgr->transfer(TMEMBER(_editorBgAlpha));
	persistMgr->transfer(TMEMBER(_editorBgFile));
	persistMgr->transfer(TMEMBER(_editorBgOffsetX));
	persistMgr->transfer(TMEMBER(_editorBgOffsetY));
	persistMgr->transfer(TMEMBER(_editorMuted));
	persistMgr->transfer(TMEMBER(_finished));

	_frames.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_lastFrameTime));
	persistMgr->transfer(TMEMBER(_looping));
	persistMgr->transfer(TMEMBER(_moveX));
	persistMgr->transfer(TMEMBER(_moveY));
	persistMgr->transfer(TMEMBER(_owner));
	persistMgr->transfer(TMEMBER(_precise));
	persistMgr->transfer(TMEMBER(_streamed));
	persistMgr->transfer(TMEMBER(_streamedKeepLoaded));


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetFrame") == 0) {
		stack->correctParams(1);
		int Index = stack->pop()->getInt(-1);
		if (Index < 0 || Index >= _frames.getSize()) {
			script->runtimeError("Sprite.GetFrame: Frame index %d is out of range.", Index);
			stack->pushNULL();
		} else stack->pushNative(_frames[Index], true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteFrame") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		if (Val->isInt()) {
			int Index = Val->getInt(-1);
			if (Index < 0 || Index >= _frames.getSize()) {
				script->runtimeError("Sprite.DeleteFrame: Frame index %d is out of range.", Index);
			}
		} else {
			CBFrame *Frame = (CBFrame *)Val->getNative();
			for (int i = 0; i < _frames.getSize(); i++) {
				if (_frames[i] == Frame) {
					if (i == _currentFrame) _lastFrameTime = 0;
					delete _frames[i];
					_frames.removeAt(i);
					break;
				}
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Reset") == 0) {
		stack->correctParams(0);
		reset();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddFrame") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL()) filename = val->getString();

		CBFrame *frame = new CBFrame(Game);
		if (filename != NULL) {
			CBSubFrame *sub = new CBSubFrame(Game);
			if (DID_SUCCEED(sub->setSurface(filename))) {
				sub->setDefaultRect();
				frame->_subframes.add(sub);
			} else delete sub;
		}
		_frames.add(frame);

		stack->pushNative(frame, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertFrame") == 0) {
		stack->correctParams(2);
		int index = stack->pop()->getInt();
		if (index < 0) 
			index = 0;

		CScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL())
			filename = val->getString();

		CBFrame *frame = new CBFrame(Game);
		if (filename != NULL) {
			CBSubFrame *sub = new CBSubFrame(Game);
			if (DID_SUCCEED(sub->setSurface(filename))) frame->_subframes.add(sub);
			else delete sub;
		}

		if (index >= _frames.getSize())
			_frames.add(frame);
		else _frames.insertAt(index, frame);

		stack->pushNative(frame, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pause") == 0) {
		stack->correctParams(0);
		_paused = true;
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Play
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Play") == 0) {
		stack->correctParams(0);
		_paused = false;
		stack->pushNULL();
		return STATUS_OK;
	}

	else return CBScriptHolder::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBSprite::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("sprite");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFrames (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumFrames") == 0) {
		_scValue->setInt(_frames.getSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CurrentFrame") == 0) {
		_scValue->setInt(_currentFrame);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PixelPerfect") == 0) {
		_scValue->setBool(_precise);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Looping") == 0) {
		_scValue->setBool(_looping);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Owner (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Owner") == 0) {
		if (_owner == NULL) _scValue->setNULL();
		else _scValue->setNative(_owner, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Finished (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Finished") == 0) {
		_scValue->setBool(_finished);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Paused (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Paused") == 0) {
		_scValue->setBool(_paused);
		return _scValue;
	}

	else return CBScriptHolder::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "CurrentFrame") == 0) {
		_currentFrame = value->getInt(0);
		if (_currentFrame >= _frames.getSize() || _currentFrame < 0) {
			_currentFrame = -1;
		}
		_lastFrameTime = 0;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PixelPerfect") == 0) {
		_precise = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Looping") == 0) {
		_looping = value->getBool();
		return STATUS_OK;
	}

	else return CBScriptHolder::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBSprite::scToString() {
	return "[sprite]";
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSprite::killAllSounds() {
	for (int i = 0; i < _frames.getSize(); i++) {
		if (_frames[i]->_sound)
			_frames[i]->_sound->stop();
	}
	return STATUS_OK;
}

} // end of namespace WinterMute
