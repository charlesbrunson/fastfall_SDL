#pragma once

#include "fastfall/render/Drawable.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/slot_map.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/scene/SceneConfig.hpp"

#include <unordered_set>
#include <unordered_map>

// manager of drawable for the instance

namespace ff {

class World;

class SceneSystem {
public:
    struct proxy_drawable_t {
        ID<Drawable> id;
        const Drawable* ptr = nullptr;
    };

    void notify_created(World& world, ID<Drawable> id);
    void notify_erased(World& world, ID<Drawable> id);

	void set_cam_pos(Vec2f center);
	void set_bg_color(Color color);
	void set_size(Vec2u size);

    // need to do this anytime the world state is copied :(
    void reset_proxy_ptrs(const poly_id_map<Drawable>& drawables);

	inline Color get_bg_color() const { return background.getColor(); };
	inline Vec2f get_size() const { return scene_size; };

    const std::vector<proxy_drawable_t>& get_scene_order() const { return scene_order; }

    SceneConfig& config(ID<Drawable> id) { return configs.at(id); }
    const SceneConfig& config(ID<Drawable>id) const { return configs.at(id); }

    void set_config(ID<Drawable> id, SceneConfig cfg);

    void update(World& world, secs deltaTime);
    void predraw(World& world, float interp, bool updated);

    void draw(const World& world, RenderTarget& target, RenderState state = RenderState()) const;
private:
    bool enableScissor(const RenderTarget& target, Vec2f viewPos) const;
    void disableScissor() const;

    std::vector<proxy_drawable_t> scene_order;
    std::unordered_set<ID<Drawable>> to_add;
    std::unordered_set<ID<Drawable>> to_erase;
    std::unordered_map<ID<Drawable>, SceneConfig> configs;

    void add_to_scene(World& world);

	ShapeRectangle background;
	Vec2f scene_size;
	Vec2f cam_pos;

};

}