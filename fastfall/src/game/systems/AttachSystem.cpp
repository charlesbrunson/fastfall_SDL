#include <utility>

#include "fastfall/game/systems/AttachSystem.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

    struct AttachState {
        const AttachPoint& parent;
        Vec2f ppos;
        Vec2f cpos;
        Vec2f offset;
        secs deltaTime;
    };

    namespace detail {

        // UPDATES -----------------------------
        // Trigger
        Vec2f attach_update(World& w, ID<Trigger> id, Trigger& cmp, const AttachState& st) {
            auto area = cmp.get_area();
            area.setPosition(st.cpos);
            cmp.set_area(area);
            return cmp.get_area().getPosition();
        }

        // Collidable
        Vec2f attach_update(World& w, ID<Collidable> id, Collidable& cmp, const AttachState& st) {
            cmp.teleport(st.ppos);
            cmp.setPosition(st.cpos);
            cmp.set_parent_vel(st.parent.global_vel());
            cmp.set_local_vel(Vec2f{});
            return cmp.getPosition();
        }

        // Emitter
        Vec2f attach_update(World& w, ID<Emitter> id, Emitter& cmp, const AttachState& st) {
            cmp.prev_velocity = st.parent.prev_global_vel();
            cmp.velocity = st.parent.global_vel();
            cmp.prev_position = st.ppos;
            cmp.position = st.cpos;
            return cmp.position;
        }

        // AttachPoint
        Vec2f attach_update(World& w, ID<AttachPoint> id, AttachPoint& cmp, const AttachState& st) {
            if (cmp.constraint) {
                cmp.constraint(cmp, st.parent, st.offset, st.deltaTime);
            } else {
                cmp.teleport(st.ppos);
                cmp.set_pos(st.cpos);
                cmp.set_parent_vel(st.parent.global_vel());
                cmp.set_local_vel({});
            }
            return cmp.curr_pos();
        }

        // ColliderRegion
        Vec2f attach_update(World& w, ID<ColliderRegion> id, ColliderRegion& cmp, const AttachState& st) {
            cmp.delta_velocity = (st.parent.global_vel() - cmp.velocity);
            cmp.velocity = st.parent.global_vel();
            cmp.teleport(st.ppos);
            cmp.setPosition(st.cpos);
            return cmp.getPosition();
        }

        // CameraTarget
        /*
        void attach_update(World& w, ID<CameraTarget> id, CameraTarget& cmp, const AttachState& st) {
            // ???
        }
        */

        // Drawable
        Vec2f attach_update(World& w, ID<Drawable> id, Drawable& cmp, const AttachState& st) {
            auto& cfg = w.system<SceneSystem>().config(id);
            cfg.prev_pos = st.ppos;
            cfg.curr_pos = st.cpos;
            return cfg.curr_pos;
        }

        // PathMover
        Vec2f attach_update(World& w, ID<PathMover> id, PathMover& cmp, const AttachState& st) {
            cmp.set_path_offset(st.cpos);
            return cmp.get_path_offset();
        }

        // TileLayer
        //Vec2f attach_update(World& w, ID<TileLayer> id, TileLayer& cmp, const AttachState& st) {
        //    auto& attch = w.at(cmp.get_attach_id())
        //}

    }

    template<class T>
    Vec2f update_attached_component(World& w, const AttachPoint& attachpoint, ID<T> component_id, Vec2f offset, secs deltaTime)
    {
        auto& component = w.at(component_id);
        AttachState state {
            .parent = attachpoint,
            .ppos = attachpoint.prev_pos() + offset,
            .cpos = attachpoint.curr_pos() + offset,
            .offset = offset,
            .deltaTime = deltaTime,
            // .tick = w.tick_count()
        };

        if constexpr (requires(ID<T> x_id, T& x, World& x_w, const AttachState& st) { detail::attach_update(x_w, x_id, x, st); }) {
            return detail::attach_update(w, component_id, component, state);
        }
        else
        {
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

    void AttachSystem::update_attachments(World& world, ID<AttachPoint> id, secs deltaTime)
    {
        auto& ap = world.at(id);
        auto& attachments_map = attachments.at(id);
        for (auto& [child_id, data] : attachments_map)
        {

            Vec2f p = std::visit(
                [&](auto attach_id)
                {
                    return update_attached_component(world, ap, attach_id, data.offset, deltaTime);
                }, child_id);

            if (std::holds_alternative<ID<AttachPoint>>(child_id)) {
                update_attachments(world, std::get<ID<AttachPoint>>(child_id), deltaTime);
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

    bool AttachSystem::is_attachpoint_root(ID<AttachPoint> id) const {
        return !cmp_lookup.contains(id);
    }

    void AttachSystem::notify_created(World& world, ID<AttachPoint> id){
        attachments.emplace(id, std::map<ComponentID, AttachConfig>{});
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

    void AttachSystem::attach_component(World& world, ID<AttachPoint> parent_id, ComponentID child_id, AttachConfig cfg)
    {
        attachments.at(parent_id).emplace(child_id, cfg);
        cmp_lookup.emplace(child_id, parent_id);
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

    const std::map<ComponentID, AttachConfig>&
    AttachSystem::get_attachments(ID<AttachPoint> id) const
    {
        return attachments.at(id);
    }

    AttachConfig* AttachSystem::get_attach_config(ComponentID cmp) {
        if (auto attach_id_opt = get_attachpoint(cmp)) {
            auto& attach = attachments.at(*attach_id_opt);
            if (auto it = attach.find(cmp); it != attach.end())
            {
                return &it->second;
            }
        }
        return nullptr;
    }
}