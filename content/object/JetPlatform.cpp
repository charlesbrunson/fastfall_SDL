#include "JetPlatform.hpp"

using namespace ff;

AnimIDRef anim_platform("jet_platform.sax", "platform");
AnimIDRef anim_jetfx("jet_platform.sax", "effect");

const ObjectType JetPlatform::Type{
        .type = { "JetPlatform" },
        .allow_as_level_data = true,
        .anim = std::nullopt,
        .tile_size = {3, 1},
        .group_tags = {	"platform" },
        .properties = {}
};

JetPlatform::JetPlatform(World& w, ID<GameObject> id, ff::ObjectLevelData& data)
        : ff::GameObject(w, id, data)
        , scene_id(w.create_scene_object({
            .drawable = make_copyable_unique<Drawable, AnimatedSprite>(),
            .layer_id = 1,
            .type = scene_type::Object }
        ))
        , collider_id{
            w.create_collider<ColliderTileMap>(Vec2i{3, 1})
        }
{
    sprite_id = id_cast<AnimatedSprite>(scene_id);

    auto [sprite, collider] = w.at(sprite_id, collider_id);

    sprite.set_anim(anim_platform);

    collider.setTile(Vec2i{0, 0}, "oneway"_ts);
    collider.setTile(Vec2i{1, 0}, "solid"_ts);
    collider.setTile(Vec2i{2, 0}, "oneway"_ts);
    collider.applyChanges();

    auto colliderRect = Rectf{ Vec2f{}, Vec2f{ data.size } };
    auto pos = Vec2f{ data.position };
    auto off = Vec2f{ colliderRect.width / 2.f, colliderRect.height };

    base_position = pos - off;

    collider.teleport(base_position);
    sprite.set_pos(base_position);

    collider.set_on_postcontact([id = id_cast<JetPlatform>(getID()), cid = collider_id](World& w, const AppliedContact& c) {
        auto [jetpl, collider] = w.at(id, cid);

        if (c.hasContact && c.ortho_n == Vec2f{0.f, -1.f} && c.id)
        {
            auto& collidable = w.at(c.id->collidable);
            jetpl.push.y += collidable.get_vel().y * 0.9f;
        }
    });
}

void JetPlatform::update(ff::World& w, secs deltaTime) {
    auto [sprite, collider] = w.at(sprite_id, collider_id);

    lifetime += deltaTime;

    Vec2f pos = base_position;
    pos.y += (-cosf(lifetime * 2.f) + 1.f) * 4.f;

    offset += push * deltaTime;
    push.y = math::reduce(push.y, 300.f * (float)deltaTime, 0.f);

    if (push.y == 0.f)
    {
        offset.y = math::reduce(offset.y, 300.f * (float)deltaTime, 0.f);
    }

    //sprite.set_pos(pos);
    collider.setPosition(pos + offset);

    ff::Vec2f nVel = (collider.getPosition() - collider.getPrevPosition()) / deltaTime;
    collider.delta_velocity = nVel - collider.velocity;
    collider.velocity = nVel;

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