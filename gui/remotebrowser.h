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

#ifndef GUI_REMOTEBROWSER_DIALOG_H
#define GUI_REMOTEBROWSER_DIALOG_H

#include "gui/dialog.h"
#include "common/fs.h"
#include <backends/cloud/storagefile.h>
#include <backends/networking/curl/request.h>
#include <backends/cloud/storage.h>

namespace GUI {

class ListWidget;
class StaticTextWidget;
class CheckboxWidget;
class CommandSender;

class RemoteBrowserDialog : public Dialog {
public:
	RemoteBrowserDialog(const char *title);

	virtual void open();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleTickle();

	const Cloud::StorageFile	&getResult() { return _choice; }

protected:
	ListWidget		*_fileList;
	StaticTextWidget	*_currentPath;
	Cloud::StorageFile _node, _backupNode;
	Common::Array<Cloud::StorageFile> _nodeContent;
	Cloud::StorageFile _choice;
	bool _navigationLocked;
	bool _updateList;

	Networking::Request *_workingRequest;
	bool _ignoreCallback; //?

	void updateListing();
	void goUp();
	void listDirectory(Cloud::StorageFile node);
	void directoryListedCallback(Cloud::Storage::ListDirectoryResponse response);
	void directoryListedErrorCallback(Networking::ErrorResponse error);
};

} // End of namespace GUI

#endif
