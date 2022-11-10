#include "JetPlatform.hpp"

using namespace ff;

constexpr unsigned platform_width_min = 3;
constexpr unsigned platform_width_max = 5;
AnimIDRef anim_platform[] = {
        {"jet_platform.sax", "platform_3"},
        {"jet_platform.sax", "platform_4"},
        {"jet_platform.sax", "platform_5"},
};


AnimIDRef anim_jetfx("jet_platform.sax", "effect");

const ObjectType JetPlatform::Type{
        .type = { "JetPlatform" },
        .allow_as_level_data = true,
        .anim = std::nullopt,
        .tile_size = {0, 1},
        .group_tags = {	"platform" },
        .properties = {}
};

JetPlatform::JetPlatform(World& w, ID<GameObject> id, ff::ObjectLevelData& data)
        : ff::GameObject(w, id, data)
        , scene_spr_id(w.create_scene_object({
            .drawable = make_copyable_unique<Drawable, AnimatedSprite>(),
            .layer_id = 0,
            .type     = scene_type::Object,
            .priority = scene_priority::Low
        }))

        , scene_emi_id(w.create_scene_object({
             .drawable = make_copyable_unique<Drawable, Emitter>(),
             .layer_id = 0,
             .type     = scene_type::Object,
             .priority = scene_priority::Lowest
         }))

        , collider_id{}
{
    sprite_id  = id_cast<AnimatedSprite>(scene_spr_id);
    emitter_id = id_cast<Emitter>(scene_emi_id);

    tile_width = data.size.x / TILESIZE;
    assert(platform_width_min <= tile_width && tile_width <= platform_width_max);

    collider_id = w.create_collider<ColliderTileMap>(Vec2i{(int)tile_width, 1});
    auto [sprite, collider, emitter] = w.at(sprite_id, collider_id, emitter_id);
    //auto [sprite, collider] = w.at(sprite_id, collider_id);

    sprite.set_anim(anim_platform[tile_width - platform_width_min]);

    for (int i = 0; i < tile_width; ++i) {
        collider.setTile(Vec2i{i, 0}, "oneway"_ts);
    }
    collider.applyChanges();

    auto colliderRect = Rectf{ Vec2f{}, Vec2f{ data.size } };
    base_position = Vec2f{ data.position } - Vec2f{ colliderRect.width / 2.f, colliderRect.height };
    position = base_position;

    collider.teleport(base_position);
    sprite.set_pos(base_position);

    collider.set_on_postcontact(
    [id = id_cast<JetPlatform>(getID()), cid = collider_id](World& w, const AppliedContact& c)
    {
        auto [jetpl, collider] = w.at(id, cid);
        if (c.hasContact && c.id && c.id->collider == id_cast<ColliderRegion>(cid))
        {
            auto& collidable = w.at(c.id->collidable);

            jetpl.push_accum.y += (c.collidable_precontact_velocity - collider.velocity).y * 0.9f;

            for (auto [tid, track] : collidable.get_trackers()) {
                if (auto& contact = track.get_contact()) {
                    if (contact->id && contact->id->collider == id_cast<ColliderRegion>(cid)) {
                        jetpl.push_accum.x += collidable.get_friction().x;
                        jetpl.push_accel -= collidable.get_acc().x * 0.5f;
                    }
                }
            }
        }
    });

    emitter.strategy.scatter_max_radius = 1.f;
    emitter.strategy.emit_rate_min = 50;
    emitter.strategy.emit_rate_max = 50;
    emitter.strategy.emit_count_min = 1;
    emitter.strategy.emit_count_max = 2;
    emitter.strategy.open_angle_degrees = 5.f;
    emitter.strategy.direction = Angle::Degree(90.f);
    emitter.strategy.max_lifetime = 0.1;
    emitter.strategy.max_particles = -1;
    emitter.strategy.particle_speed_min = 400.f;
    emitter.strategy.particle_speed_max = 600.f;
    emitter.strategy.inherits_vel = true;
    emitter.strategy.animation = AnimIDRef{ "jet_platform.sax", "effect" };

}

void JetPlatform::update(ff::World& w, secs deltaTime) {
    auto [sprite, collider, emitter] = w.at(sprite_id, collider_id, emitter_id);
    //auto [sprite, collider] = w.at(sprite_id, collider_id);

    if (deltaTime > 0.0) {
        lifetime += deltaTime;


        // apply accumulated push to velocity
        velocity.x += push_accum.x + (push_accel * (float)deltaTime);
        velocity.y += push_accum.y;
        push_accel = 0.f;
        push_accum = Vec2f{};

        constexpr Vec2f spring{30.f, 50.f};
        constexpr Vec2f damping{8.f, 3.f};

        Vec2f accel;

        // apply spring force
        Vec2f offset = (position - base_position);
        accel.x += offset.unit().x * (-spring.x * offset.magnitude());
        accel.y += offset.unit().y * (-spring.y * offset.magnitude());

        // apply damping force
        accel.x += velocity.unit().x * (-damping.x * velocity.magnitude());
        accel.y += velocity.unit().y * (-damping.y * velocity.magnitude());

        velocity += accel * deltaTime;

        collider.delta_velocity = velocity - collider.velocity;
        collider.velocity = velocity;

        position += velocity * deltaTime;
        collider.setPosition(position);

        emitter.position = collider.getPosition();
        emitter.position += Vec2f{(float)tile_width * TILESIZE_F * 0.5f, TILESIZE_F};
        emitter.velocity = collider.velocity;
        emitter.update(deltaTime);
    }
    //sprite.update(deltaTime);
    //collider.update(deltaTime);
}

void JetPlatform::predraw(ff::World& w, float interp, bool updated) {
    auto [sprite, collider, emitter] = w.at(sprite_id, collider_id, emitter_id);
    //auto [sprite, collider] = w.at(sprite_id, collider_id);
    sprite.set_pos(math::lerp(collider.getPrevPosition(), collider.getPosition(), interp));
    sprite.predraw(interp);
    emitter.predraw(interp);
}

void JetPlatform::clean(ff::World& w) {
    w.erase(scene_spr_id);
    w.erase(scene_emi_id);
    w.erase(collider_id);
}