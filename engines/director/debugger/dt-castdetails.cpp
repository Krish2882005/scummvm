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

#include "backends/imgui/IconsMaterialSymbols.h"
#include "director/director.h"
#include "director/debugger/dt-internal.h"

#include "director/cast.h"
#include "director/castmember/castmember.h"
#include "director/movie.h"

namespace Director {
namespace DT {

static void PropRow(const char *label, const char *value) {
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("%s", label);
	ImGui::TableNextColumn();
	ImGui::Text("%s", value);
}

static void PropRowInt(const char *label, int value) {
	PropRow(label, Common::String::format("%d", value).c_str());
}

static void PropRowBool(const char *label, bool value) {
	PropRow(label, value ? "true" : "false");
}

static void PropRowRect(const char *label, const Common::Rect &r) {
	PropRow(label, Common::String::format("rect(%d, %d, %d, %d)", r.left, r.top, r.right, r.bottom).c_str());
}

static void PropRowPoint(const char *label, const Common::Point &p) {
	PropRow(label, Common::String::format("point(%d, %d)", p.x, p.y).c_str());
}

void showCastDetails() {
	if (!_state->_w.castDetails)
		return;

	ImGui::SetNextWindowPos(ImVec2(520, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(320, 500), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Property Inspector", &_state->_w.castDetails)) {
		CastMemberID id = _state->_cast._selectedCastMember;
		Movie *movie = g_director->getCurrentMovie();
		Cast *cast = nullptr;
		CastMember *member = nullptr;

		if (id.castLib != 0) {
			cast = movie->getCast(id);
			if (cast) {
				member = cast->getCastMember(id.member);
			}
		}

		if (!member) {
			ImGui::TextDisabled("No cast member selected.");
			ImGui::End();
			return;
		}

		CastMemberInfo *info = member->getInfo();
		Common::String displayName = getDisplayName(member);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", toIcon(member->_type));
		ImGui::SameLine();
		ImGui::Text("%d %s", member->getID(), displayName.c_str());

		ImGui::Separator();

		if (ImGui::BeginTabBar("##cast_details_tabs")) {

			if (ImGui::BeginTabItem("Member")) {
				if (ImGui::CollapsingHeader("Common Member Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("CommonProps", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
						ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

						PropRow("name", info->name.c_str());
						PropRowInt("number", member->getID());
						if (cast)
							PropRowInt("castLibNum", cast->_castLibID);
						PropRow("fileName", info->fileName.c_str());
						PropRow("type", toString(member->_type));
						PropRow("scriptText", info->script.c_str());
						PropRowInt("creationDate", info->creationTime);
						PropRowInt("modifiedDate", info->modifiedTime);
						PropRow("modifiedBy", info->modifiedBy.c_str());
						PropRow("comments", info->comments.c_str());
						PropRowInt("purgePriority", member->_purgePriority);
						PropRowBool("modified", member->isModified());

						ImGui::EndTable();
					}
				}

				if (ImGui::CollapsingHeader("Media Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("MediaPropsGen", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
						ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

						PropRowBool("loaded", member->isLoaded());
						PropRowInt("size", member->getCastDataSize());

						ImGui::EndTable();
					}
				}

				if (ImGui::CollapsingHeader("Graphic Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("GraphicProps", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
						ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
						ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

						PropRowBool("hilite", member->_hilite);
						Common::Rect r = member->getBbox();
						PropRowPoint("regPoint", member->getRegistrationOffset());
						PropRowInt("width", r.width());
						PropRowInt("height", r.height());
						PropRowRect("rect", r);

						ImGui::EndTable();
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Cast")) {
				if (ImGui::BeginTable("CastProps", 2, ImGuiTableFlags_Borders)) {
					PropRow("name", info->name.c_str());
					PropRowInt("number", member->getID());
					PropRow("fileName", info->fileName.c_str());
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

} // namespace DT
} // namespace Director
