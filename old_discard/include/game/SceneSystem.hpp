#pragma once

#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/slot_map.hpp"

// manager of drawable for the instance

namespace ff {

enum class scene_type {
	Object,
	Level
};

// 0 is the default layer, 
// >0 is towards top (foreground)
// <0 is towards bottom (background)
using scene_layer = int;

// determines draw priority within the layer
enum class scene_priority {
	Lowest,
	Low,
	Normal,
	High,
	Highest
};

struct scene_config {
	scene_layer		layer_id = 0;
	scene_type		type = scene_type::Object;
	scene_priority	priority = scene_priority::Normal;
};

struct scene_id {
	slot_key value;
	bool operator==(const scene_id& other) const { return value == other.value; };
	bool operator!=(const scene_id& other) const { return value != other.value; };
};

class SceneSystem : public Drawable {
public:
	template<class T, class... Args>
		requires std::derived_from<T, Drawable>
	scene_id create(scene_config cfg, Args&&... args)
	{
		auto id = scene_id{
			drawables.emplace_back()
		};

		drawables.at(id.value) = std::make_unique<T>(std::forward<Args>(args)...);

		add_config(id, cfg);
		return id;
	}

	bool erase(scene_id target);


	Drawable* get(scene_id target) {
		return drawables.exists(target.value) ? drawables.at(target.value).get() : nullptr;
	}

	const Drawable* get(scene_id target) const {
		return drawables.exists(target.value) ? drawables.at(target.value).get() : nullptr;
	}

	scene_config get_config(scene_id target) const;
	void set_config(scene_id target, scene_config cfg);

	bool exists(scene_id target) const {
		return drawables.exists(target.value);
	}

	//void clearType(scene_type scene_type);
	//void clear();

	void set_cam_pos(Vec2f center);
	void set_bg_color(Color color);
	void set_size(Vec2u size);

	inline Color get_bg_color() const { return background.getColor(); };
	inline Vec2f get_size() const { return scene_size; };

private:
	struct scene_drawable {
		scene_id drawable;
		scene_config config;
	};

	void add_config(scene_id id, scene_config cfg);

	std::vector<scene_drawable> layers;
	slot_map<std::unique_ptr<Drawable>> drawables;

	ShapeRectangle background;
	Vec2f scene_size;
	Vec2f cam_pos;

	void draw(ff::RenderTarget& target, ff::RenderState state = RenderState()) const override;

	bool enableScissor(const RenderTarget& target, Vec2f viewPos) const;
	void disableScissor() const;
};

}