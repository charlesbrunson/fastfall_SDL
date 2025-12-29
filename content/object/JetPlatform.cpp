#include "JetPlatform.hpp"

using namespace ff;

constexpr int platform_width_min = 3;
constexpr int platform_width_max = 5;
AnimIDRef anim_platform[] = {
    {"jet_platform.sax", "platform_3"},
    {"jet_platform.sax", "platform_4"},
    {"jet_platform.sax", "platform_5"},
};

const EmitterStrategy jet_emitter_str = {
    .emit_rate_min      = 60,
    .emit_rate_max      = 60,
    .emit_count_min     = 1,
    .emit_count_max     = 1,
    .max_lifetime       = 0.120,
    .max_particles      = -1,
    //.direction          = Angle::Degree(90.f),
    .open_angle_degrees = 4.f,
    .particle_speed_min = 250.f,
    .particle_speed_max = 350.f,
    .scatter_max_radius = 0.f,
    .inherits_vel       = 0.f,
    // .inherits_acc       = -5.f,
    .move_with_emitter  = true,
    .collision_enabled  = true,
    .collision_destroys = true,
    .constant_accel     = {},
    .animation          = AnimIDRef{ "jet_platform.sax", "effect" },
    .event_captures     = { ParticleEventType::Collide }
};

const EmitterStrategy smoke_emitter_str = {
        .emission_enabled               = false,

        .emit_rate_min                  = 0,
        .emit_rate_max                  = 0,

        .emit_count_min                 = 1,
        .emit_count_max                 = 1,

        .burst_count_min                = 1,
        .burst_count_max                = 1,
        .burst_chance                   = 1.f,

        .max_lifetime                   = 0.25,
        .max_particles                  = -1,

        //.direction                      = Angle::Radian(0.f),
        .open_angle_degrees             = 60.f,

        .particle_speed_min             = 0.f,
        .particle_speed_max             = 50.f,

        .particle_damping               = 0.5f,

        .local_spawn_area               = {},
        .scatter_max_radius             = 0.f,

        .inherits_vel                   = false,

        .collision_enabled              = false,

        .draw_order                     = ParticleDrawOrder::NewestFirst,
        .constant_accel                 = { Vec2f{ 0.f, -250.f } },
        .animation                      = AnimIDRef{ "jet_platform.sax", "smoke" },
};

const ActorType JetPlatform::actor_type = ActorType::create<JetPlatform>({
    .name       = "JetPlatform",
    .anim       = {},
    .tile_size  = { 0, 1 },
    .group_tags = {	"platform" },
    .properties = {
        { "path",  ObjLevelID{ ObjLevelID::NO_ID } }
    },
});

JetPlatform::JetPlatform(ActorInit init, const LevelObjectData& data)
    : JetPlatform(init, data.area.topleft(), (int)data.area.getSize().x / TILESIZE, data.get_prop<ObjLevelID>("path"))
{
}

JetPlatform::JetPlatform(ff::ActorInit init, ff::Vec2f pos, int width, ff::ObjLevelID path_objid)
    : Actor(init.type_or(&actor_type))
{
    Vec2f base_position = pos;
    int   tile_width    = std::clamp(width, platform_width_min, platform_width_max);

    World& w = init.world;

    // collider
    auto collider = w.create<ColliderTileMap>(entity_id, Vec2i{tile_width, 1});
    collider->fill("oneway"_ts);
    collider->applyChanges();

    // sprite
    auto sprite = w.create<AnimatedSprite>(entity_id);
    sprite->set_anim(anim_platform[tile_width - platform_width_min]);
    w.system<SceneSystem>().config(sprite) = {
        .layer_id = 0,
        .type     = scene_type::Object,
        .priority = scene_priority::Low
    };

    // emitters
    auto jet_emitter = w.create<Emitter>(entity_id);
    jet_emitter->strategy   = jet_emitter_str;
    jet_emitter->emit_angle = Angle::Degree(90.f);
    w.system<SceneSystem>().config(jet_emitter->get_drawid()) = {
        .layer_id = 0,
        .type     = scene_type::Object,
        .priority = scene_priority::Lowest,
    };

    auto smoke_emitter = w.create<Emitter>(entity_id);
    smoke_emitter->strategy = smoke_emitter_str;
    w.system<SceneSystem>().config(smoke_emitter->get_drawid()) = {
        .layer_id = 0,
        .type     = scene_type::Object,
        .priority = scene_priority::Low,
    };

    jet_emitter->strategy.events_callback = [ s_id = smoke_emitter.id ](World& w, std::span<const ParticleEvent> events) {
        auto& smoke_emitter = w.at(s_id);
        for (auto &e : events) {
            if (e.particle.id % 6 == 0) {
                smoke_emitter.burst(e.particle.position, e.particle.velocity, math::angle(*e.particle.collision_normal));
            }
        }
    };

    // spring attachpoint
    auto attach = w.create<AttachPoint>(entity_id, id_placeholder);
    attach->teleport(base_position);
    attach->constraint = makeSpringConstraint({30, 50}, {8, 3}, 48.f);
    w.system<AttachSystem>().attach_component(w, attach, sprite);
    w.system<AttachSystem>().attach_component(w, attach, collider);
    w.system<AttachSystem>().attach_component(w, attach, jet_emitter, { (float)tile_width * TILESIZE_F * 0.5f, TILESIZE_F - 5.f });
    // attach->sched = AttachPoint::Schedule::PostCollision;

    // base attachpoint
    Level* active_level = w.system<LevelSystem>().get_active(w);
    ID<AttachPoint> base_attach_id;
    if (active_level && path_objid) {
        Path p = active_level->get_obj_layer().getObjectDataByID(path_objid);
        base_attach_id = w.create<PathMover>(entity_id, p)->get_attach_id();
    }
    else {
        base_attach_id = w.create<AttachPoint>(entity_id, id_placeholder, base_position);
    }

    auto& base_attach = w.at(base_attach_id);
    base_attach.teleport(base_position);
    w.system<AttachSystem>().attach_component(w, base_attach_id, attach);

    collider->set_on_postcontact(
    [
        id  = id_cast<JetPlatform>(actor_id),
        cid = collider.id,
        aid = attach.id
    ]
        (World& w, const AppliedContact& c, secs deltaTime)
    {
        if (c.has_contact_with(cid))
        {
            auto [attach, collider, collidable] = w.at(aid, cid, c.id->collidable);

            Vec2f precontact_vel = c.collidable_precontact_velocity - attach.global_vel();
            precontact_vel.y = std::max(0.f, precontact_vel.y);
            Vec2f push_vel{};
            Vec2f push_acc{};
            push_vel.y += precontact_vel.y * 0.9f;

            if (auto& track = collidable.tracker();
                track && track->has_contact_with(cid))
            {
                // auto& contact = track->get_contact();
                push_vel.x += collidable.get_friction().x;
                push_acc.x -= collidable.get_accel().x * 0.5f;
            }

            attach.set_local_vel(attach.local_vel() + push_vel + (push_acc * (float)deltaTime));
        }
    });
    w.system<AttachSystem>().notify_teleport(w, base_attach_id);
}


