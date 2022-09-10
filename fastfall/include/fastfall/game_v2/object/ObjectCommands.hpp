#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/commandable.hpp"

namespace ff {

enum class ObjCmd {
	NoOp,
	GetPosition,
	Hurt
};

COMMANDABLE_DEF_CMD(ObjCmd, NoOp,			void,	void);
COMMANDABLE_DEF_CMD(ObjCmd, GetPosition,	void,	Vec2f);
COMMANDABLE_DEF_CMD(ObjCmd, Hurt,			float,	void);

}