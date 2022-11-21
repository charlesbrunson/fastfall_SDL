#include <utility>

#include "fastfall/game/AttachSystem.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

    struct AttachState {
        const AttachPoint& parent;
        Vec2f ppos;
        Vec2f cpos;
        Vec2f offset;
        Vec2f vel;
        secs deltaTime;
        size_t tick;
    };

    namespace detail {

        // UPDATES -----------------------------
        // trigger
        void attach_update(World& w, ID<Trigger> id, Trigger& cmp, const AttachState& st) {
            auto area = cmp.get_area();
            area.setPosition(st.cpos);
            cmp.set_area(area);
        }

        // Collidable
        void attach_update(World& w, ID<Collidable> id, Collidable& cmp, const AttachState& st) {
            cmp.teleport(st.ppos);
            cmp.setPosition(st.cpos);
            cmp.set_vel(st.vel);
        }

        // Emitter
        void attach_update(World& w, ID<Emitter> id, Emitter& cmp, const AttachState& st) {
            cmp.velocity = st.vel;
            cmp.prev_position = st.ppos;
            cmp.position = st.cpos;
        }

        // AttachPoint
        void attach_update(World& w, ID<AttachPoint> id, AttachPoint& cmp, const AttachState& st) {
            if (st.cpos != cmp.curr_pos()
                || st.vel != cmp.vel())
            {
                if (cmp.constraint) {
                    cmp.constraint(cmp, st.parent, st.offset, st.deltaTime);
                }
                else {
                    cmp.set_pos(st.cpos);
                    cmp.set_vel(st.vel);
                }
                cmp.set_tick(st.tick);
            }
        }

        // ColliderRegion
        void attach_update(World& w, ID<ColliderRegion> id, ColliderRegion& cmp, const AttachState& st) {
            cmp.delta_velocity = (st.vel - cmp.velocity); // + cmp.delta_velocity;
            cmp.velocity = st.vel;
            cmp.teleport(st.ppos);
            cmp.setPosition(st.cpos);
        }

        // CameraTarget
        void attach_update(World& w, ID<CameraTarget> id, CameraTarget& cmp, const AttachState& st) {
            // ???
        }

        // Drawable
        void attach_update(World& w, ID<Drawable> id, Drawable& cmp, const AttachState& st) {
            w.scene().config(id).prev_pos = st.ppos;
            w.scene().config(id).curr_pos = st.cpos;
        }

        // PathMover
        void attach_update(World& w, ID<PathMover> id, PathMover& cmp, const AttachState& st) {
            cmp.set_path_offset(st.cpos);
        }

        // POSITION -----------------------------
        // Trigger
        Vec2f attach_get_pos(World& w, ID<Trigger> id, Trigger& cmp) {
            return cmp.get_area().getPosition();
        }

        // Collidable
        Vec2f attach_get_pos(World& w, ID<Collidable> id, Collidable& cmp) {
            return cmp.getPosition();
        }

        // Emitter
        Vec2f attach_get_pos(World& w, ID<Emitter> id, Emitter& cmp) {
            return cmp.position;
        }

        // AttachPoint
        Vec2f attach_get_pos(World& w, ID<AttachPoint> id, AttachPoint& cmp) {
            return cmp.curr_pos();
        }

        // ColliderRegion
        Vec2f attach_get_pos(World& w, ID<ColliderRegion> id, ColliderRegion& cmp) {
            return cmp.getPosition();
        }

        // CameraTarget
        Vec2f attach_get_pos(World& w, ID<CameraTarget> id, CameraTarget& cmp) {
            return cmp.get_target_pos();
        }

        // Drawable
        Vec2f attach_get_pos(World& w, ID<Drawable> id, Drawable& cmp) {
            return w.scene().config(id).curr_pos;
        }

        // PathMover
        Vec2f attach_get_pos(World& w, ID<PathMover> id, PathMover& cmp) {
            return cmp.get_path_offset();
        }
    }

    template<class T>
    Vec2f update_attachment(World& w, const AttachPoint& attachpoint, ID<T> attachment_id, Vec2f offset, secs deltaTime)
    {
        auto& component = w.at(attachment_id);
        AttachState state {
            .parent = attachpoint,
            .ppos = attachpoint.prev_pos() + offset,
            .cpos = attachpoint.curr_pos() + offset,
            .offset = offset,
            .vel = attachpoint.vel(),
            .deltaTime = deltaTime,
            .tick = w.tick_count()
        };

        detail::attach_update(w, attachment_id, component, state);
        return detail::attach_get_pos(w, attachment_id, component);
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