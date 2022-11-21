#pragma once

#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/engine/time/time.hpp"
#include "AnyID.hpp"

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
        struct Attachment {
            ComponentID id;
            Vec2f offset;
            //ConstraintFn constraint;

            bool operator==(const Attachment& other) const {
                return id == other.id;
            }

            bool operator!=(const Attachment& other) const {
                return id != other.id;
            }

            bool operator<(const Attachment& other) const {
                return id < other.id;
            }
        };

    public:
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
        const std::set<Attachment>& get_attachments(ID<AttachPoint> id) const;

        bool is_attachpoint_root(ID<AttachPoint> id) const;

    private:
        void update_attachments(World& world, ID<AttachPoint> id, std::set<ID<AttachPoint>>& visited);

        std::map<ID<AttachPoint>, std::set<Attachment>> attachments;
        std::map<ComponentID, ID<AttachPoint>> cmp_lookup;
        secs curr_delta;
    };

    //AttachSystem::ConstraintFn makeSpringConstraint(Vec2f springF, Vec2f dampingF);
}