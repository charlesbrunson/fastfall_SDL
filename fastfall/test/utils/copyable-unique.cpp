#include "gtest/gtest.h"

#include "fastfall/util/copyable_uniq_ptr.hpp"

#include <numeric>

using namespace ff;

struct Base {
    virtual int test() const { return 0; }
};

struct DerivedA : public Base {
    int test() const override { return 1; }
};

struct DerivedB : public Base {
    int test() const override { return 2; }
};

TEST(copyable_unique, constructor)
{
    copyable_unique_ptr<Base> base_ptr;
    EXPECT_TRUE(!base_ptr);

    base_ptr = make_copyable_unique<Base, DerivedA>();
    EXPECT_TRUE((bool)base_ptr);
    EXPECT_EQ(1, base_ptr->test());

    base_ptr = make_copyable_unique<Base, DerivedB>();
    EXPECT_TRUE((bool)base_ptr);
    EXPECT_EQ(2, base_ptr->test());

    base_ptr = make_copyable_unique<Base, Base>();
    EXPECT_TRUE((bool)base_ptr);
    EXPECT_EQ(0, base_ptr->test());

    base_ptr.reset();
    EXPECT_TRUE(!base_ptr);

    base_ptr = make_copyable_unique<Base, DerivedB>();
    EXPECT_TRUE((bool)base_ptr);
    EXPECT_EQ(2, base_ptr->test());
}

copyable_unique_ptr<Base> make_A() {
    return make_copyable_unique<Base, DerivedA>();
}

copyable_unique_ptr<Base> make_B() {
    return make_copyable_unique<Base, DerivedB>();
}

copyable_unique_ptr<Base> multi() {
    return make_B();
}


TEST(copyable_unique, assignment)
{
    auto ptr = make_A();
    EXPECT_EQ(1, ptr->test());

    ptr = make_B();
    EXPECT_EQ(2, ptr->test());

    ptr = multi();
    EXPECT_EQ(2, ptr->test());

}
