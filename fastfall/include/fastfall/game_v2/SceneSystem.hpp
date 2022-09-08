#pragma once

#include "fastfall/render/Drawable.hpp"
#include "fastfall/game_v2/scene/SceneObject.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/slot_map.hpp"
#include "fastfall/util/id.hpp"

// manager of drawable for the instance

namespace ff {

class World;

class SceneSystem : public Drawable {
public:

    inline void set_world(World* w) { world = w; }
    void notify_created(ID<SceneObject> id);
    void notify_erased(ID<SceneObject> id);

	void set_cam_pos(Vec2f center);
	void set_bg_color(Color color);
	void set_size(Vec2u size);

	inline Color get_bg_color() const { return background.getColor(); };
	inline Vec2f get_size() const { return scene_size; };

private:
    void draw(ff::RenderTarget& target, ff::RenderState state = RenderState()) const override;
    bool enableScissor(const RenderTarget& target, Vec2f viewPos) const;
    void disableScissor() const;

	std::vector<ID<SceneObject>> scene_order;

	ShapeRectangle background;
	Vec2f scene_size;
	Vec2f cam_pos;

    World* world;


};

}