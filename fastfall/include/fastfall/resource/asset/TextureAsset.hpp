#pragma once

#include "fastfall/resource/Asset.hpp"
//#include <SFML/Graphics.hpp>

#include "fastfall/render/Texture.hpp"

namespace ff {

class TextureAsset : public Asset {
public:
	TextureAsset(const std::string& filename);

	inline const Texture& getTexture() const noexcept {
		return tex;
	}

	bool loadFromFile(const std::string& relpath) override { return false; };

	bool reloadFromFile() override {
		//TODO
		return false;
	};

	void ImGui_getContent() override;

	inline std::string getTexPath() const noexcept { return fullpath; };

protected:

	std::string fullpath;

	std::string imgui_title;
	bool imgui_showTex = false;

	//sf::Texture tex;
	Texture tex;
};

}