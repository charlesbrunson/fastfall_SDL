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
        using attach_map = std::map<ID<AttachPoint>, std::map<ComponentID, AttachConfig>>;

        void update(World& world, secs deltaTime);

        void notify_created(World& world, ID<AttachPoint> id);
        void notify_erased(World& world, ID<AttachPoint> id);

        void notify_created(World& world, ID<Collidable> id);
        void notify_erased(World& world, ID<Collidable> id);

        void notify_created(World& world, ID<PathMover> id);
        void notify_erased(World& world, ID<PathMover> id);

        void attach_component(World& world, ID<AttachPoint> parent_id, ComponentID child_id, AttachConfig cfg = {});
        void detach_component(ComponentID cmp_id);

        bool is_attached(ComponentID cmp_id) const;
        std::optional<ID<AttachPoint>> get_attachpoint(ComponentID cmp_id) const;

        bool has_attachments(ID<AttachPoint> id) const;
        const std::map<ComponentID, AttachConfig>& get_attachments(ID<AttachPoint> id) const;

        AttachConfig* get_attach_config(ComponentID cmp);

        bool is_attachpoint_root(ID<AttachPoint> id) const;

        void update_attachments(World& world, ID<AttachPoint> id, secs deltaTime = 0.0);
    private:

        attach_map attachments;
        std::map<ComponentID, ID<AttachPoint>> cmp_lookup;
    };

}