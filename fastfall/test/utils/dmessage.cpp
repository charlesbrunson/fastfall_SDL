#include "gtest/gtest.h"

#include "fastfall/util/dmessage.hpp"

using namespace ff;

// define some message formats: message name, return type, message parameters... (up to 4 for now)
constexpr auto dGetPosition  = dformat<"getpos", dtype::Vec2i>{};
constexpr auto dSetPosition  = dformat<"setpos", dtype::Nil, dtype::Vec2i>{};
constexpr auto dHasHPBetween = dformat<"ishpbetween", dtype::Bool, dtype::Float, dtype::Float>{};
constexpr auto dUnhandled    = dformat<"unhandled">{};

// this mailbox responds correctly to messages
struct Mailbox {
    dresult message(dmessage msg) {
        switch(msg)
        {
        case dGetPosition:
            return dGetPosition.accept(pos);
        case dSetPosition:
            std::tie(pos) = dSetPosition.unwrap(msg);
            return dSetPosition.accept();
        case dHasHPBetween:
            //auto [min, max] = dHasHPBetween.unwrap(msg);
            //return dHasHPBetween.accept(min <= health && health <= max);
            // OR
            //return dHasHPBetween.apply(msg, &Mailbox::hasHPBetween, this);
            // OR
            return dHasHPBetween.apply(msg, [this](float min, float max) { return min <= health && health <= max; });
        }
        return {};
    }

    bool hasHPBetween(float min, float max) const {
        return min <= health && health <= max;
    }

    Vec2i pos;
    float health = 50.f;
};

// this mailbox responds incorrectly to messages
struct Badbox {
    dresult message(dmessage msg) {
        switch(msg)
        {
        case dGetPosition:
            return {true, {} };
        case dSetPosition:
            return {true, Vec2i{} };
        }
        return {};
    }
};

TEST(dmessage, wrap_unwrap)
{
    auto getpos_msg = dGetPosition.wrap();

    auto test_vec1 = Vec2i{12, 15};

    auto setpos_msg = dSetPosition.wrap(test_vec1);

    bool found = false;
    switch (setpos_msg) {
        case dSetPosition: found = true; break;
    }
    ASSERT_TRUE(found);

    auto [vec] = dSetPosition.unwrap(setpos_msg);
    ASSERT_TRUE(vec == test_vec1);
}

TEST(dmessage, mailbox) {
    Mailbox box;

    Vec2i r1 = {100, 20};
    Vec2i v1 = *dGetPosition.send(box);
    ASSERT_TRUE(v1 == Vec2i{});

    dSetPosition.send(box, r1);

    Vec2i v2 = *dGetPosition.send(box);
    ASSERT_TRUE(v2 == r1);

    auto h = dUnhandled.send(box);
    ASSERT_FALSE(h);

    bool hp = *dHasHPBetween.send(box, 20.f, 80.f);
    ASSERT_TRUE(hp);
}

TEST(dmessage, badbox) {
    Badbox box;

    Vec2i r1 = {100, 20};
    ASSERT_ANY_THROW(dGetPosition.send(box));
    ASSERT_ANY_THROW(dSetPosition.send(box, r1));

    auto h = dUnhandled.send(box);
    ASSERT_FALSE(h);
}



