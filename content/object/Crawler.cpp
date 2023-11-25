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
   .group_tags = { "player" },
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

constexpr Vec2f grav_falling  = {0.f, 500.f};
constexpr float grav_attached = 10.f;
constexpr float move_speed    = 50.f;
constexpr float air_slow_rate = 150.f;

Crawler::Crawler(ActorInit init, Vec2f position, Cardinal surface_dir, bool face_left)
    : Actor(init.type_or(&actor_type))
    , spr_id(init.world.create<AnimatedSprite>(init.entity_id))
    , col_id(init.world.create<Collidable>(init.entity_id, position, Vec2f{ 14.f, 14.f }, grav_falling))
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
        .has_friction        = false,
        .use_surf_vel        = true,
        .stick_angle_max     = Angle::Degree(180.f),
        .max_speed           = move_speed,
        .slope_stick_speed_factor = 0.f,
    };

    spr.set_anim(anims[surface_dir]);
    spr.set_hflip(face_left);

    move_dir = (!face_left ? 1 : -1);

    w.system<AttachSystem>().create(w, col.get_attach_id(), spr_id);

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

    if (w.input()[Input::Left].is_pressed(0.1)) {
        move_dir = -1;
    }
    else if (w.input()[Input::Right].is_pressed(0.1)) {
        move_dir = 1;
    }

    if (tracker.has_contact()) {
        auto& contact = *tracker.get_contact();
        col.set_gravity(contact.ortho_n * -grav_attached );
        tracker.traverse_add_accel(500.f * move_dir);
        // tracker.traverse_set_speed(move_speed * move_dir);

        if (w.input()[Input::Jump].is_pressed(0.1)) {
            col.set_local_vel(contact.ortho_n * 50.f);
        }
        spr.set_anim(anims[*direction::from_vector(contact.ortho_n)]);
    }
    else {
        spr.set_anim(anims[Cardinal::N]);

        Vec2f vel = col.get_local_vel();
        vel.x = math::reduce(vel.x, static_cast<float>(deltaTime * air_slow_rate), 0.f);
        col.set_local_vel(vel);

        col.set_gravity(grav_falling);
    }
}