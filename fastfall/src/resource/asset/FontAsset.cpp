#include "fastfall/resource/asset/FontAsset.hpp"
namespace ff
{

	FontAsset::FontAsset(const std::string& filename) 
		: Asset(filename)
	{
		imgui_title = "Font Preview" + ImGui_ContentTag();
	}

	bool FontAsset::loadFromFile(const std::string& path_to_image) 
	{
		assetFilePath = path_to_image;

		if (loaded) {
			Font n_font;
			bool n_loaded = n_font.loadFromFile(assetFilePath + assetName);
			if (n_loaded) {
				font = std::move(n_font);
			}
		}
		else {
			loaded = font.loadFromFile(assetFilePath + assetName);
		}

		return loaded;
	}

	bool FontAsset::reloadFromFile() 
	{
		return loadFromFile(assetFilePath);
	}

	void FontAsset::ImGui_getContent() {
		ImGui::Text("%s", getAssetName().c_str());
		/*
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
		*/
	}
}