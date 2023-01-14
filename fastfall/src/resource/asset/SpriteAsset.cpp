
#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/resource/asset/AnimAsset.hpp"
#include "fastfall/render/AnimatedSprite.hpp"

#include "fastfall/util/xml.hpp"

#include <fstream>
#include <chrono>

#include "fastfall/resource/Resources.hpp"

namespace ff {

class AnimCompiler {
public:
	static AnimID parseAnimation(xml_node<>* animationNode, SpriteAsset& asset);
};

SpriteAsset::SpriteAsset(const std::filesystem::path& t_asset_path) :
	TextureAsset(t_asset_path)
{
}

SpriteAsset::~SpriteAsset() {
	if (imgui_anim) {
		delete imgui_anim;
	}
}

const std::vector<SpriteAsset::ParsedAnim>& SpriteAsset::getParsedAnims() {
	return parsedAnims;
}
bool SpriteAsset::loadFromFile() {

	bool texLoaded = false;
	bool r = true;

	std::unique_ptr<char[]> charPtr = readXML(asset_path);

	parsedAnims.clear();
	anims.clear();

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

                    set_texture_path(asset_path.parent_path() / source_attr->value());
					if (!TextureAsset::loadFromFile())
						throw parse_error("could not load sprite source", nullptr);

					texLoaded = true;

				}
				// parse animation
				else if (strcmp("animation", node->name()) == 0) {
					anims.push_back(AnimCompiler::parseAnimation(node, *this));
				}
				node = node->next_sibling();
			}

			if (!texLoaded)
				throw parse_error("no image", nullptr);
		}
		catch (parse_error& err) {
			std::cout << asset_path << ": " << err.what() << std::endl;
			r = false;
		}
	}
	else {
		std::cout << "Could not open file: " << asset_path << std::endl;
		r = false;
	}

	loaded = r;
	return r;
}


bool SpriteAsset::reloadFromFile() {
	return loadFromFile();
}

AnimID AnimCompiler::parseAnimation(xml_node<>* animationNode, SpriteAsset& asset)
{
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

	return AnimDB::add_animation(anim);
}
void SpriteAsset::ImGui_getContent() {
	ImGui::Text("%s", asset_path.c_str());
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	if (ImGui::Button("Show Sprite")) {
		imgui_showTex = true;
	}

	using namespace std::chrono;
	
	static steady_clock anim_clock;
	static time_point<steady_clock> last_time;
	static time_point<steady_clock> curr_time = anim_clock.now();

	last_time = curr_time;
	curr_time = anim_clock.now();

	duration<double> elapsed = (curr_time - last_time);
	double secs = elapsed.count();

	if (imgui_showTex) {
		if (ImGui::Begin(imgui_title.c_str(), &imgui_showTex)) {

			static char tab_name[32];
			snprintf(tab_name, 32, "Sprite Asset#%p", this);

			if (ImGui::BeginTabBar(tab_name)) {

				if (ImGui::BeginTabItem("Animations"))
				{
					static unsigned scale = 4;
					static float playback_speed = 1.f;

					ImGui::Text("This is for animations!");
					if (!imgui_anim || anims_labels.empty())
					{
						//imgui_anim = std::make_unique<AnimatedSprite>();
						imgui_anim = new AnimatedSprite();
						anims_labels.clear();
							
						std::transform(anims.begin(), anims.end(), std::back_inserter(anims_labels), 
							[](const AnimID& id) {
								auto anim = AnimDB::get_animation(id);
								return anim ? anim->anim_name.data() : "";
							});
						anims_current = 0;
					}
					if (!imgui_anim->has_anim())
					{
						if (!anims.empty())
						{
							imgui_anim->set_anim(*anims.begin());
						}

						scale = 4;
						playback_speed = 1.0;
					}
					if (imgui_anim->has_anim()) 
					{
						imgui_anim->update(secs * playback_speed);
						imgui_anim->predraw(secs * playback_speed, false);

						if (imgui_anim->is_complete() 
							&& imgui_anim->get_anim()->loop != 0)
						{
							imgui_anim->reset_anim();
						}
						else if (imgui_anim->get_anim()->anim_id != anims[anims_current])
						{
							imgui_anim->set_anim(anims[anims_current]);
						}

						Sprite spr = imgui_anim->get_sprite();
						TextureRef tex = *spr.getTexture();

						Rectf rect = spr.getTextureRect();


						ImVec2 tex_size = {
							(float)rect.width * scale,
							(float)rect.height * scale
						};

						ImVec2 uv0 = {
							rect.left * tex.get()->inverseSize().x,
							rect.top * tex.get()->inverseSize().y
						};

						ImVec2 uv1 = {
							(rect.left + rect.width) * tex.get()->inverseSize().x,
							(rect.top + rect.height) * tex.get()->inverseSize().y
						};

						if (ImGui::Combo("Animations", &anims_current, anims_labels.data(), anims_labels.size()))
						{
							imgui_anim->set_anim(anims[anims_current]);
						}

						if (ImGui::Button("-")) { scale = std::max(1u, scale - 1); }
						ImGui::SameLine();
						if (ImGui::Button("+")) { scale = std::min(16u, scale + 1); }
						ImGui::SameLine();
						ImGui::Text("%2dx", scale);

						ImGui::DragFloat("Playback Rate", &playback_speed, 0.05f, 0.1f, 5.f);
						if (ImGui::Button("Reset Playback")) { playback_speed = 1.f; }

						static ImVec4 bg_color = { 0.75f, 0.75f, 0.75f, 1.f };
						ImGui::ColorEdit3("Bg Color", (float*)&bg_color);

						static char anim_child[32];
						snprintf(anim_child, 32, "##%p", this);
						ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_color);
						if (ImGui::BeginChild(anim_child)) {
							ImGui::Image((void*)(intptr_t)tex.get()->getID(), tex_size, uv0, uv1);
						}
						ImGui::EndChild();
						ImGui::PopStyleColor();

					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Texture"))
				{
					ImGui::Image((void*)(intptr_t)tex.getID(), ImVec2(tex.size().x, tex.size().y));
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		else {
			//imgui_anim.reset();
			if (imgui_anim) {
				delete imgui_anim;
			}
			anims_labels.clear();
			anims_current = 0;
		}

		ImGui::End();
	}
}

std::map<std::pair<std::string, std::string>, AnimID> AnimDB::anim_lookup_table;
std::map<AnimID, Animation> AnimDB::animation_table;

const Animation* AnimDB::get_animation(AnimID id)
{
    if (auto iter = animation_table.find(id);
            iter != animation_table.end())
    {
        return &iter->second;
    }
    return nullptr;
}

AnimID AnimDB::get_animation_id(std::string_view sprite_name, std::string_view anim_name)
{
    std::pair<std::string, std::string> key{ sprite_name, anim_name };
    if (auto iter = anim_lookup_table.find(key);
            iter != anim_lookup_table.end())
    {
        return iter->second;
    }
    return AnimID::NONE;
}

AnimID AnimDB::add_animation(const SpriteAsset::ParsedAnim& panim) {

    auto doChain = [&panim](Animation& anim) {

        if (anim.chain.has_chain) {

            AnimID chain_id = get_animation_id(panim.chain_spr_name, panim.chain_anim_name);

            if (chain_id == AnimID::NONE) {
                // operate under assumption that there will be an anim later on to fill this
                anim.chain.anim_id = AnimID::reserve_id();
                auto chain_key = std::pair<std::string, std::string>(panim.chain_spr_name, panim.chain_anim_name);
                anim_lookup_table.insert(std::make_pair(chain_key, anim.chain.anim_id));
            }
            else {
                anim.chain.anim_id = chain_id;
            }

            anim.chain.start_frame = panim.chain_frame;
        }
    };

    log::scope scope;

    AnimID existing_id = get_animation_id(panim.owner->get_name(), panim.name);

    if (existing_id == AnimID::NONE) {
        AnimID nID = AnimID::reserve_id();
        auto key = std::pair<std::string, std::string>(panim.owner->get_name(), panim.name);
        anim_lookup_table.insert(std::make_pair(key, nID));


        Animation anim{ panim.owner, nID };
        anim.anim_name = panim.name;
        anim.area = panim.area;
        anim.origin = panim.origin;
        anim.loop = panim.loop;

        anim.framerateMS = panim.framerateMS;
        anim.chain.has_chain = panim.has_chain;
        doChain(anim);

        //resource.animation_table.insert(std::make_pair( anim.anim_id, anim ));
        animation_table[anim.anim_id] = std::move(anim);
        existing_id = anim.anim_id;
    }
    else {
        Animation anim{ panim.owner, existing_id };
        anim.anim_name = panim.name;
        anim.area = panim.area;
        anim.origin = panim.origin;
        anim.loop = panim.loop;

        anim.framerateMS = panim.framerateMS;
        anim.chain.has_chain = panim.has_chain;
        doChain(anim);

        animation_table[existing_id] = std::move(anim);
    }
    LOG_INFO("added anim: {} - {}", panim.owner->get_name().data(), panim.name);
    return existing_id;
}

void AnimDB::reset() {
    anim_lookup_table.clear();
    animation_table.clear();
}

}
