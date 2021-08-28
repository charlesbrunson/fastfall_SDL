#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/commandable.hpp"

namespace ff {

enum class ObjCmd {
	NoOp,
	GetPosition,
};

COMMANDABLE_DEF_CMD(ObjCmd, NoOp, void, void);
COMMANDABLE_DEF_CMD(ObjCmd, GetPosition, void, Vec2f);

}