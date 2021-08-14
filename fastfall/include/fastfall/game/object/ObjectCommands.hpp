#pragma once

#include "fastfall/util/commandable.hpp"

enum class ObjCmd {
	NoOp,
};

COMMANDABLE_DEF_CMD(ObjCmd, NoOp, void, void);