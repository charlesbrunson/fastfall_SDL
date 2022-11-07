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
        , scene_id(w.create_scene_object({
            .drawable = make_copyable_unique<Drawable, AnimatedSprite>(),
            .layer_id = 0,
            .type     = scene_type::Object,
            .priority = scene_priority::Lowest
        }))
        , collider_id{}
{
    sprite_id = id_cast<AnimatedSprite>(scene_id);


    tile_width = data.size.x / TILESIZE;
    assert(platform_width_min <= tile_width && tile_width <= platform_width_max);

    collider_id = w.create_collider<ColliderTileMap>(Vec2i{(int)tile_width, 1});
    auto [sprite, collider] = w.at(sprite_id, collider_id);

    sprite.set_anim(anim_platform[tile_width - platform_width_min]);

    for (int i = 0; i < tile_width; ++i) {
        collider.setTile(Vec2i{i, 0}, "oneway"_ts);
    }
    collider.applyChanges();

    auto colliderRect = Rectf{ Vec2f{}, Vec2f{ data.size } };
    auto pos = Vec2f{ data.position };
    auto off = Vec2f{ colliderRect.width / 2.f, colliderRect.height };

    base_position = pos - off;
    position = base_position;

    collider.teleport(base_position);
    sprite.set_pos(base_position);

    collider.set_on_postcontact([id = id_cast<JetPlatform>(getID()), cid = collider_id](World& w, const AppliedContact& c) {

        auto [jetpl, collider] = w.at(id, cid);

        if (c.hasContact && c.ortho_n == Vec2f{0.f, -1.f} && c.id)
        {
            auto& collidable = w.at(c.id->collidable);

            auto vel = c.collidable_precontact_velocity;
            vel.x /= -3.f;
            Vec2f acc = (vel - collider.velocity);
            acc.x /= 2.f;
            jetpl.push_accum += acc;
        }

    });
}

void JetPlatform::update(ff::World& w, secs deltaTime) {
    auto [sprite, collider] = w.at(sprite_id, collider_id);

    if (deltaTime > 0.0) {
        lifetime += deltaTime;

        // apply accumulated push to velocity
        velocity += push_accum;
        push_accum = Vec2f{};

        constexpr Vec2f spring{50.f, 50.f};
        constexpr Vec2f damping{5.f, 3.5f};

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

    }
    sprite.update(deltaTime);
    collider.update(deltaTime);
}

void JetPlatform::predraw(ff::World& w, float interp, bool updated) {
    auto [sprite, collider] = w.at(sprite_id, collider_id);
    sprite.set_pos(math::lerp(collider.getPrevPosition(), collider.getPosition(), interp));
    sprite.predraw(interp);
}

void JetPlatform::clean(ff::World& w) {
    w.erase(scene_id);
    w.erase(collider_id);
}