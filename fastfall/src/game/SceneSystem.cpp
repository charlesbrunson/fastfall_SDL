#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/util/log.hpp"

namespace ff {

void SceneSystem::add_config(scene_id id, scene_config cfg) {

	struct Comp
	{
		bool operator() (const scene_drawable& s, scene_layer i) const { return s.config.layer_id < i; }
		bool operator() (scene_layer i, const scene_drawable& s) const { return i < s.config.layer_id; }
	};

	auto [beg, end] = std::equal_range(layers.begin(), layers.end(), cfg.layer_id, Comp{});

	auto it = std::upper_bound(beg, end, cfg.priority, [](scene_priority p, const scene_drawable& d) {
		return p < d.config.priority;
		});

	layers.insert(it, { id, cfg });
}

bool SceneSystem::erase(scene_id target) {
	bool e = drawables.exists(target.value);
	if (e) {
		drawables.erase(target.value);
		layers.erase(std::find_if(layers.begin(), layers.end(), [&](const scene_drawable& d) { return d.drawable == target; }));
	}
	return e;
}


scene_config SceneSystem::get_config(scene_id target) const {

	auto it = std::find_if(layers.begin(), layers.end(), [&](const scene_drawable& scn) {
		return scn.drawable == target;
		});

	if (it != layers.end())
	{
		return it->config;
	}
	return {};
}

void SceneSystem::set_config(scene_id target, scene_config cfg) {
	auto it = std::find_if(layers.begin(), layers.end(), [&](const scene_drawable& scn) {
		return scn.drawable == target;
		});

	if (it != layers.end())
	{
		layers.erase(it);
		add_config(target, cfg);
	}
}

/*
void SceneSystem::clear() {
	layers.clear();
}

void SceneSystem::clearType(scene_type type) 
{
	std::erase_if(layers, [&](const scene_drawable& d) {
		if (d.config.type == type)
		{
			drawables.erase(d.drawable.value);
			return true;
		}
		return false;
	});
}
*/

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

	//LOG_INFO("-----------");
	for (auto& layer : layers) {
		//LOG_INFO("{}", layer.layer_id);

		
		if (scissor_enabled && layer.config.layer_id >= 0) {
			scissor_enabled = false;
			disableScissor();
		}
		else if (!scissor_enabled && layer.config.layer_id < 0)
		{
			continue;
		}
		

		target.draw(*get(layer.drawable), state);
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