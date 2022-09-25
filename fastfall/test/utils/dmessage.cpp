#include "gtest/gtest.h"

#include "fastfall/util/dmessage.hpp"

using namespace ff;

constexpr auto dGetPosition = dformat<"getpos", dtype::Vec2i>{};
constexpr auto dSetPosition = dformat<"setpos", dtype::Nil, dtype::Vec2i>{};

struct Mailbox {
    std::optional<dparam> message(dmessage msg) {
        switch(msg)
        {
            case dGetPosition:
                return dGetPosition.wrap_r(pos);
            case dSetPosition:
                std::tie(pos) = dSetPosition.unwrap(msg);
                break;
        }
        return {};
    }

    Vec2i pos;
};

struct Badbox {
    std::optional<dparam> message(dmessage msg) {
        switch(msg)
        {
            case dGetPosition:
                return Vec2f{};
            case dSetPosition:
                return Vec2i{};
        }
        return {};
    }
};

TEST(dmessage, wrap_unwrap)
{
    auto getpos_msg = dGetPosition.wrap();

    auto test_vec1 = Vec2i{12, 15};
    auto test_vec2 = Vec2i{10, 100};

    auto setpos_msg = dSetPosition.wrap(test_vec1);

    bool found = false;
    switch (setpos_msg) {
        case dSetPosition: found = true; break;
    }
    ASSERT_TRUE(found);

    auto [vec] = dSetPosition.unwrap(setpos_msg);
    ASSERT_TRUE(vec == test_vec1);

    auto r = dGetPosition.wrap_r(test_vec2);

    auto r2 = dGetPosition.unwrap_r(r);
    ASSERT_TRUE(r2 == test_vec2);

}

TEST(dmessage, mailbox) {
    Mailbox box;

    Vec2i r1 = {100, 20};
    Vec2i v1 = dGetPosition.send(box);
    ASSERT_TRUE(v1 == Vec2i{});

    dSetPosition.send(box, r1);

    Vec2i v2 = dGetPosition.send(box);
    ASSERT_TRUE(v2 == r1);
}

TEST(dmessage, badbox) {
    Badbox box;

    Vec2i r1 = {100, 20};
    ASSERT_ANY_THROW(dGetPosition.send(box));
    ASSERT_ANY_THROW(dSetPosition.send(box, r1));
}



