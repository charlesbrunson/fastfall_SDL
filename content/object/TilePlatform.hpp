#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/level/TileLayer.hpp"
class TilePlatform : public ff::Object {
public:
    static const ff::ObjectType Type;
    TilePlatform(ff::ActorInit init, ff::ObjectLevelData& data);
    void ImGui_Inspect() override { ImGui::Text("test1"); };
};



