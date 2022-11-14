#include "fastfall/game/attach/AttachPoint.hpp"

namespace ff {

    AttachConstraint makeSpringConstraint(Vec2f spring, Vec2f damping) {
        return [spr = spring, damp = damping]
            (AttachPoint& self, const AttachPoint& attached, Vec2f offset, secs delta)
            {
                Vec2f accel;
                auto diff = (self.curr_pos() - (attached.curr_pos() + offset));
                accel += diff.unit() * (-spr * diff.magnitude()); // spring
                accel += self.vel().unit() * (-damp * self.vel().magnitude()); // damping
                self.add_vel(accel * delta);
                self.update(delta);
            };
    }
}