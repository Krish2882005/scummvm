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

#include "backends/cloud/box/boxlistdirectorybyidrequest.h"
#include "backends/cloud/box/boxstorage.h"
#include "backends/cloud/box/boxtokenrefresher.h"
#include "backends/cloud/iso8601.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace Box {

#define BOX_LIST_DIRECTORY_LIMIT 1000

BoxListDirectoryByIdRequest::BoxListDirectoryByIdRequest(BoxStorage *storage, Common::String id, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _requestedId(id), _storage(storage), _listDirectoryCallback(cb),
	 _workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

BoxListDirectoryByIdRequest::~BoxListDirectoryByIdRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _listDirectoryCallback;
}

void BoxListDirectoryByIdRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_files.clear();
	_ignoreCallback = false;

	makeRequest(0);
}

void BoxListDirectoryByIdRequest::makeRequest(uint32 offset) {
	Common::String url = Common::String::format(
		"https://api.box.com/2.0/folders/%s/items?offset=%u&limit=%u&fields=%s",
		_requestedId.c_str(),
		offset,
		BOX_LIST_DIRECTORY_LIMIT,
		"id,type,name,size,modified_at"
	);

	Networking::JsonCallback callback = new Common::Callback<BoxListDirectoryByIdRequest, Networking::JsonResponse>(this, &BoxListDirectoryByIdRequest::responseCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<BoxListDirectoryByIdRequest, Networking::ErrorResponse>(this, &BoxListDirectoryByIdRequest::errorCallback);
	Networking::CurlJsonRequest *request = new BoxTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	_workingRequest = ConnMan.addRequest(request);
}

void BoxListDirectoryByIdRequest::responseCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	if (response.request) _date = response.request->date();

	Networking::ErrorResponse error(this);
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq && rq->getNetworkReadStream())
		error.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	Common::JSONValue *json = response.value;
	if (json) {
		Common::JSONObject responseObject = json->asObject();

		//debug("%s", json->stringify(true).c_str());

		//TODO: check that error is returned the right way
		/*
		if (responseObject.contains("error") || responseObject.contains("error_summary")) {
			warning("Box returned error: %s", responseObject.getVal("error_summary")->asString().c_str());
			error.failed = true;
			error.response = json->stringify();
			finishError(error);
			delete json;
			return;
		}
		*/

		//TODO: check that ALL keys exist AND HAVE RIGHT TYPE to avoid segfaults

		if (responseObject.contains("entries") && responseObject.getVal("entries")->isArray()) {
			Common::JSONArray items = responseObject.getVal("entries")->asArray();
			for (uint32 i = 0; i < items.size(); ++i) {
				Common::JSONObject item = items[i]->asObject();
				Common::String id = item.getVal("id")->asString();
				Common::String name = item.getVal("name")->asString();
				bool isDirectory = (item.getVal("type")->asString() == "folder");
				uint32 size = 0, timestamp = 0;
				if (item.contains("size")) {
					if (item.getVal("size")->isString())
						size = item.getVal("size")->asString().asUint64();
					else if (item.getVal("size")->isIntegerNumber())
						size = item.getVal("size")->asIntegerNumber();
					else
						warning("strange type for field 'size'");
				}
				if (item.contains("modified_at") && item.getVal("modified_at")->isString())
					timestamp = ISO8601::convertToTimestamp(item.getVal("modified_at")->asString());

				//as we list directory by id, we can't determine full path for the file, so we leave it empty
				_files.push_back(StorageFile(id, "", name, size, timestamp, isDirectory));
			}
		}

		uint32 received = 0;
		uint32 totalCount = 0;
		if (responseObject.contains("total_count") && responseObject.getVal("total_count")->isIntegerNumber())
			totalCount = responseObject.getVal("total_count")->asIntegerNumber();
		if (responseObject.contains("offset") && responseObject.getVal("offset")->isIntegerNumber())
			received = responseObject.getVal("offset")->asIntegerNumber();
		if (responseObject.contains("limit") && responseObject.getVal("limit")->isIntegerNumber())
			received += responseObject.getVal("limit")->asIntegerNumber();
		bool hasMore = (received < totalCount);

		if (hasMore) makeRequest(received);
		else finishListing(_files);
	} else {
		warning("null, not json");
		error.failed = true;
		finishError(error);
	}

	delete json;
}

void BoxListDirectoryByIdRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	if (error.request) _date = error.request->date();
	finishError(error);
}

void BoxListDirectoryByIdRequest::handle() {}

void BoxListDirectoryByIdRequest::restart() { start(); }

Common::String BoxListDirectoryByIdRequest::date() const { return _date; }

void BoxListDirectoryByIdRequest::finishListing(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_listDirectoryCallback) (*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, files));
}

} // End of namespace Box
} // End of namespace Cloud
