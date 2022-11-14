#include "fastfall/game/AttachSystem.hpp"


#include "fastfall/game/World.hpp"

namespace ff {

    template<class T>
    void update_attachment(World& w, ID<T> id, Vec2f ppos, Vec2f cpos, Vec2f vel) {
        if constexpr (std::same_as<T, Collidable>) {
            // dunno how this'll work lmao
            Collidable& t = w.at(id);
            t.teleport(ppos);
            t.setPosition(cpos);
        }
        else if constexpr (std::same_as<T, Trigger>) {
            Trigger& t = w.at(id);
            auto area = t.get_area();
            area.setPosition(cpos);
            t.set_area(area);
        }
        else if constexpr (std::same_as<T, Emitter>) {
            Emitter& t = w.at(id);
            t.velocity = vel;
            t.prev_position = ppos;
            t.position = cpos;
        }
        else if constexpr (std::same_as<T, AttachPoint>) {
            // or this
            AttachPoint& t = w.at(id);

            // TODO hopefully no stack overflow???
            if (ppos != t.prev_pos()
            || cpos != t.curr_pos()
            || vel != t.vel())
            {
                t.teleport(ppos);
                t.set_pos(cpos);
                t.set_vel(vel);
                w.attach().notify(w, id);
            }
        }
        else if constexpr (std::same_as<T, ColliderRegion>) {
            ColliderRegion& t = w.at(id);
            t.delta_velocity = vel - t.velocity;
            t.velocity = vel;
            t.setPosition(cpos);
        }
        else if constexpr (std::same_as<T, CameraTarget>) {
            CameraTarget& t = w.at(id);
            // ???
        }
    }

    template<class T>
    void interp_attachment(World& w, ID<T> id, Vec2f ipos) {
        if constexpr (std::same_as<T, Drawable>) {
            // Drawable& t = w.at(id);
            // ???
            auto& cfg = w.scene().config(id);
            cfg.rstate.transform = Transform(ipos);
        }
    }

    void AttachSystem::update(World& world, secs deltaTime) {
        for (auto [id, ap] : world.all<AttachPoint>())
        {
            ap.update_prev();
        }
    }

    void AttachSystem::predraw(World& world, float interp, bool updated) {
        for (auto [aid, ap] : world.all<AttachPoint>())
        {
            auto ipos = ap.interpolate(interp);
            for (auto attach : get_attachments(aid))
            {
                std::visit(
                        [&](auto id) { interp_attachment(world, id, ipos + attach.offset); },
                        attach.id);
            }
        }
    }

    void AttachSystem::notify_created(World& world, ID<AttachPoint> id){
        attachments.emplace(id, std::set<Attachment>{});
    }

    void AttachSystem::notify_erased(World& world, ID<AttachPoint> id){
        auto attchs = std::move(attachments.at(id));
        for (auto& at : attchs) {
            erase(at.id);
        }
        attachments.erase(id);
    }

    void AttachSystem::notify_created(World& world, ID<Collidable> id) {
        auto ent = world.get_entity_of(id);
        auto attachid = world.create_attachpoint(ent);
        auto& col = world.at(id);
        col.set_attach_id(attachid);
        auto& attach = world.at(attachid);
        attach.teleport(col.getPrevPosition());
        attach.set_pos(col.getPosition());
        attach.set_vel(col.get_vel());
    }

    void AttachSystem::notify_erased(World& world, ID<Collidable> id) {
        auto& col = world.at(id);
        world.erase(col.get_attach_id());
    }

    void AttachSystem::notify(World& world, ID<AttachPoint> id) {
        if (auto ap = world.get(id);
            ap &&  attachments.contains(id))
        {
            auto& ats = attachments.at(id);

            for (auto& at : ats)
            {
                auto ppos = ap->prev_pos() + at.offset;
                auto cpos = ap->curr_pos() + at.offset;
                std::visit(
                    [&](auto id) { update_attachment(world, id, ppos, cpos, ap->vel()); },
                    at.id
                );
            }
        }
    }

    void AttachSystem::create(ID<AttachPoint> id, ComponentID cmp_id, Vec2f offset)
    {
        attachments.at(id).insert(Attachment{ cmp_id, offset });
    }

    void AttachSystem::erase(ComponentID cmp_id) {
        auto iter = cmp_lookup.find(cmp_id);
        if (iter != cmp_lookup.end())
        {
            auto ap = iter->second;
            attachments.at(ap).erase(Attachment{.id = cmp_id});
            cmp_lookup.erase(iter);
        }
    }

    bool AttachSystem::is_attached(ComponentID cmp_id) const {
        return cmp_lookup.find(cmp_id) != cmp_lookup.end();
    }

    std::optional<ID<AttachPoint>> AttachSystem::get_attachpoint(ComponentID cmp_id) const {
        return is_attached(cmp_id) ? std::make_optional(cmp_lookup.at(cmp_id)) : std::nullopt;
    }

    bool AttachSystem::has_attachments(ID<AttachPoint> id) const {
        return attachments.contains(id) && !attachments.at(id).empty();
    }

    const std::set<AttachSystem::Attachment>&
    AttachSystem::get_attachments(ID<AttachPoint> id) const
    {
        return attachments.at(id);
    }
}