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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

CBofDebugOptions *g_pDebugOptions = nullptr;
CBofLog *g_pDebugLog = nullptr;

CBofDebugOptions::CBofDebugOptions(const char *pszFileName) : CBofOptions(pszFileName) {
	// Add programmer definable debug options here
	ConfMan.registerDefault("AbortsOn", true);
	ConfMan.registerDefault("MessageBoxOn", true);
	ConfMan.registerDefault("RandomOn", true);
	ConfMan.registerDefault("DebugLevel", gDebugLevel);
	ConfMan.registerDefault("ShowIO",false);
	ConfMan.registerDefault("MessageSpy", false);


	ReadSetting("DebugOptions", "AbortsOn", &m_bAbortsOn, ConfMan.getBool("AbortsOn"));
	ReadSetting("DebugOptions", "MessageBoxOn", &m_bMessageBoxOn, ConfMan.getBool("MessageBoxOn"));
	ReadSetting("DebugOptions", "RandomOn", &m_bRandomOn, ConfMan.getBool("RandomOn"));
	ReadSetting("DebugOptions", "DebugLevel", &m_nDebugLevel, ConfMan.getInt("DebugLevel"));
	ReadSetting("DebugOptions", "ShowIO", &m_bShowIO, ConfMan.getBool("ShowIO"));
	ReadSetting("DebugOptions", "MessageSpy", &m_bShowMessages, ConfMan.getBool("MessageSpy"));
}

void BofAssert(bool bExpression, int nLine, const char *pszSourceFile, const char *pszTimeStamp) {
	static char szBuf[200];
	static bool bAlready = false;

	/* Assert fails when expression is false
	 */
	if (!bExpression) {
		if (!bAlready) {
			bAlready = true;

			/* if this compiler supports the __TIMESTAMP__ macro, then show that also
			 */
			Common::sprintf_s(szBuf, "Internal error: File %s at line %d\n", pszSourceFile, nLine);
			if (pszTimeStamp != nullptr) {
				Common::sprintf_s(szBuf, "Internal error: File %s at line %d (FileDate: %s)\n", pszSourceFile, nLine, pszTimeStamp);
			}

			/*
			 * write this error to the log file
			 */
			if (g_pDebugLog != nullptr) {
				g_pDebugLog->WriteMessage(LOG_ERROR, szBuf, 0, nullptr);
			}

			bAlready = false;
		}

		error("%s", szBuf);
	}
}

void BofAbort(const char *pszInfo, const char *pszFile, int nLine) {
	char szBuf[200];

	Common::strcpy_s(szBuf, "Unknown reason for Abort");
	if (pszInfo != nullptr) {
		Common::strcpy_s(szBuf, pszInfo);
	}

	// log this message to DEBUG.LOG and to output window
	//
	if (g_pDebugLog != nullptr) {
		g_pDebugLog->WriteMessage(LOG_FATAL, szBuf, 0, pszFile, nLine);
	}

	// display message box saying why we are aborting
	//
	if (g_pDebugOptions->m_bMessageBoxOn) {
		warning("%s", szBuf);
	}

	if (g_pDebugOptions->m_bAbortsOn) {
		if (g_pDebugLog != nullptr) {
			g_pDebugLog->WriteMessage(LOG_FATAL, "Aborting!");
		}

		error("Aborted");
	}
}

} // namespace Bagel
