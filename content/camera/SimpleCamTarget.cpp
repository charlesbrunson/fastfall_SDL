#include "SimpleCamTarget.hpp"

using namespace ff;

SimpleCamTarget::SimpleCamTarget(GameContext context, CamTargetPriority priority, std::function<ff::Vec2f()>&& callback)
	: CameraTarget(context, priority)
	, pos_callback(callback)
{
}

void SimpleCamTarget::update(secs delta) 
{
}

Vec2f SimpleCamTarget::get_target_pos() const {
	return pos_callback();
}