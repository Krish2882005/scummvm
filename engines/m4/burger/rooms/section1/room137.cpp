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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/burger/rooms/section1/room137.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "PATROL CAR",       "137W002", "137W003", "137W004" },
	{ "TRUNK",            nullptr,   "137W008", "137W008" },
	{ "JAWZ O' LIFE",     "137W010", nullptr,   nullptr   },
	{ "SIGN",             "137W012", "137W008", "137W008" },
	{ "CAR WINDOW",       nullptr,   "137W008", "137W008" },
	{ "HIGHWAY 2",        "137W013", "137W008", "137W008" },
	{ "FORK IN THE ROAD", nullptr,   "137W008", "137W008" },
	{ "KEYS",             "137W009", nullptr,   nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesStreamBreak SERIES1[] = {
	{  9, "100_010", 1, 255, -1, 0, 0, 0 },
	{ 20, nullptr,   1, 255, 12, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY1[] = {
	{  0, 24, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 25, 32, "137_005", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 33, 33, nullptr,   1, 255, 16, 0, 0, nullptr, 0 },
	{ 34, 40, nullptr,   1, 255, 22, 0, 0, nullptr, 0 },
	{ 41, -1, nullptr,   1, 255, 19, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0,  7, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  8,  8, nullptr,   1, 255, 17, 0, 0, nullptr, 0 },
	{  9, 13, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 14, 14, nullptr,   1, 255, 21, 0, 0, nullptr, 0 },
	{ 14, 14, "137_006", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 15, -1, nullptr,   1, 255, 18, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  0, 11, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, nullptr,   1, 255, 20, 0, 0, nullptr, 0 },
	{ 13, -1, "137_007", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room137::init() {
	_G(player).walker_in_this_scene = true;
	player_set_commands_allowed(true);
	_G(kernel).call_daemon_every_loop = true;
	_volume = 255;
	_flag1 = true;
	_flag2 = false;
	_flag3 = false;

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		if (_G(flags)[V048])
			_G(flags)[V048] = 1;
		break;

	case 136:
		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		kernel_timing_trigger(1, gCHANGE_WILBUR_ANIMATION);
		break;

	case 138:
		ws_demand_facing(2);

		if (_G(flags)[V048] >= 200) {
			ws_demand_location(264, 347);
			_flag1 = false;
			digi_preload("137_003");
		} else {
			ws_demand_location(290, 334);
			ws_hide_walker();
			player_set_commands_allowed(false);
			_G(wilbur_should) = 1;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}

		if (_G(flags)[V048] < 200 && _G(flags)[V048] && _G(flags)[V047] == 4)
			_G(flags)[V047] = 5;
		break;

	default:
		ws_demand_location(183, 216, 8);
		break;
	}

	_series1 = series_play("137do01", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	const char *NAMES[18] = {
		"137_120", "137_021", "137_022", "137_023", "137_024", "137_025",
		"137_026", "137_027", "137_028", "137_013", "137_014", "137_026",
		"137_027", "137_028", "137_013", "137_014", "137_015", "137_016"
	};
	for (int i = 0; i < 18; ++i)
		digi_preload(NAMES[i]);

	if (inv_object_in_scene("keys", 138) && _G(flags)[V047] != 2) {
		digi_preload("137_001");
		digi_play_loop("137_001", 3);
	} else {
		digi_preload("137_002");
		digi_play_loop("137_002", 3);
	}

	if (_G(flags)[V048] < 200) {
		_mode1 = 27;
	} else {
		_mode1 = _G(flags)[V047] == 2 || _G(flags)[V047] == 3 || _G(flags)[V047] == 4 ? 34 : 27;
		digi_play("137_003", 1);
	}

	_mode2 = 27;
	kernel_trigger_dispatch_now(2);
}

void Room137::daemon() {
	// TODO
}

void Room137::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V046] && (player_said("gear", "trunk") || player_said("gear", "trunk "))) {
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("conv15")) {
		conv15();

	} else if (_G(walker).wilbur_said(SAID)) {
		// Nothing needed
	} else if (player_said("LOOK AT", "TRUNK") || player_said("LOOK AT", "TRUNK ")) {
		if (!_G(flags)[V046]) {
			wilbur_speech("137w005");
		} else {
			wilbur_speech(inv_object_is_here("JAWZ O' LIFE") ? "137w006" : "137w007");
		}
	} else if (player_said("keys", "trunk")) {
		_G(wilbur_should) = 6;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("take", "jawz o' life") && inv_object_is_here("jawz o' life")) {
		_G(wilbur_should) = 8;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if ((player_said("take", "keys") || player_said("gear", "keys")) &&
			!inv_player_has("keys")) {
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("gear", "jawz o' life") && inv_object_is_here("jawz o' life")) {
		wilbur_speech("137w011");

	} else if (player_said("ENTER", "FORK IN THE ROAD") ||
			player_said("look at", "FORK IN THE ROAD")) {
		player_set_commands_allowed(false);
		pal_fade_init(1009);

	} else if (player_said("LOOK AT", "CAR WINDOW")) {
		_G(wilbur_should) = 35;
		player_set_commands_allowed(false);
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room137::conv15() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 14) {
		if (who <= 0) {
			if (node == 7) {
				_mode3 = 25;
			} else {
				_mode3 = 22;
				conv_resume_curr();
			}
		} else if (who == 1) {
			conv_resume_curr();
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 3 || node == 9 || node == 12 || node == 13 || node == 19 || node == 11)
				_flag3 = true;
			_mode3 = (node == 20 && entry == 1) || (node == 21 && entry == 1) ? 24 : 23;
			_digi1 = conv_sound_to_play();

		} else if (who == 1) {
			wilbur_speech(conv_sound_to_play(), 14);
		}
	} else {
		conv_resume_curr();
	}
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
