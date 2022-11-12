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
    , sprite_id(w.create_drawable<AnimatedSprite>(id))
    , emitter_id(w.create_emitter(id))
    , collider_id{w.create_collider<ColliderTileMap>(id, Vec2i{(int)(data.size.x / TILESIZE), 1})}
{
    tile_width = data.size.x / TILESIZE;
    assert(platform_width_min <= tile_width && tile_width <= platform_width_max);

    base_position = Vec2f{ data.position } - Vec2f{ (float)data.size.x / 2.f, (float)data.size.y };
    position = base_position;

    auto [sprite, collider, emitter] = w.at(sprite_id, collider_id, emitter_id);

    collider.fill("oneway"_ts);
    collider.applyChanges();
    collider.teleport(base_position);
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

    sprite.set_anim(anim_platform[tile_width - platform_width_min]);
    sprite.set_pos(base_position);

    emitter.strategy = jet_emitter_str;

    w.scene().config(sprite_id) = {
        .layer_id = 0,
        .type = scene_type::Object,
        .priority = scene_priority::Low,
    };

    w.scene().config(emitter.get_vertexarray()) = {
        .layer_id = 0,
        .type = scene_type::Object,
        .priority = scene_priority::Lowest,
    };
}

void JetPlatform::update(ff::World& w, secs deltaTime) {

    if (deltaTime > 0.0) {
        auto [sprite, collider, emitter] = w.at(sprite_id, collider_id, emitter_id);
        lifetime += deltaTime;

        // apply accumulated push to velocity
        velocity += push_vel + (push_accel * (float)deltaTime);
        push_accel = Vec2f{};
        push_vel = Vec2f{};

        constexpr Vec2f spring{30.f, 50.f};
        constexpr Vec2f damping{8.f, 3.f};

        Vec2f accel{};

        // apply spring force
        Vec2f offset = (position - base_position);
        accel.x += offset.unit().x * (-spring.x * offset.magnitude());
        accel.y += offset.unit().y * (-spring.y * offset.magnitude());

        // apply damping force
        accel.x += velocity.unit().x * (-damping.x * velocity.magnitude());
        accel.y += velocity.unit().y * (-damping.y * velocity.magnitude());

        prev_position = position;
        velocity += accel * deltaTime;
        position += velocity * deltaTime;

        collider.delta_velocity = velocity - collider.velocity;
        collider.velocity = velocity;
        collider.setPosition(position);

        emitter.prev_position = prev_position;
        emitter.prev_position += Vec2f{(float)tile_width * TILESIZE_F * 0.5f, TILESIZE_F - 5.f};
        emitter.position = position;
        emitter.position += Vec2f{(float)tile_width * TILESIZE_F * 0.5f, TILESIZE_F - 5.f};
        emitter.velocity = collider.velocity;
    }
}

void JetPlatform::predraw(ff::World& w, float interp, bool updated) {
    auto [sprite, collider] = w.at(sprite_id, collider_id);
    sprite.set_pos(math::lerp(collider.getPrevPosition(), collider.getPosition(), interp));
    sprite.predraw(interp);
}
