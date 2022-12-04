#include "fastfall/game/attach/AttachPoint.hpp"

namespace ff {

AttachPoint::AttachPoint(ID<AttachPoint> t_id, Vec2f init_pos, Vec2f init_vel, Schedule sch)
    : _id(t_id)
    , sched(sch)
{
    teleport(init_pos);
    set_parent_vel(init_vel);
    set_local_vel({});
}

void AttachPoint::apply_vel(secs deltaTime) {
    _curr_pos += global_vel() * deltaTime;
}

void AttachPoint::set_pos(Vec2f next_pos) {
    _curr_pos = next_pos;
}

void AttachPoint::teleport(Vec2f next_pos) {
    _curr_pos = next_pos;
    _prev_pos = next_pos;
}

void AttachPoint::move(Vec2f offset) {
    _curr_pos += offset;
}

void AttachPoint::shift(Vec2f offset) {
    _curr_pos += offset;
    _prev_pos += offset;
}

Vec2f AttachPoint::interpolate(float interp) const {
    return _prev_pos + (_curr_pos - _prev_pos) * interp;
}

Vec2f AttachPoint::curr_pos() const {
    return _curr_pos;
}

Vec2f AttachPoint::prev_pos() const {
    return _prev_pos;
}

void AttachPoint::update_prev() {
    _prev_pos = _curr_pos;
    _prev_lvel = _lvel;
    _prev_pvel = _pvel;
}

ID<AttachPoint> AttachPoint::id() const {
    return _id;
}

}