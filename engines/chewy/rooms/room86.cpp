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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/rooms/room86.h"

namespace Chewy {
namespace Rooms {

void Room86::entry(int16 eib_nr) {
	_G(spieler).ScrollxStep = 2;
	SetUpScreenFunc = setup_func;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(spieler).ZoomXy[P_HOWARD][0] = 20;
	_G(spieler).ZoomXy[P_HOWARD][1] = 20;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 20;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 24;
	_G(zoom_horizont) = 110;
	_G(spieler).DiaAMov = 0;
	if (_G(spieler).flags32_2) {
		det->start_detail(0, 255, false);
		det->set_static_pos(0, 352, 107, false, false);
		det->show_static_spr(0);
	}

	if (flags.LoadGame)
		return;

	if (eib_nr == 127) {
		set_person_pos(82, 56, P_HOWARD, P_RIGHT);
		set_person_pos(134, 56, P_NICHELLE, P_LEFT);
		return;
	}

	if (!_G(spieler).flags32_10) {
		set_person_pos(260, 66, P_CHEWY, P_RIGHT);
		set_person_pos(298, 44, P_HOWARD, P_LEFT);
		set_person_pos(320, 42, P_NICHELLE, P_LEFT);
		_G(spieler).scrollx = 164;
	} else {
		spieler_mi[P_CHEWY].Vorschub = 16;
		hide_cur();
		flags.ZoomMov = false;
		_G(spieler).scrollx = 246;
		_G(spieler).ScrollxStep = 8;
		set_person_pos(443, 66, P_CHEWY, P_RIGHT);
		_G(spieler).PersonRoomNr[P_HOWARD] = 84;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 0;
		auto_move(2, P_CHEWY);
		flags.NoScroll = true;
		_G(spieler.ScrollxStep = 2);
		auto_scroll(30, 0);
		start_spz_wait(13, 1, false, P_CHEWY);
		flags.NoScroll = false;
		spieler_mi[P_CHEWY].Vorschub = 8;
		det->stop_detail(0);
		det->show_static_spr(4);
		det->show_static_spr(5);
		invent_2_slot(94);
		auto_move(4, P_CHEWY);
		flags.NoScroll = true;
		auto_scroll(246, 0);
		proc3(false);
		flic_cut(92, CFO_MODE);
		flags.NoScroll = false;
		auto_move(0, P_CHEWY);
		_G(spieler).flags32_20 = true;
		switch_room(85);
		show_cur();
	}
}

void Room86::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;
	switch (eib_nr) {
	case 128:
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 86)
			_G(spieler).PersonRoomNr[P_HOWARD] = 85;
		
		if (_G(spieler).PersonRoomNr[P_NICHELLE] == 86)
			_G(spieler).PersonRoomNr[P_NICHELLE] = 85;
		break;
	case 132:
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 86)
			_G(spieler).PersonRoomNr[P_HOWARD] = 87;

		if (_G(spieler).PersonRoomNr[P_NICHELLE] == 86)
			_G(spieler).PersonRoomNr[P_NICHELLE] = 87;
		break;
	default:
		break;
	}
}

void Room86::setup_func() {
	calc_person_look();

	int nicDestX;
	int howDestY = 56;
	int howDestX;
	int nicDestY = 56;

	int xyPos = spieler_vector[P_CHEWY].Xypos[0];	
	if (xyPos > 390) {
		howDestX = 298;
		howDestY = 44;
		nicDestX = 320;
		nicDestY = 42;
	} else if (xyPos > 250) {
		howDestX = 216;
		nicDestX = 240;
	} else {
		howDestX = 82;
		nicDestX = 134;
	}

	go_auto_xy(howDestX, howDestY, P_HOWARD, ANI_GO);
	go_auto_xy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
}

int Room86::proc2() {
	if (!is_cur_inventar(94))
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	det->start_detail(0, 255, false);
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	del_inventar(_G(spieler).AkInvent);
	auto_move(3, P_CHEWY);
	proc3(true);
	atds->del_steuer_bit(499, ATS_AKTIV_BIT, ATS_DATEI);
	atds->set_ats_str(497, 1, ATS_DATEI);
	atds->set_ats_str(498, 1, ATS_DATEI);
	_G(spieler).flags32_2 = true;
	_G(spieler).room_e_obj[132].Attribut = 2;
	start_spz(16, 255, false, P_CHEWY);
	start_aad_wait(468, -1);

	show_cur();
	return 1;
}

void Room86::proc3(bool cond) {
	int destY, deltaY;

	if (cond) {
		destY = 199;
		deltaY = -2;
	} else {
		destY = 104;
		deltaY = 2;
	}

	if (flags.NoScroll)
		auto_scroll(196, 0);

	det->set_static_pos(0, 352, destY, false, false);
	det->show_static_spr(0);
	det->enable_sound(0, 1);
	det->enable_sound(0, 2);
	det->play_sound(0, 1);
	det->play_sound(0, 2);

	for (int i = 0; i < 48; ++i) {
		set_up_screen(NO_SETUP);
		det->set_static_pos(0, 352, destY, false, false);
		destY += deltaY;
		out->setze_zeiger(nullptr);
		out->back2screen(workpage);
	}

	det->disable_sound(0, 1);
	det->disable_sound(0, 2);
	flags.NoScroll = false;
}

} // namespace Rooms
} // namespace Chewy
