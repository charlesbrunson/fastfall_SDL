#pragma once

#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/slot_map.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/game/scene/SceneObject.hpp"

// manager of drawable for the instance

namespace ff {

class World;

class SceneSystem {
public:

    //inline void set_world(World* w) { world = w; }
    void notify_created(World& world, ID<SceneObject> id);
    void notify_erased(World& world, ID<SceneObject> id);

	void set_cam_pos(Vec2f center);
	void set_bg_color(Color color);
	void set_size(Vec2u size);

	inline Color get_bg_color() const { return background.getColor(); };
	inline Vec2f get_size() const { return scene_size; };

    const std::vector<ID<SceneObject>>& get_scene_order() const { return scene_order; }

    void predraw(World& world, float interp, bool updated);

    void draw(const World& world, RenderTarget& target, RenderState state = RenderState()) const;
private:
    //void draw(ff::RenderTarget& target, ff::RenderState state = RenderState()) const override;
    bool enableScissor(const RenderTarget& target, Vec2f viewPos) const;
    void disableScissor() const;

    std::vector<ID<SceneObject>> to_add;
    std::vector<ID<SceneObject>> to_erase;
	std::vector<ID<SceneObject>> scene_order;

    void add_to_scene(World& world);

	ShapeRectangle background;
	Vec2f scene_size;
	Vec2f cam_pos;

    //World* world;


};

}