#include "gtest/gtest.h"

#include "fastfall/util/math.hpp"
#include "fastfall/util/dmessage.hpp"

using namespace ff;

// declare valid parameter types ( up to 16 for now )
using dvar = std::variant<
    std::monostate, // <- std::monostate is required at index zero
    bool,
    int,
    float,
    Vec2i,
    Vec2f
>;

struct ExampleAddtlArg {

};

using my_config = dconfig<dvar, ExampleAddtlArg>;

// define some message formats: message name, return type, message parameters... (up to 4 for now)
constexpr auto dGetPosition  = my_config::dformat<"getpos",    Vec2i>{};
constexpr auto dSetPosition  = my_config::dformat<"setpos",    std::monostate, Vec2i>{};
constexpr auto dHasHPBetween = my_config::dformat<"hpbetween", bool, float, float>{};
constexpr auto dUnhandled    = my_config::dformat<"unhandled">{};

// this mailbox responds correctly to messages
struct Mailbox {
    my_config::dresult message(ExampleAddtlArg hw, const my_config::dmessage& msg) {
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
        return my_config::reject;
    }

    bool hasHPBetween(float min, float max) const {
        return min <= health && health <= max;
    }

    Vec2i pos;
    float health = 50.f;
};

// this mailbox responds incorrectly to messages
struct Badbox {
    my_config::dresult message(ExampleAddtlArg hw, const my_config::dmessage& msg) {
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
    Vec2i v1 = *dGetPosition.send(box, ExampleAddtlArg{});
    ASSERT_TRUE(v1 == Vec2i{});

    dSetPosition.send(box, ExampleAddtlArg{}, r1);

    Vec2i v2 = *dGetPosition.send(box, ExampleAddtlArg{});
    ASSERT_TRUE(v2 == r1);

    auto h = dUnhandled.send(box, ExampleAddtlArg{});
    ASSERT_FALSE(h);

    bool hp = *dHasHPBetween.send(box, ExampleAddtlArg{}, 20.f, 80.f);
    ASSERT_TRUE(hp);
}

TEST(dmessage, badbox) {
    Badbox box;

    Vec2i r1 = {100, 20};
    ASSERT_ANY_THROW(dGetPosition.send(box, ExampleAddtlArg{}));
    ASSERT_ANY_THROW(dSetPosition.send(box, ExampleAddtlArg{}, r1));

    auto h = dUnhandled.send(box, ExampleAddtlArg{});
    ASSERT_FALSE(h);
}



