
#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/resource/asset/AnimAsset.hpp"

#include "fastfall/util/xml.hpp"

#include <fstream>

#include "fastfall/resource/Resources.hpp"

namespace ff {

class AnimCompiler {
public:
	static bool parseAnimation(xml_node<>* animationNode, SpriteAsset& asset);
};

/*
const Animation* SpriteAsset::getAnimation(const std::string_view animName) const {
	auto r = animations.find(animName);

	if (r != animations.end()) {
		return &r->second;
	}
	return nullptr;
}
*/

SpriteAsset::SpriteAsset(const std::string& filename) :
	TextureAsset(filename)
{

}

const std::vector<SpriteAsset::ParsedAnim>& SpriteAsset::getParsedAnims() {
	return parsedAnims;
}

/*
std::map<AnimID, Animation> SpriteAsset::addAnimations(const std::vector<SpriteAsset::ParsedAnim>& allParsed) {

	std::map<AnimID, Animation> anims;

	for (const auto& animp : allParsed) {

		AnimID id = AnimID::reserve_id();

		Animation anim{ animp.owner };
		anim.anim_id = id;
		anim.anim_name = animp.name;
		anim.area = animp.area;
		anim.origin = animp.origin;
		anim.framerateMS = animp.framerateMS;
		anim.loop = animp.loop;

		//anims.push_back(Animation{ animp.owner, AnimID::reserve_id() });
		//Animation& anim = &anims.back();

		auto [iter, inserted] = anims.insert( std::make_pair(id, std::move(anim)) );
		assert(inserted);

	}
	return anims;
}
*/

bool SpriteAsset::loadFromFile(const std::string& relpath) {

	bool texLoaded = false;
	bool r = true;

	assetFilePath = relpath;
	std::unique_ptr<char[]> charPtr = readXML(assetFilePath + assetName);

	parsedAnims.clear();

	if (charPtr) {
		char* xmlContent = charPtr.get();

		auto doc = std::make_unique<xml_document<>>();

		try {
			doc->parse<0>(xmlContent);
			xml_node<>* index = doc->first_node("sprite");

			if (!index)
				throw parse_error("no sprite", nullptr);

			xml_node<>* node = index->first_node();

			while (node) {
				// parse image
				if (strcmp("image", node->name()) == 0) {
					xml_attribute<>* source_attr = node->first_attribute("source");

					if (!source_attr)
						throw parse_error("no sprite image source", nullptr);

					if (texLoaded)
						throw parse_error("too many sprite sources", nullptr);

					if (!TextureAsset::loadFromFile(relpath + std::string(source_attr->value())))
						throw parse_error("could not load sprite source", nullptr);

					texLoaded = true;

				}
				// parse animation
				else if (strcmp("animation", node->name()) == 0) {
					AnimCompiler::parseAnimation(node, *this);
				}
				node = node->next_sibling();
			}

			if (!texLoaded)
				throw parse_error("no image", nullptr);
		}
		catch (parse_error& err) {
			std::cout << assetName << ": " << err.what() << std::endl;
			r = false;
		}
	}
	else {
		std::cout << "Could not open file: " << relpath + assetName << std::endl;
		r = false;
	}

	loaded = r;
	return r;
}


bool SpriteAsset::reloadFromFile() {
	/*
	if (TextureAsset::reloadFromFile()) {

		// reload anims

		return true;
	}
	*/

	return loadFromFile(assetFilePath);

	//return false;
}

bool AnimCompiler::parseAnimation(xml_node<>* animationNode, SpriteAsset& asset) {


	//auto [iter, inserted] = asset.animations.insert(std::make_pair(name, std::move(Animation{ &asset })));

	SpriteAsset::ParsedAnim& anim = asset.parsedAnims.emplace_back(SpriteAsset::ParsedAnim{});

	if (!animationNode->first_attribute("name"))
		throw parse_error("animation has no name", nullptr);

	anim.name = animationNode->first_attribute("name")->value();
	anim.owner = &asset;

	xml_node<>* prop = animationNode->first_node();
	while (prop) {
		char* propName = prop->name();

		if (strcmp("area", propName) == 0) {
			anim.area.left = atoi(prop->first_attribute("x")->value());
			anim.area.top = atoi(prop->first_attribute("y")->value());
			anim.area.width = atoi(prop->first_attribute("w")->value());
			anim.area.height = atoi(prop->first_attribute("h")->value());
		}
		else if (strcmp("origin", propName) == 0) {
			anim.origin.x = atoi(prop->first_attribute("x")->value());
			anim.origin.y = atoi(prop->first_attribute("y")->value());
		}
		else if (strcmp("loop", propName) == 0) {
			anim.loop = atoi(prop->value());
		}
		else if (strcmp("framerate", propName) == 0) {
			unsigned count = 1;
			if (auto attr = prop->first_attribute("count")) {
				count = atoi(attr->value());
			}

			unsigned defMS = 0;
			if (auto attr = prop->first_attribute("defaultMS")) {
				defMS = atoi(attr->value());
			}

			anim.framerateMS = std::vector<unsigned>(count, defMS);

			xml_node<>* frameProp = prop->first_node("frame");
			unsigned i = 0;
			while (frameProp) {
				if (i < anim.framerateMS.size()) {
					anim.framerateMS[i] = atoi(frameProp->value());
				}
				else {
					anim.framerateMS.push_back(atoi(frameProp->value()));
				}
				i++;
				frameProp = frameProp->next_sibling("frame");
			}
		}
		else if (strcmp("chainsTo", propName) == 0) {
			anim.chain_anim_name = prop->first_attribute("name")->value();
			anim.chain_spr_name = prop->first_attribute("sprite")->value();
			anim.chain_frame = atoi(prop->first_attribute("frame")->value());

			if (!anim.chain_anim_name.empty() && !anim.chain_spr_name.empty())
				anim.has_chain = true;
		}
		prop = prop->next_sibling();
	}

	Resources::add_animation(anim);

	return true;
}
void SpriteAsset::ImGui_getContent() {
	TextureAsset::ImGui_getContent();
}

bool SpriteAsset::loadFromFlat(const flat::resources::SpriteAssetF* builder) {
	//auto t = std::unique_ptr<flat::resources::SpriteAssetT>(builder->UnPack());

	assetName = builder->name()->string_view();

	parsedAnims.clear();

	//for (auto& animT : builder->animations) {
	for (auto animT = builder->animations()->begin(); animT != builder->animations()->end(); animT++) {
		ParsedAnim anim;

		anim.owner = this;

		anim.name = animT->name()->string_view();
		anim.area = Recti(
			animT->area()->left(),
			animT->area()->top(),
			animT->area()->width(),
			animT->area()->height()
		);
		anim.origin = Vec2i(
			animT->origin()->x(),
			animT->origin()->y()
		);
		anim.framerateMS.insert(anim.framerateMS.end(), animT->framerate_ms()->begin(), animT->framerate_ms()->end());

		anim.loop = animT->loop();
		anim.has_chain = animT->has_chain();
		if (anim.has_chain) {
			anim.chain_anim_name = animT->chain_anim_name()->string_view();
			anim.chain_spr_name = animT->chain_spr_name()->string_view();
			anim.chain_frame = animT->chain_frame();
		}
		parsedAnims.push_back(std::move(anim));
		Resources::add_animation(parsedAnims.back());
	}

	loaded = tex.loadFromStream(builder->image()->Data(), builder->image()->size());
	return loaded;
}

flatbuffers::Offset<flat::resources::SpriteAssetF> SpriteAsset::writeToFlat(flatbuffers::FlatBufferBuilder& builder) const {

	using namespace flat::resources;
	using namespace flat::math;

	// animations
	std::vector<flatbuffers::Offset<AnimationAssetF>> anims;
	for (const ParsedAnim& anim : parsedAnims) {
		//const Animation* anim = &animpair.second;

		//auto assetName = builder.CreateString(anim.owner->assetName());
		auto animName = builder.CreateString(anim.name);
		RectFi area(anim.area.left, anim.area.top, anim.area.width, anim.area.height);
		Vec2Fi origin(anim.origin.x, anim.origin.y);
		auto framerate = builder.CreateVector(anim.framerateMS);

		auto chain_spr_str = builder.CreateString(anim.chain_spr_name);
		auto chain_name_str = builder.CreateString(anim.chain_anim_name);

		AnimationAssetFBuilder animBuilder(builder);
		animBuilder.add_name(animName);
		//animBuilder.add_spr_name(assetName);
		animBuilder.add_area(&area);
		animBuilder.add_origin(&origin);
		animBuilder.add_framerate_ms(framerate);
		animBuilder.add_loop(anim.loop);
		animBuilder.add_has_chain(anim.has_chain);
		if (anim.has_chain) {
			animBuilder.add_chain_spr_name(chain_spr_str);
			animBuilder.add_chain_anim_name(chain_name_str);
			animBuilder.add_chain_frame(anim.chain_frame);
		}
		anims.push_back(animBuilder.Finish());
	}

	auto flat_assetName = builder.CreateString(assetName);
	auto flat_anims = builder.CreateVector(anims);

	// texture data
	assert(!fullpath.empty());
	std::vector<int8_t> texData = readFile(fullpath.c_str());
	auto flat_texdata = builder.CreateVector(texData);

	SpriteAssetFBuilder spriteBuilder(builder);

	// sprite name
	assert(!assetName.empty());
	spriteBuilder.add_name(flat_assetName);
	spriteBuilder.add_animations(flat_anims);
	spriteBuilder.add_image(flat_texdata);

	return spriteBuilder.Finish();
}

}