#include "SimpleCamTarget.hpp"

using namespace ff;

SimpleCamTarget::SimpleCamTarget(GameContext context, CamTargetPriority priority, const Vec2f* pos, Vec2f offset, bool add_to_cam)
	: CameraTarget(context, priority, add_to_cam)
	, position_ptr(pos)
	, m_offset(offset)
{
}

SimpleCamTarget::SimpleCamTarget(GameContext context, CamTargetPriority priority, const Vec2f* pos, bool add_to_cam)
	: CameraTarget(context, priority, add_to_cam)
	, position_ptr(pos)
{
}

void SimpleCamTarget::update(secs delta) 
{
}

Vec2f SimpleCamTarget::get_target_pos() const {
	return *position_ptr + m_offset;
}