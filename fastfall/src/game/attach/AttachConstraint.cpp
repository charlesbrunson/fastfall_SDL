#include "fastfall/game/attach/AttachPoint.hpp"

namespace ff {

    AttachConstraint makeSpringConstraint(Vec2f spring, Vec2f damping, float max_radius) {
        return [spr = spring, damp = damping, rad = max_radius]
            (AttachPoint& self, const AttachPoint& attached, Vec2f offset, secs delta)
            {
                Vec2f accel;
                auto diff = (self.curr_pos() - (attached.curr_pos() + offset));
                auto du = math::unit(diff);

                if (math::mag(diff) > rad) {

                    diff = du * rad;
                    self.set_pos(attached.curr_pos() + offset + diff);
                    if (math::dot(self.local_vel(), du) > 0) {
                        self.set_local_vel(math::proj(self.local_vel(), math::lefthand_normal(du)));
                    }
                }
                self.set_parent_vel(attached.global_vel());

                accel += diff * -spr;
                accel += self.local_vel() * -damp;
                self.set_local_vel(self.local_vel() + accel * static_cast<float>(delta));
                self.apply_vel(delta);
            };
    }
}