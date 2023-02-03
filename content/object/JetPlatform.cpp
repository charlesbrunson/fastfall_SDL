#include "JetPlatform.hpp"

using namespace ff;

constexpr unsigned platform_width_min = 3;
constexpr unsigned platform_width_max = 5;
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

JetPlatform::JetPlatform(ActorInit init, ff::ObjectLevelData& data)
    : ff::Object(init, data)
{
    Vec2f base_position = data.getTopLeftPos();
    int tile_width = (int)data.size.x / TILESIZE;
    assert(platform_width_min <= tile_width && tile_width <= platform_width_max);

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
    ObjLevelID path_id = data.getPropAsID("path");
    ID<AttachPoint> base_attach_id = path_id
            ? w.create<PathMover>(entity_id, Path{data.get_sibling(path_id)})->get_attach_id()
            : w.create<AttachPoint>(entity_id, id_placeholder, data.getTopLeftPos());

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

            Vec2f push_vel{};
            Vec2f push_acc{};
            push_vel.y += (c.collidable_precontact_velocity - attach.global_vel()).y * 0.9f;

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

void JetPlatform::update(ff::World& w, secs deltaTime)
{
}
