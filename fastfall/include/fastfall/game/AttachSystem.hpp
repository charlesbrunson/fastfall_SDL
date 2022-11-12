#pragma once

#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/engine/time/time.hpp"
#include "AnyID.hpp"

#include <map>
#include <set>

namespace ff {

    class World;

    class AttachSystem {
    private:
        struct Attachment {
            ComponentID id;
            Vec2f offset;

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

        void notify_created(World& world, ID<AttachPoint> id);
        void notify_erased(World& world, ID<AttachPoint> id);

        void notify(World& world, ID<AttachPoint> id);

        void create(ID<AttachPoint> id, ComponentID cmp_id, Vec2f offset);
        void erase(ComponentID cmp_id);

        bool is_attached(ComponentID cmp_id) const;
        std::optional<ID<AttachPoint>> get_attachpoint(ComponentID cmp_id) const;

        bool has_attachments(ID<AttachPoint> id) const;
        const std::set<Attachment>& get_attachments(ID<AttachPoint> id) const;

    private:
        std::map<ID<AttachPoint>, std::set<Attachment>> attachments;
        std::map<ComponentID, ID<AttachPoint>> cmp_lookup;
    };

}