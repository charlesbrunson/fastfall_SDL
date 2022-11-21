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
                    auto local_vel = self.vel() - attached.vel();
                    self.set_pos(attached.curr_pos() + offset + diff);
                    if (math::dot(local_vel, du) > 0) {
                        self.set_vel(math::projection(local_vel, du.lefthand(), true) + attached.vel());
                    }
                }
                accel += diff.unit() * (-spr * diff.magnitude()); // spring
                accel += self.vel().unit() * (-damp * self.vel().magnitude()); // damping
                self.add_vel(accel * delta);
                self.apply_vel(delta);
            };
    }
}