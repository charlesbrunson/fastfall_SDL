#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/attach/AttachConstraint.hpp"

namespace ff {

class World;

class AttachPoint {
public:

    enum class Schedule {
        PostUpdate,
        PostCollision
    };

    AttachPoint(ID<AttachPoint> t_id, Vec2f init_pos = {}, Vec2f init_vel = {}, Schedule sch = Schedule::PostUpdate);

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

    void set_parent_vel(Vec2f v) { _pvel = v; }
    void set_local_vel(Vec2f v) { _lvel = v; }

    Vec2f local_vel() const { return _lvel; }
    Vec2f parent_vel() const { return _pvel; }
    Vec2f global_vel() const { return _lvel + _pvel; }

    Vec2f prev_local_vel() const { return _prev_lvel; }
    Vec2f prev_parent_vel() const { return _prev_pvel; }
    Vec2f prev_global_vel() const { return _prev_pvel + _prev_lvel; }

    void update_prev();

    ID<AttachPoint> id() const;

    void set_tick(size_t t) { _tick = t; }
    size_t get_tick() const { return _tick; }

    AttachConstraint constraint;

    Schedule sched = Schedule::PostUpdate;


private:
    Vec2f _curr_pos = {};
    Vec2f _prev_pos = {};

    Vec2f _lvel = {};
    Vec2f _pvel = {};

    Vec2f _prev_lvel = {};
    Vec2f _prev_pvel = {};

    ID<AttachPoint> _id;
    size_t _tick = 0;

};

void imgui_component(World& w, ID<AttachPoint> id);

}