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

#include "bagel/baglib/dossier_object.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/master_win.h"

namespace Bagel {

CBagDossierObject *CBagDossierObject::m_pCurDossier;

void CBagDossierObject::initialize() {
	m_pCurDossier = nullptr;
}

CBagDossierObject::CBagDossierObject() {
	m_xObjType = DOSSIEROBJ;

	m_bDossierSet = false;
	m_cIndexRect.SetRect(0, 0, 0, 0);

	// Keep track of the original text rect.
	m_cDossierRect.SetRect(0, 0, 0, 0);
	m_bDosRectInit = false;

	SetNotActive(false);

	// Ptr to parent rp obj nullptr for now.
	m_pRPObj = nullptr;

	// Start with index line displayed
	m_bShowIndex = true;
}

CBagDossierObject::~CBagDossierObject() {
}

PARSE_CODES CBagDossierObject::setInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;
	char szLocalStr[256];
	CBofString sStr(szLocalStr, 256);

	while (!istr.eof()) {
		istr.EatWhite();
		char ch = (char)istr.peek();
		switch (ch) {

		//  SIZE n - n point size of the txt
		//
		//  -or-
		//
		// SUSPECTVAR is a variable that will be in the LOGPDA list of
		// objects and will be tightly tied to residue printing.  If a res print
		// yields positive results on a character then set the rp field of the
		// associated suspect object to true.
		case 'S':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SIZE")) {
				istr.EatWhite();
				int n;
				GetIntFromStream(istr, n);
				SetPointSize((byte)n);
				nObjectUpdated = true;
			} else if (!sStr.Find("SUSPECTVAR")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				m_sSuspectVar = sStr;
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  NOTACTIVE KEYWORD MEANS DON'T DO ANYTHING ON MOUSE DOWN!!!
		//
		case 'N':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("NOTACTIVE")) {
				SetNotActive(true);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  FONT MONO or DEFAULT
		//
		case 'F':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FONT")) {
				istr.EatWhite();
				int n;
				GetIntFromStream(istr, n);
				n = MapFont(n);
				SetFont(n);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  INDEX line, required.  This is the line that will show up in the
		//  log entry.
		//
		case 'I':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("INDEX")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);

				// replace underlines with spaces.

				sStr.ReplaceChar('_', ' ');
				m_sIndexLine = sStr;

				nObjectUpdated = true;

				// If the next character is a '[' then we got another rect coming
				// along.  This rect is for the index string.

				istr.EatWhite();
				if ((char)istr.peek() == '[') {
					CBofRect r;
					GetRectFromStream(istr, r);
					m_cIndexRect = r;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

		//
		//  AS [CAPTION]  - how to run the link
		case 'A':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("CAPTION")) {
					m_bCaption = true;
					nObjectUpdated = true;

				} else if (!sStr.Find("TITLE")) {
					m_bTitle = true;
					nObjectUpdated = true;

				} else {
					PutbackStringOnStream(istr, sStr);
					PutbackStringOnStream(istr, "AS ");
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;

			//
			//  COLOR n - n color index
			//
#define CTEXT_YELLOW RGB(255, 255, 0)
#define CTEXT_WHITE RGB(255, 255, 255)
		case 'C':
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("COLOR")) {
				int nColor;
				istr.EatWhite();
				GetIntFromStream(istr, nColor);
				SetColor(nColor);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		//
		//  no match return from funtion
		//
		default:
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
	}

	return PARSING_DONE;
}

// Implement attach and detach just so we can set our own attributes

ErrorCode CBagDossierObject::attach() {
	ErrorCode ec = CBagTextObject::attach();

	// Keep track of the original text rectangle (for the dossier).
	if (m_bDosRectInit == false) {
		m_cDossierRect = CBagTextObject::getRect();
		m_bDosRectInit = true;
	}

	SetVisible(false); // Don't display until needed.
	SetActive(false);  // Not active until we need it.
	return ec;
}

ErrorCode CBagDossierObject::detach() {
	ErrorCode ec = CBagTextObject::detach();

	SetVisible(false); // Make this invisible, don't want it redrawn.
	return ec;
}

CBofPoint CBagDossierObject::ArrangeFloater(CBofPoint nPos, CBagObject * /*pObj*/) {
	CBofPoint cNextPos = nPos;
	return cNextPos;
}

// Called to splash one of these guys to the screen
ErrorCode CBagDossierObject::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int n) {
	if (m_bShowIndex) {
		if (m_bDossierSet == false) {
			// Set the text to be the index line.
			SetPSText(&m_sIndexLine);
			m_bDossierSet = true;
		}
	} else if (m_bDossierSet == false) {
		// Set the text to be nullptr, this forces the bagtx code to
		// paint the text file.
		SetPSText(nullptr);

		CBagTextObject::attach();

		m_bDossierSet = true;
	}

	return CBagTextObject::update(pBmp, pt, pSrcRect, n);
}

CBofRect CBagDossierObject::getRect() {
	CBofRect r;
	CBofPoint p = GetPosition();

	if (m_bShowIndex) {
		r = m_cIndexRect;
		r.OffsetRect(p);
	} else {
		r = m_cDossierRect;
	}

	return r;
}

void CBagDossierObject::OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * /*pInfo*/) {
	// If this is an inactive object (such as "plus residue too old" or "negative")
	// then ignore the mousedown.

	if (GetNotActive()) {
		return;
	}

	ShowDosText();
}

// utility routine for activating dossier objects
void CBagDossierObject::ActivateDosObject(CBagLog *pLogWld) {
	if (pLogWld) {
		pLogWld->ActivateLocalObject(this);
	}

	if (CBagRPObject::GetLogState() == RP_READ_DOSSIER) {
		SetFloating(false); // This is not floating
	} else {
		SetFloating(); // This is not floating
	}

	SetVisible(); // can see this thing now...
	SetActive();  // make sure it counts for something

	// Starting state will ALWAYS be index first
	m_bDossierSet = false;
	m_bShowIndex = true;
}

void CBagDossierObject::DeactivateDosObject(CBagLog *pLogWld) {
	if (CBagRPObject::GetLogState() == RP_READ_DOSSIER) {
		SetFloating(false); // This is not floating
	} else {
		SetFloating(); // This is not floating
	}

	SetVisible(false); // Cover it up
	SetActive(false);  // Make sure it counts for something
	SetLocal(false);   // Not local anymore
}

// Called from script to deactivate a dossier view screen.
void CBagDossierObject::DeactivateCurDossier() {
	// If we have a current dossier, then turn it off and activate the
	// previous rp object.
	if (m_pCurDossier != nullptr) {
		CBagLog *pLogWld = nullptr;
		CBagRPObject *pRPObj = ((CBagRPObject *)(m_pCurDossier->m_pRPObj));

		if (pRPObj) {
			if (pRPObj->Zoomed()) {
				pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
			} else {
				pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
			}
		}
		m_pCurDossier->DeactivateDosObject(pLogWld);

		// When we hit the update code, we're gonna need to reevaluate this
		// guys current view status.
		m_pCurDossier->m_bShowIndex = true;
		m_pCurDossier->m_bDossierSet = false;

		((CBagRPObject *)(m_pCurDossier->m_pRPObj))->ActivateRPObject();

		m_pCurDossier = nullptr;
	}
}

void CBagDossierObject::ShowDosText() {
	// Make sure our script knows where we're going with this.
	CBagRPObject::SetLogState(RP_READ_DOSSIER);

	// We got a mouse down on one of our dossier's, so now we have to deactivate
	// everything that is in the residue print object that this dossier is
	// contained in.
	bool bZoomed = false;
	if (m_pRPObj) {
		bZoomed = ((CBagRPObject *)m_pRPObj)->Zoomed();
		((CBagRPObject *)m_pRPObj)->DeactivateRPObject();
	}

	// Get the appropriate storage device

	CBagLog *pLogWld;
	if (bZoomed) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	// Now reactivate this object.
	ActivateDosObject(pLogWld);

	// Mark this one as our current dossier
	m_pCurDossier = this;

	// Make sure the text file is displayed, not the index line.  See the
	// update code to see why this makes this happen.
	m_bDossierSet = false;
	m_bShowIndex = false;

	// Let the residue printing object know which one we're displaying.

	CBagRPObject *pRPObj = (CBagRPObject *)m_pRPObj;
	pRPObj->SetActiveDossier(this);
}

} // namespace Bagel
