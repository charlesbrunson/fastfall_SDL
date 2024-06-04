#include "SimpleCamTarget.hpp"

using namespace ff;

SimpleCamTarget::SimpleCamTarget(CamTargetPriority priority, std::function<ff::Vec2f(World&)>&& callback)
	: CameraTarget(priority)
	, pos_callback(callback)
{
}

void SimpleCamTarget::update(World& w, secs delta)
{
    position = pos_callback(w);
}
