#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/util/log.hpp"

namespace ff {



void SceneSystem::set_config(ID<Drawable> id, SceneConfig cfg) {
    auto it = configs.find(id);
    if (it != configs.end())
    {
        it->second = cfg;
    }
}

void SceneSystem::update(World& world, secs deltaTime) {
    for (auto [did, drawable] : world.all<Drawable>()) {
        auto id = id_cast<Drawable>(did);
        auto& cfg = config(id);
        cfg.prev_pos = cfg.curr_pos;
    }
}

void SceneSystem::predraw(World& world, float interp, bool updated)
{
    if (updated) {
        add_to_scene(world);
    }

    for (auto [did, drawable] : world.all<Drawable>()) {
        drawable->predraw(interp, updated);

        auto id = id_cast<Drawable>(did);
        auto& cfg = config(id);
        Vec2f prev = cfg.prev_pos;
        Vec2f curr = cfg.curr_pos;
        config(id).rstate.transform.setPosition( prev + (curr - prev) * interp );
    }
}

void SceneSystem::notify_created(World& world, ID<Drawable> id)
{
    to_add.insert(id);
    configs.emplace(id, SceneConfig{});
}

void SceneSystem::add_to_scene(World& world)
{

    struct Comp {
        std::unordered_map<ID<Drawable>, SceneConfig>* configs;
        bool operator()(ID<Drawable> s, scene_layer i) const { return configs->at(s).layer_id < i; }
        bool operator()(scene_layer i, ID<Drawable> s) const { return i < configs->at(s).layer_id; }
    };

    for (auto id : to_add) {
        auto &scene_obj = configs.at(id);

        auto [beg, end] = std::equal_range(
                scene_order.begin(),
                scene_order.end(),
                scene_obj.layer_id,
                Comp{&configs});

        auto it = std::upper_bound(beg, end, scene_obj.priority, [this](scene_priority p, ID<Drawable> d) {
            return p < configs.at(d).priority;
        });

        scene_order.insert(it, id);
    }
    to_add.clear();

    for (auto id : to_erase) {
        scene_order.erase(
            std::find(scene_order.begin(), scene_order.end(), id)
        );
        to_add.erase(id);
        configs.erase(id);
    }
    to_erase.clear();

}

void SceneSystem::notify_erased(World& world, ID<Drawable> id)
{
    to_erase.insert(id);
}

void SceneSystem::set_cam_pos(Vec2f center) {
	cam_pos = center;
}

void SceneSystem::set_bg_color(Color color) {
	background.setColor(color);
}

void SceneSystem::set_size(Vec2u size) {
	scene_size = Vec2f{ size } * TILESIZE_F;

	// pixel buffer prevent visual artifacts at the edges of the level
	background.setPosition(Vec2f{ -1.f, -1.f });
	background.setSize(scene_size + Vec2f{2.f, 2.f});
}

void SceneSystem::draw(const World& world, RenderTarget& target, RenderState state) const {

	bool scissor_enabled = enableScissor(target, cam_pos);

	target.draw(background, state);

    for (auto id : scene_order) {
        auto& cfg = configs.at(id);

        if (scissor_enabled && cfg.layer_id >= 0) {
            scissor_enabled = false;
            disableScissor();
        }
        else if (!scissor_enabled && cfg.layer_id < 0)
        {
            continue;
        }

        if (cfg.render_enable)
        {
            auto st = state;
            if (cfg.rstate.texture.exists()
                && cfg.rstate.texture.get()->getID() != Texture::getNullTexture().getID())
            {
                st.texture = cfg.rstate.texture;
            }

            st.transform = Transform::combine(st.transform, cfg.rstate.transform);

            if (cfg.rstate.program)
                st.program = cfg.rstate.program;

            st.blend = cfg.rstate.blend;

            target.draw(world.at(id), st);
        }
    }

	if (scissor_enabled) {
		disableScissor();
	}
}


bool SceneSystem::enableScissor(const RenderTarget& target, Vec2f viewPos) const {

	glm::fvec4 scissor;
	View view = target.getView();
	scissor = view.getViewport();

	Vec2f vpOff{ view.getViewport()[0], view.getViewport()[1] };
	Vec2f vpSize{ view.getViewport()[2], view.getViewport()[3] };

	if (scene_size.x * scene_size.y > 0.f) {
		float zoom = view.getViewport()[2] / view.getSize().x;

		glm::fvec2 levelsize = Vec2f{ scene_size };
		glm::fvec2 campos = viewPos;
		glm::fvec2 campos2window = vpOff + (vpSize / 2.f);
		glm::fvec2 lvlbotleft2cam{ campos.x, levelsize.y - campos.y };
		glm::fvec2 levelbotleft2window = campos2window - (lvlbotleft2cam * zoom);

		scissor[0] = roundf(std::max(scissor[0], levelbotleft2window.x));
		scissor[1] = roundf(std::max(scissor[1], levelbotleft2window.y));

		scissor[2] = std::min(vpOff.x + vpSize.x, levelbotleft2window.x + (levelsize.x * zoom)) - scissor[0];
		scissor[3] = std::min(vpOff.y + vpSize.y, levelbotleft2window.y + (levelsize.y * zoom)) - scissor[1];

		scissor[2] = roundf(std::max(scissor[2], 0.f));
		scissor[3] = roundf(std::max(scissor[3], 0.f));
	}

	if (scissor[2] > 0.f && scissor[3] > 0.f) {
		glScissor(
			scissor[0],
			scissor[1],
			scissor[2],
			scissor[3]
		);
		glEnable(GL_SCISSOR_TEST);
	}
	return scissor[2] > 0.f && scissor[3] > 0.f;
}

void SceneSystem::disableScissor() const {
	glDisable(GL_SCISSOR_TEST);
}

}