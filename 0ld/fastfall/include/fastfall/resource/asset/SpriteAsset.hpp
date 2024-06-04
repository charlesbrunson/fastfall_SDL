#pragma once

#include "fastfall/resource/asset/TextureAsset.hpp"
#include "fastfall/resource/asset/AnimAsset.hpp"

#include "fastfall/util/math.hpp"

#include <vector>
#include <map>
//#include <chrono>
//using namespace std::chrono;


namespace ff {

class AnimCompiler;
class AnimatedSprite;

class SpriteAsset : public TextureAsset {
public:
	struct ParsedAnim {
		const SpriteAsset* owner = nullptr;
		std::string name;
		Recti area;
		Vec2i origin;
		std::vector<unsigned> framerateMS;
        std::map<std::string, Vec2f, std::less<>> offsets;

		unsigned loop;

		bool has_chain = false;

		std::string chain_spr_name;
		std::string chain_anim_name;
		unsigned chain_frame;
	};

	explicit SpriteAsset(const std::filesystem::path& t_asset_path);
	~SpriteAsset() override;

	bool loadFromFile() override;
	bool reloadFromFile() override;

    bool postLoad() override { addParsedAnimsToDB(); return true; };

    void addParsedAnimsToDB();

	void ImGui_getContent(secs deltaTime) override;

    std::vector<std::filesystem::path> getDependencies() const override {
        return {
            get_path(),
            get_texture_path()
        };
    }

protected:
	friend AnimCompiler;

	AnimatedSprite* imgui_anim = nullptr;

    std::vector<ParsedAnim> parsedAnims;
	std::vector<AnimID> anims;
	std::vector<const char*> anims_labels;
	int anims_current = 0;
};

class AnimDB {
private:
    static std::map<std::pair<std::string, std::string>, AnimID> anim_lookup_table;
    static std::map<AnimID, Animation> animation_table;
public:
    static AnimID add_animation(const SpriteAsset::ParsedAnim& panim);
    static const Animation* get_animation(AnimID id);
    static AnimID get_animation_id(std::string_view sprite_name, std::string_view anim_name);
    static void reset();
};

}
