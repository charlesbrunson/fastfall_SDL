#pragma once

#include "fastfall/resource/Asset.hpp"
//#include <SFML/Graphics.hpp>

#include "fastfall/render/util/Texture.hpp"

namespace ff {

class TextureAsset : public Asset {
public:
	TextureAsset(const std::filesystem::path& t_asset_path);

	inline const Texture& getTexture() const noexcept {
		return tex;
	}

    void set_texture_path(const std::filesystem::path& t_tex_path);

	bool loadFromFile() override;

	bool reloadFromFile() override;

	void ImGui_getContent() override;

	inline auto get_texture_path() const noexcept { return texture_path; };

protected:

    std::filesystem::path texture_path;

	std::string imgui_title;
	bool imgui_showTex = false;

	Texture tex;
};

}