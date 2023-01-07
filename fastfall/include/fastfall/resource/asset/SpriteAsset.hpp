#pragma once


#include "fastfall/resource/asset/TextureAsset.hpp"
#include "fastfall/resource/asset/AnimAsset.hpp"

//#include "SFML/System.hpp"
#include "fastfall/util/math.hpp"

#include "fastfall/schema/resource-flat.hpp"

#include <chrono>
using namespace std::chrono;

//using AnimMap = std::map<AnimID, Animation, std::less<>>;

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
		unsigned loop;

		bool has_chain = false;

		std::string chain_spr_name;
		std::string chain_anim_name;
		unsigned chain_frame;
	};


	SpriteAsset(const std::string& filename);
	~SpriteAsset();

	bool loadFromFile(const std::string& relpath) override;
	bool loadFromFlat(const flat::resources::SpriteAssetF* builder);
	flatbuffers::Offset<flat::resources::SpriteAssetF> writeToFlat(flatbuffers::FlatBufferBuilder& builder) const;

	bool reloadFromFile() override;

	// returns a pointer to the animation, or nullptr if not found
	//const Animation* getAnimation(const std::string_view animName) const;

	[[nodiscard]]
	const std::vector<ParsedAnim>& getParsedAnims();

	void set_anim_IDs(std::vector<AnimID>&& anim_ids);

	void ImGui_getContent() override;

	//static std::map<AnimID, Animation> addAnimations(const std::vector<ParsedAnim>& allParsedAnims);

    std::vector<std::filesystem::path> getDependencies() const override {
        return {
            getFilePath() + getAssetName(),
            getTexPath()
        };
    }

protected:
	friend AnimCompiler;

	std::vector<ParsedAnim> parsedAnims;

	bool imgui_showAnim = false;

	//std::unique_ptr<AnimatedSprite> imgui_anim;

	AnimatedSprite* imgui_anim = nullptr;

	std::vector<AnimID> anims;
	std::vector<const char*> anims_labels;
	int anims_current = 0;

	//AnimMap animations;
};


template<>
struct flat_type<SpriteAsset>
{
	using type = flat::resources::SpriteAssetF;
};

}
