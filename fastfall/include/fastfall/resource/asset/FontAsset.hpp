#pragma once

#include "fastfall/resource/Asset.hpp"
#include "fastfall/render/Font.hpp"

namespace ff {

	class FontAsset : public Asset {
	public:
		FontAsset(const std::filesystem::path& t_asset_path);

		bool loadFromFile() override;

		bool reloadFromFile() override;

		inline const Font& getFont() const noexcept { return font; }

		void ImGui_getContent() override;

		operator const Font&() const {
			return getFont();
		}

        std::vector<std::filesystem::path> getDependencies() const override {
            return { asset_path };
        }

	protected:
		std::string fullpath;
		Font font;

		std::string imgui_title;
		bool imgui_showTex = false;
	};

}