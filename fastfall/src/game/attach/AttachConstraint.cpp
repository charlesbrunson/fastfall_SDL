#include "fastfall/game/attach/AttachPoint.hpp"

namespace ff {

    AttachConstraint makeSpringConstraint(Vec2f spring, Vec2f damping, float max_radius) {
        return [spr = spring, damp = damping, rad = max_radius]
            (AttachPoint& self, const AttachPoint& attached, Vec2f offset, secs delta)
            {
                Vec2f accel;
                auto diff = (self.curr_pos() - (attached.curr_pos() + offset));
                auto du = diff.unit();

                if (diff.magnitude() > rad) {

                    diff = du * rad;
                    self.set_pos(attached.curr_pos() + offset + diff);
                    if (math::dot(self.local_vel(), du) > 0) {
                        self.set_local_vel(math::projection(self.local_vel(), du.lefthand(), true));
                    }
                }
                self.set_parent_vel(attached.global_vel());

                accel += diff.unit() * (-spr * diff.magnitude()); // spring
                accel += self.local_vel().unit() * (-damp * self.local_vel().magnitude()); // damping
                self.set_local_vel(self.local_vel() + accel * delta);
                self.apply_vel(delta);
            };
    }
}