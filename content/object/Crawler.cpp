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

Crawler::Crawler(ActorInit init, Vec2f position, Cardinal surface_dir, bool face_left)
    : Actor(init.type_or(&actor_type))
    , spr_id(init.world.create<AnimatedSprite>(init.entity_id))
    , col_id(init.world.create<Collidable>(init.entity_id, position, Vec2f{ 14.f, 14.f }, Vec2f{ 0.f, 500.f }))
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
        .stick_angle_max = Angle::Degree(180.f),
        .max_speed = 100.f,
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
    auto& col = w.at(col_id);
    auto& tracker = *col.tracker();

    if (tracker.has_contact()) {
        col.set_gravity(tracker.get_contact()->ortho_n * -500.f );
        tracker.traverse_set_speed(100.f * move_dir);
    }
    else {
        Vec2f vel = col.get_local_vel();
        vel.x = math::reduce(vel.x, static_cast<float>(deltaTime * 100.f), 0.f);
        col.set_local_vel(vel);

        col.set_gravity({ 0, 500.f });
    }
}