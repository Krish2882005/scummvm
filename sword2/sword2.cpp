/* Copyright (C) 1994-2003 Revolution Software Ltd
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
#include "backends/fs/fs.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "sword2/sword2.h"
#include "sword2/driver/driver96.h"
#include "sword2/defs.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/credits.h"
#include "sword2/debug.h"
#include "sword2/header.h"
#include "sword2/interpreter.h"
#include "sword2/layers.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/protocol.h"
#include "sword2/resman.h"
#include "sword2/save_rest.h"
#include "sword2/sound.h"
#include "sword2/speech.h"
#include "sword2/startup.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

extern uint16 _debugLevel;

static const GameSettings sword2_settings[] = {
	/* Broken Sword 2 */
	{"sword2", "Broken Sword II", MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "players.clu" },
	{"sword2alt", "Broken Sword II (alt)", MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER, "r2ctlns.ocx" },
	{"sword2demo", "Broken Sword II (Demo)", MDT_ADLIB | MDT_NATIVE, GF_DEFAULT_TO_1X_SCALER | Sword2::GF_DEMO, "players.clu" },
	{NULL, NULL, MDT_NONE, 0, NULL}
};

GameList Engine_SWORD2_gameList() {
	const GameSettings *g = sword2_settings;
	GameList games;
	while (g->gameName)
		games.push_back(*g++);
	return games;
}

GameList Engine_SWORD2_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const GameSettings *g;
	
	// TODO: It would be nice if we had code here which distinguishes between
	// the 'sword2' and 'sword2demo' targets. The current code can't do that
	// since they use the same detectname.

	for (g = sword2_settings; g->gameName; ++g) {
		// Iterate over all files in the given directory
		for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(*g);
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst) {
	return new Sword2::Sword2Engine(detector, syst);
}

REGISTER_PLUGIN("Broken Sword II", Engine_SWORD2_gameList, Engine_SWORD2_create, Engine_SWORD2_detectGames);

namespace Sword2 {

Sword2Engine *g_sword2 = NULL;
Input *g_input = NULL;
Sound *g_sound = NULL;
Graphics *g_graphics = NULL;

Sword2Engine::Sword2Engine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	g_sword2 = this;
	_debugger = NULL;
	_features = detector->_game.features;
	_targetName = strdup(detector->_targetName.c_str());
	_bootParam = ConfMan.getInt("boot_param");
	_saveSlot = ConfMan.getInt("save_slot");
	_debugLevel = ConfMan.getInt("debuglevel");

	// Setup mixer
	if (!_mixer->bindToSystem(syst))
		warning("Sound initialization failed");

	// We have our own volume settings panel, so don't let ScummVM's mixer
	// soften the sound in any way.

	_mixer->setVolume(256);
	_mixer->setMusicVolume(256);

	// get some falling RAM and put it in your pocket, never let it slip
	// away

	memory = new MemoryManager();
	res_man = new ResourceManager(this);
	_logic = new Logic(this);
	fontRenderer = new FontRenderer();
	gui = new Gui(this);
	g_input = _input = new Input();
	g_sound = _sound = new Sound(_mixer);
	g_graphics = _graphics = new Graphics(640, 480);
	_debugger = new Debugger(this);

	_lastPaletteRes = 0;

	_largestLayerArea = 0;
	_largestSpriteArea = 0;

	strcpy(_largestLayerInfo,  "largest layer:  none registered");
	strcpy(_largestSpriteInfo, "largest sprite: none registered");

	_fps = 0;
	_cycleTime = 0;
	_frameCount = 0;

	_wantSfxDebug = false;
	_grabbingSequences = false;

	// For the menus

	_totalTemp = 0;
	memset(_tempList, 0, sizeof(_tempList));

	_totalMasters = 0;
	memset(_masterMenuList, 0, sizeof(_masterMenuList));

	memset(&_thisScreen, 0, sizeof(_thisScreen));

	memset(_mouseList, 0, sizeof(_mouseList));

	_mouseTouching = 0;
	_oldMouseTouching = 0;
	_menuSelectedPos = 0;
	_examiningMenuIcon = false;
	_mousePointerRes = 0;
	_mouseMode = 0;
	_mouseStatus = false;
	_mouseModeLocked = false;
	_currentLuggageResource = 0;
	_oldButton = 0;
	_buttonClick = 0;
	_pointerTextBlocNo = 0;
	_playerActivityDelay = 0;
	_realLuggageItem = 0;

	// used to be a define, but now it's flexible
	_scrollFraction = 16;

	_gamePaused = false;
	_stepOneCycle = false;
	_graphicsLevelFudged = false;
}

Sword2Engine::~Sword2Engine() {
	free(_targetName);
	delete _debugger;
	delete _graphics;
	delete _sound;
	delete _input;
	delete gui;
	delete fontRenderer;
	delete _logic;
	delete res_man;
	delete memory;
}

void Sword2Engine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the debugger. Otherwise
	// exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		printf("%s\n", buf2);	// (Print it again in case debugger segfaults)
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

int32 Sword2Engine::InitialiseGame(void) {
	// init engine drivers

	uint8 *file;

	// initialise global script variables
	// res 1 is the globals list
	file = res_man->openResource(1);
	debug(5, "CALLING: SetGlobalInterpreterVariables");
	_logic->setGlobalInterpreterVariables((int32 * ) (file + sizeof(_standardHeader)));

	// DON'T CLOSE VARIABLES RESOURCE - KEEP IT OPEN AT VERY START OF
	// MEMORY SO IT CAN'T MOVE!

	// DON'T CLOSE PLAYER OBJECT RESOURCE - KEEP IT OPEN IN MEMORY SO IT
	// CAN'T MOVE!

	file = res_man->openResource(8);

	// Set up font resource variables for this language version

	debug(5, "CALLING: initialiseFontResourceFlags");
	initialiseFontResourceFlags();

	// initialise the sound fx queue

	debug(5, "CALLING: Init_fx_queue");
	initFxQueue();

	// all demos (not just web)
	if (_features & GF_DEMO) {
		// set script variable
		DEMO = 1;
	}

	return 0;
}

void Sword2Engine::closeGame(void) {
	// Stop music instantly!
	killMusic();
	g_system->quit();
}

void Sword2Engine::gameCycle(void) {
	// do one game cycle

	// got a screen to run?
	if (_logic->getRunList()) {
		//run the logic session UNTIL a full loop has been performed
		do {
			// reset the graphic 'buildit' list before a new
			// logic list (see fnRegisterFrame)
			resetRenderLists();

			// reset the mouse hot-spot list (see fnRegisterMouse
			// and fnRegisterFrame)
			resetMouseList();

			// keep going as long as new lists keep getting put in
			// - i.e. screen changes
		} while (_logic->processSession());
	} else {
		// start the console and print the start options perhaps?
		_debugger->attach("AWAITING START COMMAND: (Enter 's 1' then 'q' to start from beginning)");
	}

	// if this screen is wide, recompute the scroll offsets every cycle
	if (_thisScreen.scroll_flag)
		setScrolling();

	mouseEngine();
	processFxQueue();

	// update age and calculate previous cycle memory usage
	res_man->nextCycle();
}

void Sword2Engine::go() {
	_keyboardEvent ke;

	// Call the application "Revolution" until the resource manager is
	// ready to dig the name out of a text file. See InitialiseGame()
	// which calls InitialiseFontResourceFlags() in maketext.cpp
	//
	// Have to do it like this since we cannot really fire up the resource
	// manager until a window has been created as any errors are displayed
	// via a window, thus time becomes a loop.

	debug(5, "CALLING: readOptionSettings");
	gui->readOptionSettings();

	debug(5, "CALLING: InitialiseGame");
	if (InitialiseGame()) {
		closeGame();
		return;
	}

	if (_saveSlot != -1) {
		if (saveExists(_saveSlot))
			restoreGame(_saveSlot);
		else { // show restore menu
			setMouse(NORMAL_MOUSE_ID);
			if (!gui->restoreControl())
				startGame();
		}
	} else
		startGame();

	debug(5, "CALLING: initialiseRenderCycle");
	_graphics->initialiseRenderCycle();

	_renderSkip = false;		// Toggled on 'S' key, to render only
					// 1 in 4 frames, to speed up game

	_gameCycle = 0;

	while (1) {
		if (_debugger->isAttached())
			_debugger->onFrame();

		_graphics->updateDisplay();

#ifdef _SWORD2_DEBUG
// FIXME: If we want this, we should re-work it to use the backend's
// screenshot functionality.
//		if (_debugger->_grabbingSequences && !console_status)
//			GrabScreenShot();
#endif

		// the screen is build. Mostly because of first scroll
		// cycle stuff

#ifdef _SWORD2_DEBUG
		// if we've just stepped forward one cycle while the
		// game was paused

		if (_stepOneCycle) {
			pauseGame();
			_stepOneCycle = false;
		}
#endif

		if (g_input->keyWaiting()) {
			g_input->readKey(&ke);

			char c = toupper(ke.ascii);

			if (ke.modifiers == OSystem::KBD_CTRL) {
				if (ke.keycode == 'd') {
					_debugger->attach();
				}
			}

			if (c == '~' || c == '#')
				_debugger->attach();

			if (_gamePaused) {	// if currently paused
				if (c == 'P') {
					// 'P' while paused = unpause!
					unpauseGame();
				}
#ifdef _SWORD2_DEBUG
				// frame-skipping only allowed on
				// debug version

				else if (c == ' ') {
					// SPACE bar while paused =
					// step one frame!
					_stepOneCycle = true;
					unpauseGame();
				}
#endif
			} else if (c == 'P') {
				// 'P' while not paused = pause!
				pauseGame();
			} else if (c == 'C' && !(_features & GF_DEMO)) {
				_logic->fnPlayCredits(NULL);
			}
#ifdef _SWORD2_DEBUG
			else if (c == 'S') {
				// 'S' toggles speed up (by skipping
				// display rendering)
				_renderSkip = !_renderSkip;
			}
#endif
		}

		// skip GameCycle if we're paused
		if (!_gamePaused) {
#ifdef _SWORD2_DEBUG
			_gameCycle++;
#endif
			gameCycle();
		}

		// creates the debug text blocks
		_debugger->buildDebugText();

#ifdef _SWORD2_DEBUG
		// if not in console & '_renderSkip' is set, only render
		// display once every 4 game-cycles

		if (console_status || !_renderSkip || (_gameCycle % 4) == 0)
			buildDisplay();	// create and flip the screen
#else
		// create and flip the screen
		buildDisplay();
#endif
	}

	closeGame();		// close engine systems down
}

void Sword2Engine::startGame(void) {
	// boot the game straight into a start script

	int screen_manager_id;

	debug(5, "startGame() STARTING:");

	// all demos not just web
	if (_features & GF_DEMO)
		screen_manager_id = 19;		// DOCKS SECTION START
	else
		screen_manager_id = 949;	// INTRO & PARIS START

	// FIXME this could be validated against startup.inf for valid numbers
	// to stop people shooting themselves in the foot

	if (_bootParam != 0)
		screen_manager_id = _bootParam;
	
	char *raw_script;
	char *raw_data_ad;

	// the required start-scripts are both script #1 in the respective
	// ScreenManager objects

	uint32 null_pc = 1;

	// open george object, ready for start script to reference
	raw_data_ad = (char *) res_man->openResource(8);

	// open the ScreenManager object
	raw_script = (char *) res_man->openResource(screen_manager_id);

	// run the start script now (because no console)
	_logic->runScript(raw_script, raw_data_ad, &null_pc);

	// close the ScreenManager object
	res_man->closeResource(screen_manager_id);

	// close george
	res_man->closeResource(8);

	debug(5, "startGame() DONE.");
}

// FIXME: Move this to some better place?

void Sword2Engine::sleepUntil(uint32 time) {
	while (_system->get_msecs() < time) {
		// Make sure menu animations and fades don't suffer
		_graphics->processMenu();
		_graphics->updateDisplay();

		_system->delay_msecs(10);
	}
}

void Sword2Engine::pauseGame(void) {
	// uint8 *text;

	// open text file & get the line "PAUSED"
	// text = FetchTextLine(res_man->openResource(3258), 449);
	// pause_text_bloc_no = Build_new_block(text + 2, 320, 210, 640, 184, RDSPR_TRANS | RDSPR_DISPLAYALIGN, SPEECH_FONT_ID, POSITION_AT_CENTRE_OF_BASE);
	// now ok to close the text file
	// res_man->closeResource(3258);

	// don't allow Pause while screen fading or while black
	if (_graphics->getFadeStatus() != RDFADE_NONE)
		return;
	
	pauseAllSound();

	// make a normal mouse
	clearPointerText();

	// mouse_mode=MOUSE_normal;

	// this is the only place allowed to do it this way
	_graphics->setLuggageAnim(NULL, 0);

	// blank cursor
	setMouse(0);

	// forces engine to choose a cursor
	_mouseTouching = 1;

	// if level at max, turn down because palette-matching won't work
	// when dimmed

	if (gui->_currentGraphicsLevel == 3) {
		gui->updateGraphicsLevel(2);
		_graphicsLevelFudged = true;
	}

	// don't dim it if we're single-stepping through frames
	// dim the palette during the pause

	if (!_stepOneCycle)
		_graphics->dimPalette();

	_gamePaused = true;
}

void Sword2Engine::unpauseGame(void) {
	// removed "PAUSED" from screen
	// Kill_text_bloc(pause_text_bloc_no);

	if (OBJECT_HELD && _realLuggageItem)
		setLuggage(_realLuggageItem);

	unpauseAllSound();

	// put back game screen palette; see Build_display.cpp
	setFullPalette(0xffffffff);

	// If graphics level at max, turn up again
	if (_graphicsLevelFudged) {
		gui->updateGraphicsLevel(3);
		_graphicsLevelFudged = false;
	}

	_gamePaused = false;
	_logic->_unpauseZone = 2;

	// if mouse is about or we're in a chooser menu
	if (!_mouseStatus || _logic->_choosing)
		setMouse(NORMAL_MOUSE_ID);
}

} // End of namespace Sword2
