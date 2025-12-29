#include <utility>

#include "fastfall/game/systems/AttachSystem.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

    struct AttachState {
        const AttachPoint& parent;
        bool teleport;
        Vec2f ppos;
        Vec2f cpos;
        Vec2f offset;
        secs deltaTime;
        // size_t tick;
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

            for (auto& pair : w.system<AttachSystem>().get_attachments(id)) {

                auto& cmp_id = pair.first;
                auto& data   = pair.second;

                std::visit(
                    [&]<class T>(ID<T> cid) {
                        log::scope sc;
                        LOG_INFO("attach teleport visit: {} -> {}", cmpid_str(id), cmpid_str(cmp_id));
                        if constexpr (requires(ID<T> x_id, T& x, World& x_w, const AttachPoint& ap, Vec2f x_off) { detail::attach_teleport(x_w, x_id, x, ap, x_off); }) {
                        detail::attach_teleport(w, cid, w.at(cid), w.at(id), data.offset);
                    }
                }, cmp_id);
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
            cfg.rstate.transform.setPosition(cfg.curr_pos);
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
            cmp.prev_velocity = st.parent.prev_global_vel();
            cmp.velocity = st.parent.global_vel();
            cmp.prev_position = st.ppos;
            cmp.position = st.cpos;
        }

        // AttachPoint
        void attach_update(World& w, ID<AttachPoint> id, AttachPoint& cmp, const AttachState& st) {
            if (cmp.constraint && !st.teleport) {
                cmp.constraint(cmp, st.parent, st.offset, st.deltaTime);
            } else {
                cmp.set_pos(st.cpos);
                cmp.set_parent_vel(st.parent.global_vel());
                cmp.set_local_vel({});
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
    Vec2f update_attached_component(World& w, const AttachPoint& attachpoint, ID<T> component_id, Vec2f offset, secs deltaTime, bool teleport)
    {
        auto& component = w.at(component_id);
        AttachState state {
            .parent = attachpoint,
            .teleport = teleport,
            .ppos = attachpoint.prev_pos() + offset,
            .cpos = attachpoint.curr_pos() + offset,
            .offset = offset,
            .deltaTime = deltaTime,
            // .tick = w.tick_count()
        };

        if constexpr (requires(ID<T> x_id, T& x, World& x_w, const AttachState& st) { detail::attach_update(x_w, x_id, x, st); }) {
            detail::attach_update(w, component_id, component, state);
        }

        if constexpr (requires(ID<T> x_id, T& x, World& x_w) { detail::attach_get_pos(x_w, x_id, x); }) {
            return detail::attach_get_pos(w, component_id, component);
        }
        else {
            return {};
        }
    }

    void AttachSystem::update(World& world, secs deltaTime) {
        if (deltaTime > 0.0) {
            // curr_delta = deltaTime;
            for (auto [id, ap]: world.all<AttachPoint>()) {
                ap.update_prev();
            }
        }
    }

    void AttachSystem::update_attachments(World& world, ID<AttachPoint> id, secs deltaTime, bool teleport)
    {
        auto& ap = world.at(id);
        for (auto& [c_id, data] : attachments.at(id))
        {
            Vec2f p = std::visit(
                [&](auto attach_id)
                {
                    return update_attached_component(world, ap, attach_id, data.offset, deltaTime, teleport);
                }, c_id);

            if (holds_alternative<ID<AttachPoint>>(c_id)) {
                update_attachments(world, std::get<ID<AttachPoint>>(c_id), deltaTime, teleport);
            }

            if (debug::enabled(debug::Attach)) {
                auto draw = debug::draw(&data, Primitive::LINES, 6);

                for (auto & ndx : draw) {
                    ndx.color = Color::Green;
                }

                draw[0].pos = ap.curr_pos() + Vec2f{-2, -2};
                draw[1].pos = ap.curr_pos() + Vec2f{ 2,  2};
                draw[2].pos = ap.curr_pos() + Vec2f{-2,  2};
                draw[3].pos = ap.curr_pos() + Vec2f{ 2, -2};
                draw[4].pos = ap.curr_pos();
                draw[5].pos = p;
            }
        }
    }

    /*
    void AttachSystem::update_attachpoints(World& world, secs deltaTime, AttachPoint::Schedule sched) {
        if (deltaTime > 0.0) {
            std::set<ID<AttachPoint>> visited;
            for (auto [aid, ap]: world.all<AttachPoint>()) {

                if (is_attachpoint_root(aid)
                    && ap.get_tick() != world.tick_count()
                    && ap.sched == sched)
                {
                    ap.set_tick(world.tick_count());
                    // update_attachments(world, aid, visited);
                    visited.clear();
                }
            }
        }
    }
    */

    /*
    void AttachSystem::update_attachments(World& world, ID<AttachPoint> id, std::set<ID<AttachPoint>>& visited) {
        if (visited.contains(id))
            return;

        visited.insert(id);
        auto& ap = world.at(id);
        for (auto& attach : attachments.at(id))
        {
            auto& id = attach.first;
            auto& data = attach.second;

            Vec2f p;
            std::visit(
                [&]<class T>(ID<T> c_id) {
                    p = update_attachment(world, ap, c_id, data.offset, curr_delta);
                }, id);

            if (holds_alternative<ID<AttachPoint>>(id)) {
                update_attachments(world, std::get<ID<AttachPoint>>(id), visited);
            }

            if (debug::enabled(debug::Attach)) {
                auto draw = debug::draw((const void *)&data, Primitive::LINES, 6);

                for (auto & ndx : draw) {
                    ndx.color = Color::Green;
                }

                draw[0].pos = ap.curr_pos() + Vec2f{-2, -2};
                draw[1].pos = ap.curr_pos() + Vec2f{ 2,  2};
                draw[2].pos = ap.curr_pos() + Vec2f{-2,  2};
                draw[3].pos = ap.curr_pos() + Vec2f{ 2, -2};
                draw[4].pos = ap.curr_pos();
                draw[5].pos = p;
            }
        }
    }
    */

    bool AttachSystem::is_attachpoint_root(ID<AttachPoint> id) const {
        return !cmp_lookup.contains(id);
    }

    void AttachSystem::notify_created(World& world, ID<AttachPoint> id){
        attachments.emplace(id, std::map<ComponentID, AttachmentData>{});
    }

    void AttachSystem::notify_erased(World& world, ID<AttachPoint> id){
        auto attchs = std::move(attachments.at(id));
        for (auto& [id, _] : attchs) {
            detach_component(id);
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

    void AttachSystem::attach_component(World& world, ID<AttachPoint> parent_id, ComponentID child_id, Vec2f offset)
    {
        attachments.at(parent_id).emplace(child_id, AttachmentData{ offset });
        cmp_lookup.emplace(child_id, parent_id);
        // update_attachments(world, id, 0.0, true);
    }

    void AttachSystem::detach_component(ComponentID cmp_id) {
        auto iter = cmp_lookup.find(cmp_id);
        if (iter != cmp_lookup.end())
        {
            auto ap = iter->second;
            attachments.at(ap).erase(cmp_id);
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

    const std::map<ComponentID, AttachSystem::AttachmentData>&
    AttachSystem::get_attachments(ID<AttachPoint> id) const
    {
        return attachments.at(id);
    }

    void AttachSystem::set_attach_offset(ID<AttachPoint> id, ComponentID cmp, Vec2f offset) {
        if (attachments.contains(id)) {
            auto& attach = attachments.at(id);
            if (auto it = attach.find(cmp); it != attach.end())
            {
                it->second.offset = offset;
            }
        }
    }
}