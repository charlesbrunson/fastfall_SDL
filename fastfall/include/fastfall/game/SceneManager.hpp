#pragma once

#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/util/math.hpp"

// manager of drawable for the instance

namespace ff {

enum class SceneType {
	Object,
	Level
};

class SceneManager : public Drawable {
public:
	// 0 is the default layer, 
	// >0 is towards top (foreground)
	// <0 is towards bottom (background)
	using Layer = int;

	// determines draw priority within the layer
	enum class Priority {
		Lowest,
		Low,
		Normal,
		High,
		Highest
	};


	SceneManager(unsigned instance);

	void add(SceneType scene_type, Drawable& drawable, Layer layer = 0, Priority priority = Priority::Normal);

	void remove(Drawable& drawable);

	void clearType(SceneType scene_type);

	void clear();

	void set_bg_color(Color color);
	void set_size(Vec2u size);

	inline Color get_bg_color() const { return background.getColor(); };
	inline Vec2f get_size() const { return scene_size; };

private:
	struct SceneDrawable {
		Drawable* drawable;
		SceneType type;
		Priority priority;
	};

	struct SceneLayer {
		std::vector<SceneDrawable> drawables;
		Layer layer_id;
	};

	std::vector<SceneLayer> layers;
	ShapeRectangle background;
	Vec2f scene_size;


	void draw(ff::RenderTarget& target, ff::RenderState state = RenderState()) const override;

	bool enableScissor(const RenderTarget& target, Vec2f viewPos) const;
	void disableScissor() const;

	unsigned instanceID;
};

}