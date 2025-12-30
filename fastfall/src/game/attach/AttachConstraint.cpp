#include "fastfall/game/attach/AttachPoint.hpp"

namespace ff {

    AttachConstraint makeSpringConstraint(Vec2f spring, Vec2f damping, float max_radius) {
        return [spr = spring, damp = damping, rad = max_radius]
            (AttachPoint& child, const AttachPoint& parent, Vec2f offset, secs delta)
            {
                Vec2f accel = {};
                auto diff = (child.curr_pos() - (parent.curr_pos() + offset));

                if (diff != Vec2f{} && math::mag(diff) > rad) {
                    auto dunit = math::unit(diff);

                    diff = dunit * rad;
                    child.set_pos(parent.curr_pos() + offset + diff);
                    if (math::dot(child.local_vel(), dunit) > 0) {
                        child.set_local_vel(math::proj(child.local_vel(), math::lefthand_normal(dunit)));
                    }
                }
                child.set_parent_vel(parent.global_vel());

                accel += diff * -spr;
                accel += child.local_vel() * -damp;
                child.set_local_vel(child.local_vel() + accel * static_cast<float>(delta));
                child.apply_vel(delta);
            };
    }
}