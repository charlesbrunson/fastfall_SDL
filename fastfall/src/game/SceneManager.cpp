#include "fastfall/game/SceneManager.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/util/log.hpp"

namespace ff {


SceneManager::SceneManager(unsigned instance)
	: instanceID(instance)
{

}

void SceneManager::add(SceneType scene_type, Drawable& drawable, Layer layer, Priority priority) {
	auto layer_iter = std::upper_bound(
		layers.begin(), layers.end(),
		layer,
		[](Layer layer, const SceneLayer& sceneLayer) {
			return layer < sceneLayer.layer_id;
		});

	if (layer_iter == layers.end() || layer_iter->layer_id != layer) {
		layer_iter = layers.insert(layer_iter, SceneLayer{});
		layer_iter->layer_id = layer;
	}

	layer_iter->drawables.push_back(
		SceneDrawable{
			.drawable = &drawable,
			.type = scene_type,
			.priority = priority
		}
	);

	std::stable_sort(layer_iter->drawables.begin(), layer_iter->drawables.end(),
			[](const SceneDrawable& lhs, const SceneDrawable& rhs) {
				return lhs.priority != rhs.priority ? lhs.priority < rhs.priority : lhs.type != SceneType::Object && rhs.type == SceneType::Object;
			}
		);
}

void SceneManager::remove(Drawable& drawable) {
	for (auto it1 = layers.begin(); it1 != layers.end(); it1++) {
		for (auto it2 = it1->drawables.cbegin(); it2 != it1->drawables.cend(); it2++) {
			if (it2->drawable == &drawable) {
				it1->drawables.erase(it2);
				if (it1->drawables.empty()) {
					layers.erase(it1);
				}
				return;
			}
		}
	}
}

void SceneManager::clear() {
	layers.clear();
}

void SceneManager::clearType(SceneType scene_type) {
	std::vector<SceneLayer> copy = layers;
	layers.clear();


	for (auto it1 = copy.begin(); it1 != copy.end(); it1++) {
		SceneLayer* nLayer = nullptr;
		
		for (auto it2 = it1->drawables.cbegin(); it2 != it1->drawables.cend(); it2++) {

			if (it2->type != scene_type) {
				if (!nLayer) {
					layers.push_back(SceneLayer{});
					nLayer = &layers.back();
					nLayer->layer_id = it1->layer_id;
				}
				nLayer->drawables.push_back(*it2);
			}
		}
	}

}

void SceneManager::set_cam_pos(Vec2f center) {
	cam_pos = center;
}

void SceneManager::set_bg_color(Color color) {
	background.setColor(color);
}

void SceneManager::set_size(Vec2u size) {
	scene_size = Vec2f{ size } *TILESIZE_F;

	// pixel buffer prevent visual artifacts at the edges of the level
	background.setPosition(Vec2f{ -1.f, -1.f });
	background.setSize(scene_size + Vec2f{2.f, 2.f});
}

void SceneManager::draw(ff::RenderTarget& target, ff::RenderState state) const {

	bool scissor_enabled = enableScissor(target, cam_pos);

	target.draw(background, state);

	//LOG_INFO("-----------");
	for (auto& layer : layers) {
		//LOG_INFO("{}", layer.layer_id);

		if (scissor_enabled && layer.layer_id >= 0) {
			scissor_enabled = false;
			disableScissor();
		}
		else if (!scissor_enabled && layer.layer_id < 0)
		{
			continue;
		}

		for (auto& scene_drawable : layer.drawables) {
			target.draw(*scene_drawable.drawable, state);
		}

	}

	if (scissor_enabled) {
		disableScissor();
	}
}


bool SceneManager::enableScissor(const RenderTarget& target, Vec2f viewPos) const {

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

void SceneManager::disableScissor() const {
	glDisable(GL_SCISSOR_TEST);
}

}