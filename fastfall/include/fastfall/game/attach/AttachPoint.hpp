#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/attach/AttachConstraint.hpp"

namespace ff {

class AttachPoint {
public:

    AttachPoint(ID<AttachPoint> t_id);

    // apply vel to position
    void apply_vel(secs deltaTime);

    // set position to next_pos
    void set_pos(Vec2f next_pos);

    // set position and prev_pos to next_pos
    void teleport(Vec2f next_pos);

    // add offset to position
    void move(Vec2f offset);

    // add offset to position and prev_pos
    void shift(Vec2f offset);

    Vec2f interpolate(float interp) const;

    Vec2f curr_pos() const;
    Vec2f prev_pos() const;

    void set_vel(Vec2f v);
    void add_vel(Vec2f v);
    Vec2f vel() const;

    void update_prev();

    ID<AttachPoint> id() const;

    void set_tick(size_t t) { _tick = t; }
    size_t get_tick() const { return _tick; }

    AttachConstraint constraint;

    enum class Schedule {
        PostUpdate,
        PostCollision
    };
    Schedule sched = Schedule::PostUpdate;


private:
    Vec2f _curr_pos;
    Vec2f _prev_pos;
    Vec2f _vel;
    ID<AttachPoint> _id;
    size_t _tick = 0;

};




}