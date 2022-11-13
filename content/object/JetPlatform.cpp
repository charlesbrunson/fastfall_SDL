#include "JetPlatform.hpp"

using namespace ff;

constexpr unsigned platform_width_min = 3;
constexpr unsigned platform_width_max = 5;
AnimIDRef anim_platform[] = {
        {"jet_platform.sax", "platform_3"},
        {"jet_platform.sax", "platform_4"},
        {"jet_platform.sax", "platform_5"},
};

const ObjectType JetPlatform::Type{
        .type = { "JetPlatform" },
        .allow_as_level_data = true,
        .anim = std::nullopt,
        .tile_size = {0, 1},
        .group_tags = {	"platform" },
        .properties = {}
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

JetPlatform::JetPlatform(World& w, ID<GameObject> id, ff::ObjectLevelData& data)
    : ff::GameObject(w, id, data)
{
    tile_width = (int)data.size.x / TILESIZE;
    assert(platform_width_min <= tile_width && tile_width <= platform_width_max);

    base_position = Vec2f{ data.position } - Vec2f{ (float)data.size.x / 2.f, (float)data.size.y };

    collider_id = w.create_collider<ColliderTileMap>(id, Vec2i{tile_width, 1});
    auto& collider = w.at(collider_id);
    collider.fill("oneway"_ts);
    collider.applyChanges();
    collider.set_on_postcontact(
    [id = id_cast<JetPlatform>(getID()), cid = collider_id](World& w, const AppliedContact& c)
    {
        if (c.hasContact && c.id && c.id->collider == id_cast<ColliderRegion>(cid))
        {
            auto [jetpl, collider, collidable] = w.at(id, cid, c.id->collidable);
            jetpl.push_vel.y += (c.collidable_precontact_velocity - collider.velocity).y * 0.9f;

            if (auto* track = collidable.get_tracker();
                track && track->has_contact())
            {
                auto& contact = track->get_contact();
                if (contact->id && contact->id->collider == id_cast<ColliderRegion>(cid)) {
                    jetpl.push_vel.x += collidable.get_friction().x;
                    jetpl.push_accel.x -= collidable.get_acc().x * 0.5f;
                }
            }
        }
    });

    sprite_id = w.create_drawable<AnimatedSprite>(id);
    auto& sprite = w.at(sprite_id);
    sprite.set_anim(anim_platform[tile_width - platform_width_min]);
    w.scene().config(sprite_id) = {
        .layer_id = 0,
        .type = scene_type::Object,
        .priority = scene_priority::Low,
    };

    emitter_id = w.create_emitter(id);
    auto& emitter = w.at(emitter_id);
    emitter.strategy = jet_emitter_str;
    w.scene().config(emitter.get_vertexarray()) = {
        .layer_id = 0,
        .type = scene_type::Object,
        .priority = scene_priority::Lowest,
    };

    attach_id = w.create_attachpoint(id);
    auto& attach = w.at(attach_id);
    w.attach().create(attach_id, sprite_id, {});
    w.attach().create(attach_id, collider_id, {});
    w.attach().create(attach_id, emitter_id, { (float)tile_width * TILESIZE_F * 0.5f, TILESIZE_F - 5.f });
    attach.teleport(base_position);
}

void JetPlatform::update(ff::World& w, secs deltaTime)
{
    constexpr Vec2f spring{30.f, 50.f};
    constexpr Vec2f damping{8.f, 3.f};

    if (deltaTime > 0.0) {
        auto [attach, sprite, collider, emitter] = w.at(attach_id, sprite_id, collider_id, emitter_id);
        lifetime += deltaTime;

        // apply accumulated push to velocity
        attach.add_vel(push_vel + (push_accel * (float)deltaTime));
        push_accel = Vec2f{};
        push_vel = Vec2f{};

        // apply forces
        Vec2f accel{};
        Vec2f offset = (attach.curr_pos() - base_position);
        accel += offset.unit() * (-spring * offset.magnitude()); // spring
        accel += attach.vel().unit() * (-damping * attach.vel().magnitude()); // damping

        attach.add_vel(accel * deltaTime);
        attach.update(deltaTime);
        w.attach().notify(w, attach_id);
    }
}

void JetPlatform::predraw(ff::World& w, float interp, bool updated) {
    // auto [attach, sprite] = w.at(attach_id, sprite_id);
    // sprite.set_pos(attach.interpolate(interp));
    // sprite.predraw(interp);
}
