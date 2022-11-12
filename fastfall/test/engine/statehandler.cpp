#include "gtest/gtest.h"

#include "fastfall/engine/state/EngineStateHandler.hpp"

using namespace ff;

class BlankState : public EngineState {
public:
    void update(secs deltaTime) override {};

    void predraw(float interp, bool updated, const WindowState* win_state) override {};

    inline void setEngineAction(const EngineStateAction& act) noexcept { eAct = act; };

private:
    void draw(RenderTarget& target, RenderState state = RenderState()) const override {}

};


// test helper function
void setStateEngineAction(EngineState* ptr, EngineStateAction action) {
    if (BlankState* t = dynamic_cast<BlankState*>(ptr)) {
        t->setEngineAction(action);
    }
    else {
        FAIL() << "Could not cast EngineState* to TestState*";
    }
}

TEST(statehandler, ctor_default) {
    EngineStateHandler handleEmpty;
    EXPECT_EQ(0u, handleEmpty.size());
    EXPECT_TRUE(handleEmpty.empty());
}

TEST(statehandler, ctor_init) {
    EngineStateHandler handleSingle(std::make_unique<BlankState>());
    EXPECT_EQ(1u, handleSingle.size());
    EXPECT_TRUE(!handleSingle.empty());

    EXPECT_TRUE(handleSingle.getActiveState());
}

TEST(statehandler, goto_next) {
    EngineStateHandler handleEmpty;
    EngineStateHandler handleSingle(std::make_unique<BlankState>());

    // add state internally
    EngineState* base_st = handleSingle.getActiveState();

    base_st->createState<BlankState>();
    setStateEngineAction(base_st, EngineStateAction::CONTINUE);

    handleSingle.update();

    EXPECT_EQ(1u, handleSingle.size());
    EXPECT_EQ(base_st, handleSingle.getActiveState());

    setStateEngineAction(base_st, EngineStateAction::GOTO_NEXT);
    handleSingle.update();

    EXPECT_EQ(2u, handleSingle.size());
    EXPECT_NE(base_st, handleSingle.getActiveState());

    EXPECT_EQ(base_st->getPrevState(), nullptr);
    EXPECT_EQ(base_st->getNextState(), handleSingle.getActiveState());

    EXPECT_EQ(handleSingle.getActiveState()->getPrevState(), base_st);
    EXPECT_EQ(handleSingle.getActiveState()->getNextState(), nullptr);

    // add state externally
    EngineState* top1;
    EngineState* top2;
    EngineState* top3;

    EXPECT_EQ(0u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    top1 = handleEmpty.getActiveState();
    EXPECT_EQ(1u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    top2 = handleEmpty.getActiveState();
    EXPECT_EQ(2u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    top3 = handleEmpty.getActiveState();
    EXPECT_EQ(3u, handleEmpty.size());

    EXPECT_EQ(top1->getPrevState(), nullptr);
    EXPECT_EQ(top1->getNextState(), top2);

    EXPECT_EQ(top2->getPrevState(), top1);
    EXPECT_EQ(top2->getNextState(), top3);

    EXPECT_EQ(top3->getPrevState(), top2);
    EXPECT_EQ(top3->getNextState(), nullptr);
}

TEST(statehandler, swap_next) {
    EngineStateHandler handleSingle(std::make_unique<BlankState>());

    EXPECT_EQ(1u, handleSingle.size());

    EngineState* test_st = handleSingle.getActiveState();
    test_st->createState<BlankState>();

    EngineState* next_st = test_st->getNextState();

    setStateEngineAction(test_st, EngineStateAction::SWAP_NEXT);

    handleSingle.update();
    EXPECT_EQ(1u, handleSingle.size());
    EXPECT_EQ(next_st, handleSingle.getActiveState());

    // check double swap
    setStateEngineAction(next_st, EngineStateAction::SWAP_NEXT);

    next_st->createState<BlankState>();
    setStateEngineAction(next_st->getNextState(), EngineStateAction::SWAP_NEXT);
    test_st = next_st->getNextState();

    handleSingle.update();
    EXPECT_EQ(1u, handleSingle.size());
    EXPECT_EQ(test_st, handleSingle.getActiveState());

}
TEST(statehandler, exit_prev) {
    EngineStateHandler handleEmpty;

    EngineState* top1;
    EngineState* top2;
    EngineState* top3;

    EXPECT_EQ(0u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    top1 = handleEmpty.getActiveState();
    EXPECT_EQ(1u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    top2 = handleEmpty.getActiveState();
    EXPECT_EQ(2u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    handleEmpty.update();
    top3 = handleEmpty.getActiveState();
    EXPECT_EQ(3u, handleEmpty.size());

    top3->createState<BlankState>();
    setStateEngineAction(top3, EngineStateAction::EXIT_PREV);
    handleEmpty.update();
    EXPECT_EQ(2u, handleEmpty.size());
    EXPECT_EQ(top2, handleEmpty.getActiveState());

    setStateEngineAction(top2, EngineStateAction::EXIT_PREV);
    handleEmpty.update();
    EXPECT_EQ(1u, handleEmpty.size());
    EXPECT_EQ(top1, handleEmpty.getActiveState());

    setStateEngineAction(top1, EngineStateAction::EXIT_PREV);
    handleEmpty.update();
    EXPECT_EQ(0u, handleEmpty.size());
    EXPECT_EQ(nullptr, handleEmpty.getActiveState());
}
TEST(statehandler, close_all) {
    EngineStateHandler handleEmpty;

    EXPECT_EQ(0u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    EXPECT_EQ(1u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    EXPECT_EQ(2u, handleEmpty.size());

    handleEmpty.createState<BlankState>();
    EXPECT_EQ(3u, handleEmpty.size());
    handleEmpty.getActiveState()->createState<BlankState>();

    setStateEngineAction(handleEmpty.getActiveState(), EngineStateAction::CLOSE_ALL);

    handleEmpty.update();

    // should not be any leaks
    EXPECT_EQ(0u, handleEmpty.size());
}
