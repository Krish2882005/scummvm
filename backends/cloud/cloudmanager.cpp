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

#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "common/config-manager.h"
#include "common/debug.h"

namespace Common {

DECLARE_SINGLETON(Cloud::CloudManager);

}

namespace Cloud {

CloudManager::CloudManager() : _currentStorageIndex(0) {}

CloudManager::~CloudManager() {
	//TODO: do we have to save storages on manager destruction?	
	for (uint32 i = 0; i < _storages.size(); ++i)
		delete _storages[i];
	_storages.clear();
}

void CloudManager::init() {
	bool offerDropbox = false;
	bool offerOneDrive = false;
	bool offerGoogleDrive = true;
	
	if (ConfMan.hasKey("storages_number", "cloud")) {
		int storages = ConfMan.getInt("storages_number", "cloud");
		for (int i = 1; i <= storages; ++i) {
			Storage *loaded = 0;
			Common::String keyPrefix = Common::String::format("storage%d_", i);
			if (ConfMan.hasKey(keyPrefix + "type", "cloud")) {
				Common::String storageType = ConfMan.get(keyPrefix + "type", "cloud");
				if (storageType == "Dropbox") loaded = Dropbox::DropboxStorage::loadFromConfig(keyPrefix);
				else if (storageType == "OneDrive") loaded = OneDrive::OneDriveStorage::loadFromConfig(keyPrefix);
				else if (storageType == "Google Drive") {
					loaded = GoogleDrive::GoogleDriveStorage::loadFromConfig(keyPrefix);
					offerGoogleDrive = false;
				} else warning("Unknown cloud storage type '%s' passed", storageType.c_str());
			} else {
				warning("Cloud storage #%d (out of %d) is missing.", i, storages);
			}
			if (loaded) _storages.push_back(loaded);
		}

		uint32 index = 0;
		if (ConfMan.hasKey("current_storage", "cloud")) {
			index = ConfMan.getInt("current_storage", "cloud") - 1; //count from 1, all for UX
		}
		if (index >= _storages.size()) index = 0;
		_currentStorageIndex = index;

		if (_storages.size() == 0) offerDropbox = true;
	} else {
		offerDropbox = true;
	}
	if (offerDropbox) {
		//this is temporary console offer to auth with Dropbox
		Dropbox::DropboxStorage::authThroughConsole();
	} else if (offerOneDrive) {
		//OneDrive time
		OneDrive::OneDriveStorage::authThroughConsole();
	} else if (offerGoogleDrive) {		
		GoogleDrive::GoogleDriveStorage::authThroughConsole();
		_currentStorageIndex = 100;
	}
}

void CloudManager::save() {
	ConfMan.set("storages_number", Common::String::format("%d", _storages.size()), "cloud");
	ConfMan.set("current_storage", Common::String::format("%d", _currentStorageIndex + 1), "cloud");
	for (uint32 i = 0; i < _storages.size(); ++i)
		_storages[i]->saveConfig(Common::String::format("storage%d_", i + 1));
	ConfMan.flushToDisk();
}

void CloudManager::addStorage(Storage *storage, bool makeCurrent, bool saveConfig) {
	if (!storage) error("Cloud::CloudManager: NULL storage passed");
	_storages.push_back(storage);
	if (makeCurrent) _currentStorageIndex = _storages.size() - 1;
	if (saveConfig) save();
}

Storage *CloudManager::getCurrentStorage() {
	if (_currentStorageIndex < _storages.size())
		return _storages[_currentStorageIndex];
	return nullptr;
}

void CloudManager::printBool(Storage::BoolResponse response) const {
	debug("bool = %s", (response.value ? "true" : "false"));
}

SavesSyncRequest *CloudManager::syncSaves(Storage::BoolCallback callback, Networking::ErrorCallback errorCallback) {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->syncSaves(callback, errorCallback);
	return nullptr;
}

void CloudManager::testFeature() {
	Storage *storage = getCurrentStorage();
	//if (storage) storage->info(nullptr, nullptr);
	GoogleDrive::GoogleDriveStorage *gd = dynamic_cast<GoogleDrive::GoogleDriveStorage *>(storage);
	if (gd) {
		//new folder in root: +
		//gd->createDirectory("newfolder1", nullptr, nullptr);

		//check it's there: +
		//gd->listDirectoryById("appDataFolder", nullptr, nullptr);

		//new folder in firstfolder: +
		//gd->createDirectory("firstfolder/newfolder2", nullptr, nullptr);

		//check it's there: +
		//gd->listDirectoryById("1LWq-r1IwegkJJ0eZpswGlyjj8nu6XyUmosvxD7L0F9X3", nullptr, nullptr);

		//create existing folder in firstfolder: +
		//gd->createDirectory("firstfolder/subfolder", nullptr, nullptr);

		//check no new folder there: +
		//gd->listDirectoryById("1LWq-r1IwegkJJ0eZpswGlyjj8nu6XyUmosvxD7L0F9X3", nullptr, nullptr);

		//create folder in subfolder: +
		//gd->createDirectory("firstfolder/subfolder/newfolder3", nullptr, nullptr);

		//check it's there: +
		//gd->listDirectoryById("1OysvorQlmGl2ObMGb1c-JnjfC5yFL-Zj7AsQQhNNBnrk", nullptr, nullptr);

		//one more time: +
		//gd->createDirectory("firstfolder/subfolder/newfolder3/megafolder", nullptr, nullptr);

		//check it's there: +
		//gd->listDirectoryById("1OXWPtfNgnmR_1K7SDm2v5J923bbAWrTdVDj-zRppLZDw", nullptr, nullptr);

		//gd->listDirectory("", nullptr, nullptr);
		//gd->listDirectory("firstfolder", nullptr, nullptr);
		gd->listDirectory("firstfolder/subfolder", nullptr, nullptr, true);
	}
		//gd->resolveFileId("firstfolder/subfolder", nullptr, nullptr);
		//gd->listDirectoryById("appDataFolder", nullptr, nullptr);
		//gd->listDirectoryById("1LWq-r1IwegkJJ0eZpswGlyjj8nu6XyUmosvxD7L0F9X3", nullptr, nullptr);
		//gd->createDirectoryWithParentId("1LWq-r1IwegkJJ0eZpswGlyjj8nu6XyUmosvxD7L0F9X3", "subfolder", nullptr, nullptr);
		//gd->createDirectoryWithParentId("appDataFolder", "firstfolder", nullptr, nullptr);
	else debug("FAILURE");
}

bool CloudManager::isWorking() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->isWorking();
	return false;
}

bool CloudManager::isSyncing() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->isSyncing();
	return false;
}

double CloudManager::getSyncDownloadingProgress() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->getSyncDownloadingProgress();
	return 1;
}

double CloudManager::getSyncProgress() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->getSyncProgress();
	return 1;
}

Common::Array<Common::String> CloudManager::getSyncingFiles() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->getSyncingFiles();
	return Common::Array<Common::String>();
}

void CloudManager::cancelSync() {
	Storage *storage = getCurrentStorage();
	if (storage) storage->cancelSync();
}

void CloudManager::setSyncTarget(GUI::CommandReceiver *target) {
	Storage *storage = getCurrentStorage();
	if (storage) storage->setSyncTarget(target);
}

} // End of namespace Common
