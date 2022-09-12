#include "fastfall/game_v2/SceneSystem.hpp"
#include "fastfall/game_v2/World.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/util/log.hpp"

namespace ff {

void SceneSystem::notify_created(ID<SceneObject> id)
{
    auto& scene_obj = world->at(id);
    struct Comp
    {
        World* w;
        bool operator() (ID<SceneObject> s, scene_layer i) const { return w->at(s).layer_id < i; }
        bool operator() (scene_layer i, ID<SceneObject> s) const { return i < w->at(s).layer_id; }
    };

    auto [beg, end] = std::equal_range(
            scene_order.begin(),
            scene_order.end(),
            scene_obj.layer_id,
            Comp{world});

    auto it = std::upper_bound(beg, end, scene_obj.priority, [this](scene_priority p, ID<SceneObject> d) {
        return p < world->at(d).priority;
    });

    scene_order.insert(it, id);
}

void SceneSystem::notify_erased(ID<SceneObject> id)
{
    std::erase(scene_order, id);
}

void SceneSystem::set_cam_pos(Vec2f center) {
	cam_pos = center;
}

void SceneSystem::set_bg_color(Color color) {
	background.setColor(color);
}

void SceneSystem::set_size(Vec2u size) {
	scene_size = Vec2f{ size } *TILESIZE_F;

	// pixel buffer prevent visual artifacts at the edges of the level
	background.setPosition(Vec2f{ -1.f, -1.f });
	background.setSize(scene_size + Vec2f{2.f, 2.f});
}

void SceneSystem::draw(ff::RenderTarget& target, ff::RenderState state) const {

	bool scissor_enabled = enableScissor(target, cam_pos);

	target.draw(background, state);

	for (auto scene_id : scene_order) {
        auto& scene_object = world->at(scene_id);
        auto& drawable = *scene_object.drawable.get();

        if (!scene_object.render_enable)
            continue;

		if (scissor_enabled && scene_object.layer_id >= 0) {
			scissor_enabled = false;
			disableScissor();
		}
		else if (!scissor_enabled && scene_object.layer_id < 0)
		{
			continue;
		}

		target.draw(drawable, state);
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