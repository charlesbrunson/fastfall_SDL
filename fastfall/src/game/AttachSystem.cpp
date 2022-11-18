#include <utility>

#include "fastfall/game/AttachSystem.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

    template<class T>
    Vec2f update_attachment(World& w, const AttachPoint& attachpoint, ID<T> attachment_id, Vec2f offset, secs deltaTime) {

        Vec2f ppos = attachpoint.prev_pos() + offset;
        Vec2f cpos = attachpoint.curr_pos() + offset;
        Vec2f vel = attachpoint.vel();

        if constexpr (std::same_as<T, Collidable>) {
            Collidable& t = w.at(attachment_id);
            t.setPosition(cpos);
            return t.getPosition();
        }
        else if constexpr (std::same_as<T, Trigger>) {
            Trigger& t = w.at(attachment_id);
            auto area = t.get_area();
            area.setPosition(cpos);
            t.set_area(area);
            return area.getPosition();
        }
        else if constexpr (std::same_as<T, Emitter>) {
            Emitter& t = w.at(attachment_id);
            t.velocity = vel;
            t.prev_position = ppos;
            t.position = cpos;
            return t.position;
        }
        else if constexpr (std::same_as<T, AttachPoint>) {
            // or this
            AttachPoint& t = w.at(attachment_id);

            // TODO hopefully no stack overflow???
            if (cpos != t.curr_pos()
             || vel != t.vel())
            {
                if (t.constraint) {
                    t.constraint(t, attachpoint, offset, deltaTime);
                }
                else {
                    t.set_pos(cpos);
                    t.set_vel(vel);
                }
                t.set_tick(w.tick_count());
            }
            return t.curr_pos();
        }
        else if constexpr (std::same_as<T, ColliderRegion>) {
            ColliderRegion& t = w.at(attachment_id);
            t.delta_velocity = vel - t.velocity;
            t.velocity = vel;
            t.setPosition(cpos);
            return t.getPosition();
        }
        else if constexpr (std::same_as<T, CameraTarget>) {
            // TODO ???
            return {};
        }
        else if constexpr (std::same_as<T, Drawable>) {
            //SceneConfig& cfg = w.scene().config(attachment_id);
            return cpos + offset;
        }
        return {};
    }

    template<class T>
    void interp_attachment(World& w, ID<T> id, Vec2f ipos) {
        if constexpr (std::same_as<T, Drawable>) {
            auto& cfg = w.scene().config(id);
            cfg.rstate.transform = Transform(ipos);
        }
        /*
        if constexpr (std::same_as<T, Emitter>) {
            Emitter& e = w.at(id);
            auto& cfg = w.scene().config(e.get_drawid());
            cfg.rstate.transform = Transform(ipos);
        }
        */
    }

    void AttachSystem::update(World& world, secs deltaTime) {
        if (deltaTime > 0.0) {
            curr_delta = deltaTime;
            for (auto [id, ap]: world.all<AttachPoint>()) {
                ap.update_prev();
            }
        }
    }

    void AttachSystem::update_attachpoints(World& world, secs deltaTime, AttachPoint::Schedule sched) {
        if (deltaTime > 0.0) {
            for (auto [aid, ap]: world.all<AttachPoint>()) {

                if (is_attachpoint_root(aid)
                    && ap.get_tick() != world.tick_count()
                    && ap.sched == sched)
                {
                    std::set<ID<AttachPoint>> visited;
                    ap.set_tick(world.tick_count());
                    update_attachments(world, aid, visited);
                }
            }
        }
    }

    void AttachSystem::update_attachments(World& world, ID<AttachPoint> id, std::set<ID<AttachPoint>>& visited) {
        if (visited.contains(id))
            return;

        visited.insert(id);
        auto& ap = world.at(id);
        for (auto at : attachments.at(id))
        {
            Vec2f p;
            std::visit(
                [&]<class T>(ID<T> c_id) { p = update_attachment(world, ap, c_id, at.offset, curr_delta); },
                        at.id);

            if (holds_alternative<ID<AttachPoint>>(at.id)) {
                update_attachments(world, std::get<ID<AttachPoint>>(at.id), visited);
            }

            if (debug_draw::hasTypeEnabled(debug_draw::Type::ATTACH)  /*&& !debug_draw::repeat((void *) &at, p) */ ) {

                //debug_draw::set_offset(p);
                auto &attach = createDebugDrawable<VertexArray, debug_draw::Type::ATTACH>(
                        (const void *) &at, Primitive::LINES, 6);

                for (auto ndx = 0; ndx < attach.size(); ++ndx) {
                    attach[ndx].color = Color::Green;
                }

                attach[0].pos = ap.curr_pos() + Vec2f{-2, -2};
                attach[1].pos = ap.curr_pos() + Vec2f{ 2,  2};
                attach[2].pos = ap.curr_pos() + Vec2f{-2,  2};
                attach[3].pos = ap.curr_pos() + Vec2f{ 2, -2};
                attach[4].pos = ap.curr_pos();
                attach[5].pos = p;
                //debug_draw::set_offset();
            }

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

    bool AttachSystem::is_attachpoint_root(ID<AttachPoint> id) const {
        return !cmp_lookup.contains(id);
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
        attach.sched = AttachPoint::Schedule::PostCollision;
    }

    void AttachSystem::notify_erased(World& world, ID<Collidable> id) {
        auto& col = world.at(id);
        world.erase(col.get_attach_id());
    }

    void AttachSystem::notify_created(World& world, ID<PathMover> id) {
        auto ent = world.get_entity_of(id);
        auto attachid = world.create_attachpoint(ent);
        auto& pathmover = world.at(id);
        pathmover.set_attach_id(attachid);
        auto& attach = world.at(attachid);
        attach.teleport(pathmover.get_pos());
        attach.set_pos(pathmover.get_pos());
        attach.set_vel(pathmover.get_vel());
    }

    void AttachSystem::notify_erased(World& world, ID<PathMover> id) {
        auto& col = world.at(id);
        world.erase(col.get_attach_id());
    }

    void AttachSystem::create(World& world, ID<AttachPoint> id, ComponentID cmp_id, Vec2f offset)
    {
        attachments.at(id).insert(Attachment{ cmp_id, offset });
        cmp_lookup.emplace(cmp_id, id);
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
        return cmp_lookup.contains(cmp_id);
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