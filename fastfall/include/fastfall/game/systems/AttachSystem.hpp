#pragma once

#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/ComponentID.hpp"

#include <map>
#include <set>

namespace ff {

    class World;

    class AttachSystem {
        struct AttachmentData {
            Vec2f offset;
        };

    public:
        using attach_map = std::map<ID<AttachPoint>, std::map<ComponentID, AttachmentData>>;

        void update(World& world, secs deltaTime);

        // void update_attachpoints(World& world, secs deltaTime, AttachPoint::Schedule sched);

        void notify_created(World& world, ID<AttachPoint> id);
        void notify_erased(World& world, ID<AttachPoint> id);

        void notify_created(World& world, ID<Collidable> id);
        void notify_erased(World& world, ID<Collidable> id);

        void notify_created(World& world, ID<PathMover> id);
        void notify_erased(World& world, ID<PathMover> id);

        void attach_component(World& world, ID<AttachPoint> parent_id, ComponentID child_id, Vec2f offset = {});
        void detach_component(ComponentID cmp_id);

        bool is_attached(ComponentID cmp_id) const;
        std::optional<ID<AttachPoint>> get_attachpoint(ComponentID cmp_id) const;

        bool has_attachments(ID<AttachPoint> id) const;
        const std::map<ComponentID, AttachmentData>& get_attachments(ID<AttachPoint> id) const;
        void set_attach_offset(ID<AttachPoint> id, ComponentID cmp, Vec2f offset);

        bool is_attachpoint_root(ID<AttachPoint> id) const;

        void notify_moved(World& world, ID<AttachPoint> id, secs deltaTime)
        {
            update_attachments(world, id, deltaTime, false);
        }
        void notify_teleport(World& world, ID<AttachPoint> id)
        {
            update_attachments(world, id, 0.0, true);
        }

    private:
        void update_attachments(World& world, ID<AttachPoint> id, secs deltaTime, bool teleport);

        attach_map attachments;
        std::map<ComponentID, ID<AttachPoint>> cmp_lookup;
    };

}