#include "fastfall/engine/state/EngineState.hpp"
//#include "game/Instance.hpp"

namespace ff {

int EngineState::stateIGIDCounter = 0;

EngineState::EngineState() :
	//instanceID(InstanceID::NO_INSTANCE_ID),
	stateIGID(stateIGIDCounter++)
{

};

const std::string& EngineState::getImGuiHeader() {
	return ImGuiHeader;
};

}