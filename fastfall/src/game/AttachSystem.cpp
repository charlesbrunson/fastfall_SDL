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
        //Vec2f vel;
        secs deltaTime;
        size_t tick;
    };

    namespace detail {

        // TELEPORT ----------------------------
        // Trigger
        void attach_teleport(World& w, ID<Trigger> id, Trigger& cmp, const AttachPoint& attach, Vec2f offset) {
            auto area = cmp.get_area();
            area.setPosition(attach.curr_pos() + offset);
            cmp.set_area(area);
        }

        // Collidable
        void attach_teleport(World& w, ID<Collidable> id, Collidable& cmp, const AttachPoint& attach, Vec2f offset) {
            cmp.teleport(attach.curr_pos() + offset);
            cmp.set_parent_vel(attach.global_vel());
            cmp.set_local_vel(Vec2f{});
        }

        // Emitter
        void attach_teleport(World& w, ID<Emitter> id, Emitter& cmp, const AttachPoint& attach, Vec2f offset) {
            cmp.velocity = attach.global_vel();
            cmp.prev_position = attach.curr_pos() + offset;
            cmp.position = attach.curr_pos() + offset;
        }

        // AttachPoint
        void attach_teleport(World& w, ID<AttachPoint> id, AttachPoint& cmp, const AttachPoint& attach, Vec2f offset) {
            cmp.teleport(attach.curr_pos() + offset);
            cmp.set_parent_vel(attach.global_vel());
            cmp.set_local_vel({});

            for (auto& sub_attach : w.system<AttachSystem>().get_attachments(id)) {
                std::visit(
                    [&]<class T>(ID<T> cid) {
                        if constexpr (requires(ID<T> x_id, T& x, World& x_w, const AttachPoint& ap, Vec2f x_off) { detail::attach_teleport(x_w, x_id, x, ap, x_off); }) {
                        detail::attach_teleport(w, cid, w.at(cid), w.at(id), sub_attach.offset);
                    }
                }, sub_attach.id);
            }
        }

        // ColliderRegion
        void attach_teleport(World& w, ID<ColliderRegion> id, ColliderRegion& cmp, const AttachPoint& attach, Vec2f offset) {
            cmp.delta_velocity = Vec2f{};
            cmp.velocity = attach.global_vel();
            cmp.teleport(attach.curr_pos() + offset);
            cmp.setPosition(attach.curr_pos() + offset);
        }

        // CameraTarget
        //void attach_teleport(World& w, ID<CameraTarget> id, CameraTarget& cmp, const AttachPoint& attach) {
        //}

        // Drawable
        void attach_teleport(World& w, ID<Drawable> id, Drawable& cmp, const AttachPoint& attach, Vec2f offset) {
            auto& cfg = w.system<SceneSystem>().config(id);
            cfg.prev_pos = attach.curr_pos() + offset;
            cfg.curr_pos = attach.curr_pos() + offset;
        }

        // PathMover
        void attach_teleport(World& w, ID<PathMover> id, PathMover& cmp, const AttachPoint& attach, Vec2f offset) {
            cmp.set_path_offset(attach.curr_pos() + offset);
        }

        // TileLayer
        //void attach_teleport(World& w, ID<Emitter> id, Emitter& cmp, const AttachPoint& attach) {
        //}

        // UPDATES -----------------------------
        // Trigger
        void attach_update(World& w, ID<Trigger> id, Trigger& cmp, const AttachState& st) {
            auto area = cmp.get_area();
            area.setPosition(st.cpos);
            cmp.set_area(area);
        }

        // Collidable
        void attach_update(World& w, ID<Collidable> id, Collidable& cmp, const AttachState& st) {
            cmp.teleport(st.ppos);
            cmp.setPosition(st.cpos);
            cmp.set_parent_vel(st.parent.global_vel());
            cmp.set_local_vel(Vec2f{});
        }

        // Emitter
        void attach_update(World& w, ID<Emitter> id, Emitter& cmp, const AttachState& st) {
            cmp.velocity = st.parent.global_vel();
            cmp.prev_position = st.ppos;
            cmp.position = st.cpos;
        }

        // AttachPoint
        void attach_update(World& w, ID<AttachPoint> id, AttachPoint& cmp, const AttachState& st) {
            if (cmp.get_tick() != st.tick) {
                if (cmp.constraint) {
                    cmp.constraint(cmp, st.parent, st.offset, st.deltaTime);
                } else {
                    cmp.set_pos(st.cpos);
                    cmp.set_parent_vel(st.parent.global_vel());
                    cmp.set_local_vel({});
                }
                cmp.set_tick(st.tick);
            }
        }

        // ColliderRegion
        void attach_update(World& w, ID<ColliderRegion> id, ColliderRegion& cmp, const AttachState& st) {
            cmp.delta_velocity = (st.parent.global_vel() - cmp.velocity);
            cmp.velocity = st.parent.global_vel();
            cmp.teleport(st.ppos);
            cmp.setPosition(st.cpos);
        }

        // CameraTarget
        /*
        void attach_update(World& w, ID<CameraTarget> id, CameraTarget& cmp, const AttachState& st) {
            // ???
        }
        */

        // Drawable
        void attach_update(World& w, ID<Drawable> id, Drawable& cmp, const AttachState& st) {
            auto& cfg = w.system<SceneSystem>().config(id);
            cfg.prev_pos = st.ppos;
            cfg.curr_pos = st.cpos;
        }

        // PathMover
        void attach_update(World& w, ID<PathMover> id, PathMover& cmp, const AttachState& st) {
            cmp.set_path_offset(st.cpos);
        }

        // TileLayer
        //void attach_update(World& w, ID<TileLayer> id, TileLayer& cmp, const AttachState& st) {
        //    auto& attch = w.at(cmp.get_attach_id())
        //}

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
        /*
        Vec2f attach_get_pos(World& w, ID<CameraTarget> id, CameraTarget& cmp) {
            return cmp.get_target_pos();
        }
        */

        // Drawable
        Vec2f attach_get_pos(World& w, ID<Drawable> id, Drawable& cmp) {
            return w.system<SceneSystem>().config(id).curr_pos;
        }

        // PathMover
        Vec2f attach_get_pos(World& w, ID<PathMover> id, PathMover& cmp) {
            return cmp.get_path_offset();
        }

        // TileLayer
        //Vec2f attach_get_pos(World& w, ID<TileLayer> id, TileLayer& cmp) {
        //    //return cmp.getOffset();
        //}
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
            //.vel = attachpoint.vel(),
            .deltaTime = deltaTime,
            .tick = w.tick_count()
        };

        if constexpr (requires(ID<T> x_id, T& x, World& x_w, const AttachState& st) { detail::attach_update(x_w, x_id, x, st); }) {
            detail::attach_update(w, attachment_id, component, state);
        }

        if constexpr (requires(ID<T> x_id, T& x, World& x_w) { detail::attach_get_pos(x_w, x_id, x); }) {
            return detail::attach_get_pos(w, attachment_id, component);
        }
        else {
            return {};
        }
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
            std::set<ID<AttachPoint>> visited;
            for (auto [aid, ap]: world.all<AttachPoint>()) {

                if (is_attachpoint_root(aid)
                    && ap.get_tick() != world.tick_count()
                    && ap.sched == sched)
                {
                    ap.set_tick(world.tick_count());
                    update_attachments(world, aid, visited);
                    visited.clear();
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
        auto ent = world.entity_of(id);
        auto attach = world.create<AttachPoint>(ent, id_placeholder);
        auto& col = world.at(id);
        col.set_attach_id(attach);
        attach->teleport(col.getPrevPosition());
        attach->set_pos(col.getPosition());
        attach->set_parent_vel(col.get_global_vel());
        attach->set_local_vel({});
        attach->sched = AttachPoint::Schedule::PostCollision;
    }

    void AttachSystem::notify_erased(World& world, ID<Collidable> id) {
        auto& col = world.at(id);
        world.erase(col.get_attach_id());
    }

    void AttachSystem::notify_created(World& world, ID<PathMover> id) {
        auto ent = world.entity_of(id);
        auto attach = world.create<AttachPoint>(ent, id_placeholder);
        auto& pathmover = world.at(id);
        pathmover.set_attach_id(attach);
        attach->teleport(pathmover.get_pos());
        attach->set_pos(pathmover.get_pos());
        attach->set_parent_vel(pathmover.get_vel());
        attach->set_local_vel({});
    }

    void AttachSystem::notify_erased(World& world, ID<PathMover> id) {
        auto& col = world.at(id);
        world.erase(col.get_attach_id());
    }

    void AttachSystem::create(World& world, ID<AttachPoint> id, ComponentID cmp_id, Vec2f offset)
    {
        attachments.at(id).insert(Attachment{ cmp_id, offset });
        cmp_lookup.emplace(cmp_id, id);
        std::visit(
        [&]<class T>(ID<T> cid) {
            if constexpr (requires(ID<T> x_id, T& x, World& x_w, const AttachPoint& ap, Vec2f x_off) { detail::attach_teleport(x_w, x_id, x, ap, x_off); }) {
                detail::attach_teleport(world, cid, world.at(cid), world.at(id), offset);
            }
        }, cmp_id);
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