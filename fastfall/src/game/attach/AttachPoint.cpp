#include "fastfall/game/attach/AttachPoint.hpp"

namespace ff {

AttachPoint::AttachPoint(ID<AttachPoint> t_id)
    : _id(t_id)
{
}

void AttachPoint::apply_vel(secs deltaTime) {
    _curr_pos += _vel * deltaTime;
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

void AttachPoint::set_vel(Vec2f v) {
    _vel = v;
}

void AttachPoint::add_vel(Vec2f v) {
    _vel += v;
}

Vec2f AttachPoint::vel() const {
    return _vel;
}

void AttachPoint::update_prev() {
    _prev_pos = _curr_pos;
}

ID<AttachPoint> AttachPoint::id() const {
    return _id;
}

}