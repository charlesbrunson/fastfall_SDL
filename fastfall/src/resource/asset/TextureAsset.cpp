
//#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/asset/TextureAsset.hpp"
#include "imgui.h"

//#include "ImGui-SFML/imgui-SFML.h"

#include <sstream>
#include <fstream>

#include <assert.h>

namespace ff {

TextureAsset::TextureAsset(const std::string& filename) :
	Asset(filename)
{
	imgui_title = "Texture Preview" + ImGui_ContentTag();
}


bool TextureAsset::loadFromFile(const std::string& path_to_image) {
	fullpath = path_to_image;

	if (loaded) {
		ff::Texture n_tex;
		bool n_loaded = n_tex.loadFromFile(fullpath);
		if (n_loaded) {
			tex = std::move(n_tex);
		}
	}
	else {
		loaded = tex.loadFromFile(fullpath);
	}

	return loaded;
}

bool TextureAsset::reloadFromFile() {

	ff::Texture n_tex;
	bool n_loaded = n_tex.loadFromFile(fullpath);
	if (n_loaded) {
		tex = std::move(n_tex);
	}
	return n_loaded;
}

void TextureAsset::ImGui_getContent() {
	ImGui::Text("%s", getAssetName().c_str());
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	if (ImGui::Button("Show Texture")) {
		imgui_showTex = true;
	}

	if (imgui_showTex) {
		if (ImGui::Begin(imgui_title.c_str(), &imgui_showTex)) {
			
			ImGui::Image((void*)(intptr_t)tex.getID(), ImVec2(tex.size().x, tex.size().y));

			ImGui::End();
		}
	}
}

}