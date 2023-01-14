#include "fastfall/resource/asset/FontAsset.hpp"
namespace ff
{

	FontAsset::FontAsset(const std::filesystem::path& t_asset_path)
		: Asset(t_asset_path)
	{
		imgui_title = "Font Preview" + ImGui_ContentTag();
	}

	bool FontAsset::loadFromFile()
	{
		if (loaded) {
			Font n_font;
			bool n_loaded = n_font.loadFromFile(asset_path);
			if (n_loaded) {
				font = std::move(n_font);
			}
		}
		else {
			loaded = font.loadFromFile(asset_path);
		}

		return loaded;
	}

	bool FontAsset::reloadFromFile() 
	{
		return loadFromFile();
	}

	void FontAsset::ImGui_getContent() {
		ImGui::Text("%s", asset_path.c_str());
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