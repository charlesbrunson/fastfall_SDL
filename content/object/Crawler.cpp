#include "Crawler.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/render/drawable/AnimatedSprite.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "../camera/SimpleCamTarget.hpp"

using namespace ff;

const ff::ActorType Crawler::actor_type = ff::ActorType::create<Crawler>({
   .name       = "Crawler",
   .anim       = { "crawler.sax", "north" },
   .tile_size  = { 1u, 1u },
   .priority   = 0,
   .properties = {
       { "face_left",   false },
       { "surface_dir", ff::ObjectProperty::Type::String }
   }
});

cardinal_array<AnimIDRef> anims = {
    AnimIDRef{ "crawler.sax", "north" },
    AnimIDRef{ "crawler.sax", "east"  },
    AnimIDRef{ "crawler.sax", "south" },
    AnimIDRef{ "crawler.sax", "west"  },
};

constexpr Vec2f grav_falling   = {0.f, 500.f};
constexpr float grav_attached  = 100.f;
constexpr float move_max_speed = 125.f;
constexpr float move_accel     = 750.f;
constexpr float jump_vel       = 150.f;

Crawler::Crawler(ActorInit init, Vec2f position, Cardinal t_surface_dir, bool face_left)
    : Actor(init.type_or(&actor_type))
    , spr_id(init.world.create<AnimatedSprite>(init.entity_id))
    , col_id(init.world.create<Collidable>(init.entity_id, position, Vec2f{ 14.f, 14.f }, grav_falling))
    , surface_dir(t_surface_dir)
{
    World& w = init.world;
    auto [spr, col] = w.at(spr_id, col_id);

    col.create_tracker(
        Angle::Degree(std::nextafterf(-180.f, 0.f)),
        Angle::Degree( 180.f)
    );

    col.tracker()->settings = {
        .move_with_platforms = true,
        .slope_sticking      = true,
        .slope_wall_stop     = false,
        .has_friction        = true,
        .use_surf_vel        = true,
        .stick_angle_max     = Angle::Degree(180.f),
        .surface_friction = {
            .stationary = 3.0f,
            .kinetic = 2.0f
        },
        .max_speed           = move_max_speed,
        .slope_stick_speed_factor = 0.f,
    };

    spr.set_anim(anims[surface_dir]);
    spr.set_hflip(face_left);

    move_dir = !face_left ? 1 : -1;

    w.system<AttachSystem>().create(w, col.get_attach_id(), spr_id);

    col.tracker()->callbacks.on_stick = [](Collidable& c, auto result, const ColliderSurface& surf) {
        auto ang = result.path.diff_angle.degrees();
        if (ang != 0.f && ((result.travel_dir > 0.f) == (ang > 0.f))) {
            // outer corner
            c.set_gravity(direction::to_vector<float>(surf.id.dir) * -grav_attached);
        } else {
            // inner corner
            c.set_gravity(direction::to_vector<float>(surf.id.dir) * -grav_attached);
        }
    };

    // DEBUG

    w.create<SimpleCamTarget>(
        init.entity_id,
        ff::CamTargetPriority::Medium,
        [id = col_id](World& w) {
            return w.at(id).getPosition() - Vec2f{0.f, 16.f};
        }
    );

}

Crawler::Crawler(ActorInit init, const LevelObjectData& data)
    : Crawler(init,
              data.area.botmid(),
              direction::cardinal_from_str(data.get_prop<std::string>("surface_dir")).value_or(Cardinal::N),
              data.get_prop<bool>("face_left"))
{
}

void Crawler::update(ff::World& w, secs deltaTime)
{
    auto [col, spr] = w.at(col_id, spr_id);
    auto& tracker   = *col.tracker();

    move_dir = 0;
    if (w.input(Input::Left).is_held()) {
        move_dir += -1;
    }
    else if (w.input(Input::Right).is_held()) {
        move_dir += 1;
    }

    if (tracker.has_contact()) {
        auto& contact = *tracker.get_contact();

        if (w.input(Input::Jump).if_confirm_press(0.1)) {
            tracker.force_end_contact();
            col.set_local_vel(math::projection(col.get_local_vel(), contact.collider_n.righthand()) + (contact.collider_n * jump_vel));
            col.set_gravity(grav_falling);
        }
        else {
            col.set_gravity(contact.ortho_n * -grav_attached );
            if (move_dir != 0) {
                tracker.traverse_add_accel(move_accel * move_dir);
            }
        }
        surface_dir = *direction::from_vector(contact.ortho_n);
    }
    else {
        col.set_gravity(grav_falling);
        surface_dir = Cardinal::N;
    }
    spr.set_anim_if_not(anims[surface_dir]);
}