#pragma once

#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/ComponentID.hpp"

#include <map>
#include <set>

namespace ff {

    class World;

    class AttachSystem {
    public:
        /*
        struct ConstraintOut {
            Vec2f attachmentPos;
            Vec2f attachmentVel;
        };

        struct ConstraintIn {
            Vec2f attachmentPos;
            Vec2f attachmentVel;
            Vec2f attachpointPos;
            secs deltaTime;
        };

        using ConstraintFn = std::function<ConstraintOut(const ConstraintIn&)>;
        */

    private:
        struct AttachmentData {
            Vec2f offset;
        };

    public:
        using attach_map = std::map<ID<AttachPoint>, std::map<ComponentID, AttachmentData>>;


        void update(World& world, secs deltaTime);

        void update_attachpoints(World& world, secs deltaTime, AttachPoint::Schedule sched);

        void notify_created(World& world, ID<AttachPoint> id);
        void notify_erased(World& world, ID<AttachPoint> id);

        void notify_created(World& world, ID<Collidable> id);
        void notify_erased(World& world, ID<Collidable> id);

        void notify_created(World& world, ID<PathMover> id);
        void notify_erased(World& world, ID<PathMover> id);

        void create(World& world, ID<AttachPoint> id, ComponentID cmp_id, Vec2f offset = {});
        void erase(ComponentID cmp_id);

        bool is_attached(ComponentID cmp_id) const;
        std::optional<ID<AttachPoint>> get_attachpoint(ComponentID cmp_id) const;

        bool has_attachments(ID<AttachPoint> id) const;
        const std::map<ComponentID, AttachmentData>& get_attachments(ID<AttachPoint> id) const;
        void set_attach_offset(ID<AttachPoint> id, ComponentID cmp, Vec2f offset);

        bool is_attachpoint_root(ID<AttachPoint> id) const;

    private:
        void update_attachments(World& world, ID<AttachPoint> id, std::set<ID<AttachPoint>>& visited);

        attach_map attachments;
        std::map<ComponentID, ID<AttachPoint>> cmp_lookup;
        secs curr_delta;
    };

    //AttachSystem::ConstraintFn makeSpringConstraint(Vec2f springF, Vec2f dampingF);
}