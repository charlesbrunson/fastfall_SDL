#pragma once

#include "fastfall/resource/Asset.hpp"
#include "fastfall/render/Font.hpp"

namespace ff {

	class FontAsset : public Asset {
	public:
		FontAsset(const std::string& filename);

		bool loadFromFile(const std::string& path_to_image) override;

		bool reloadFromFile() override;

		inline const Font& getFont() const noexcept { return font; }

		void ImGui_getContent() override;

		operator const Font&() const
		{
			return getFont();
		}

	protected:
		std::string fullpath;
		Font font;

		std::string imgui_title;
		bool imgui_showTex = false;
	};

}