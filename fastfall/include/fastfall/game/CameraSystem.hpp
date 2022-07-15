#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
//#include "fastfall/game/GameContext.hpp"
#include "fastfall/game/camera/CameraTarget.hpp"

#include "fastfall/util/slot_map.hpp"
#include "fastfall/game/ComponentList.hpp"
#include "fastfall/game/WorldState.hpp"
#include "fastfall/game/WorldStateListener.hpp"

#include <array>
#include <optional>

namespace ff {

class CameraSystem : public WorldStateListener 
{
public:
	CameraSystem(WorldState& st);
	CameraSystem(WorldState& st, Vec2f initPos);

	CameraSystem(const CameraSystem&);
	CameraSystem& operator=(const CameraSystem&);

	void update(WorldState& st, secs deltaTime);

	Vec2f getPosition(float interpolation);

	float zoomFactor = 1.f;
	bool lockPosition = false;

	Vec2f deltaPosition;
	Vec2f prevPosition;
	Vec2f currentPosition;

	bool has_active_target() const { return m_active_target.has_value(); }
	ID<CameraTarget> active_target() { return *m_active_target; };

	void notify_component_created(WorldState& st, ID<Entity> entity, GenericID gid) override;
	void notify_component_destroyed(WorldState& st, ID<Entity> entity, GenericID gid) override;

private:
	void set_component_callbacks();
	void add_to_ordered(WorldState& st, ID<CameraTarget> id);

	std::optional<ID<CameraTarget>> m_active_target;
	std::vector<ID<CameraTarget>> m_ordered_targets;
};




}