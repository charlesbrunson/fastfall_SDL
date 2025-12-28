#include "fastfall/game/systems/SceneSystem.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/util/log.hpp"

namespace ff {



void SceneSystem::set_config(ID<Drawable> id, SceneConfig cfg) {
    auto it = update_configs.find(id);
    if (it != update_configs.end())
    {
        it->second = cfg;
    }
}

void SceneSystem::update(World& world, secs deltaTime) {
    for (auto [did, drawable] : world.all<Drawable>())
    {
        auto& cfg = config(did);
        if (cfg.auto_update_prev_pos)
            cfg.prev_pos = cfg.curr_pos;
    }
}

void SceneSystem::predraw(World& world, predraw_state_t predraw_state)
{
    if (predraw_state.updated) {
        add_to_scene(world);
    }

    for (auto [did, drawable] : world.all<Drawable>()) {
        drawable->predraw(predraw_state);

        if (!world.due_to_erase(did)) {
            auto& ucfg = update_configs.at(did);
            Vec2f prev = ucfg.prev_pos;
            Vec2f curr = ucfg.curr_pos;
            ucfg.rstate.transform.setPosition(math::lerp(prev, curr, predraw_state.interp));
        }
    }

    draw_configs = update_configs;
}

void SceneSystem::reset_proxy_ptrs(const poly_id_map<Drawable>& drawables) {
    for (auto& proxy : scene_order)
    {
        proxy.ptr = drawables.get(proxy.id);
        assert(proxy.ptr != nullptr);
    }
}

void SceneSystem::notify_created(World& world, ID<Drawable> id)
{
    to_add.insert(id);
    update_configs.emplace(id, SceneConfig{});
}

void SceneSystem::add_to_scene(World& world)
{

    struct Comp {
        std::unordered_map<ID<Drawable>, SceneConfig>* configs;
        bool operator()(const proxy_drawable_t& s, scene_layer i) const { return configs->at(s.id).layer_id < i; }
        bool operator()(scene_layer i, const proxy_drawable_t& s) const { return i < configs->at(s.id).layer_id; }
    };

    for (auto id : to_add) {
        auto &scene_obj = update_configs.at(id);

        auto [beg, end] = std::equal_range(
                scene_order.begin(),
                scene_order.end(),
                scene_obj.layer_id,
                Comp{ &update_configs });

        auto it = std::upper_bound(beg, end, scene_obj.priority, [this](scene_priority p, const proxy_drawable_t& d) {
            return p < update_configs.at(d.id).priority;
        });

        scene_order.insert(it, proxy_drawable_t{ .id = id, .ptr = world.get(id) });
    }
    to_add.clear();

    for (auto id : to_erase) {
        scene_order.erase(
            std::find_if(
            scene_order.begin(),
            scene_order.end(),
            [&id](const proxy_drawable_t& s) { return s.id == id; }
            )
        );
        to_add.erase(id);
        update_configs.erase(id);
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

    for (auto& proxy : scene_order) {

        auto& cfg = draw_configs.at(proxy.id);

        if (scissor_enabled && cfg.layer_id >= 0) {
            scissor_enabled = false;
            disableScissor();
        }
        else if (!scissor_enabled && cfg.layer_id < 0)
        {
            continue;
        }

        if (cfg.visible)
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

            target.draw(*proxy.ptr, st);
        }
    }

	if (scissor_enabled) {
		disableScissor();
	}
}


bool SceneSystem::enableScissor(const RenderTarget& target, Vec2f viewPos) const {

	View view = target.getView();
	Rectf vp = view.getViewport();

	if (scene_size.x * scene_size.y > 0.f) {
		float zoom = vp.width / view.getSize().x;

		Vec2f levelsize = Vec2f{ scene_size };
		Vec2f campos = viewPos;
		Vec2f campos2window = vp.center();
		Vec2f lvlbotleft2cam{ campos.x, levelsize.y - campos.y };
		Vec2f levelbotleft2window = campos2window - (lvlbotleft2cam * zoom);

		vp.left = roundf((std::max)(vp.left, levelbotleft2window.x));
		vp.top = roundf((std::max)(vp.top, levelbotleft2window.y));

		vp.width = (std::min)(vp.rightmid().x, levelbotleft2window.x + (levelsize.x * zoom)) - vp.left;
		vp.height = (std::min)(vp.botmid().y, levelbotleft2window.y + (levelsize.y * zoom)) - vp.top;

		vp.width = roundf((std::max)(vp.width, 0.f));
		vp.height = roundf((std::max)(vp.height, 0.f));
	}

	if (vp.width > 0.f && vp.height > 0.f) {
		glScissor(
			vp.left,
			vp.top,
			vp.width,
			vp.height
		);
		glEnable(GL_SCISSOR_TEST);
	}
	return vp.width > 0.f && vp.height > 0.f;
}

void SceneSystem::disableScissor() const {
	glDisable(GL_SCISSOR_TEST);
}

}