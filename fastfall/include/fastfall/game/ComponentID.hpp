#pragma once

#include "fastfall/util/id.hpp"

#include <variant>
#include <string_view>

#include "WorldComponentConfig.hpp"

namespace ff {

using ComponentID = Components::ComponentID;

std::string cmpid_str(const ComponentID& cmp);

}