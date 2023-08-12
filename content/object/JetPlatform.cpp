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
    .emit_rate_min      = 200,
    .emit_rate_max      = 200,
    .emit_count_min     = 1,
    .emit_count_max     = 1,
    .max_lifetime       = 0.1,
    .max_particles      = -1,
    .direction          = Angle::Degree(90.f),
    .open_angle_degrees = 3.f,
    .particle_speed_min = 400.f,
    .particle_speed_max = 600.f,
    .scatter_max_radius = 0.f,
    .inherits_vel       = false,
    .animation          = AnimIDRef{ "jet_platform.sax", "effect" },
};

const ActorType JetPlatform::actor_type = ActorType::create<JetPlatform>({
    .name       = "JetPlatform",
    .anim       = {},
    .tile_size  = { 0, 1 },
    .group_tags = {	"platform" },
    .properties = {
        { "path",  ObjLevelID{} }
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
        .type = scene_type::Object,
        .priority = scene_priority::Low
    };

    // emitter
    auto emitter = w.create<Emitter>(entity_id);
    emitter->strategy = jet_emitter_str;
    w.system<SceneSystem>().config(emitter->get_drawid()) = {
        .layer_id = 0,
        .type = scene_type::Object,
        .priority = scene_priority::Lowest,
    };

    // spring attachpoint
    auto attach = w.create<AttachPoint>(entity_id, id_placeholder);
    attach->teleport(base_position);
    attach->constraint = makeSpringConstraint({30, 50}, {8, 3}, 48.f);
    w.system<AttachSystem>().create(w, attach, sprite);
    w.system<AttachSystem>().create(w, attach, collider);
    w.system<AttachSystem>().create(w, attach, emitter, { (float)tile_width * TILESIZE_F * 0.5f, TILESIZE_F - 5.f });
    attach->sched = AttachPoint::Schedule::PostCollision;

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
    w.system<AttachSystem>().create(w, base_attach_id, attach);

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
                auto& contact = track->get_contact();
                push_vel.x += collidable.get_friction().x;
                push_acc.x -= collidable.get_acc().x * 0.5f;
            }

            attach.set_local_vel(attach.local_vel() + push_vel + (push_acc * (float)deltaTime));
        }
    });
}


