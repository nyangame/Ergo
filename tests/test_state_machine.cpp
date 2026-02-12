#include "test_framework.hpp"
#include "engine/core/state_machine.hpp"

namespace {

struct StateA {
    static inline int enter_count = 0;
    static inline int exit_count = 0;
    static inline int update_count = 0;
    void enter() { ++enter_count; }
    void exit() { ++exit_count; }
    void update(float) { ++update_count; }
    static void reset() { enter_count = exit_count = update_count = 0; }
};

struct StateB {
    static inline int enter_count = 0;
    static inline int exit_count = 0;
    static inline int update_count = 0;
    void enter() { ++enter_count; }
    void exit() { ++exit_count; }
    void update(float) { ++update_count; }
    static void reset() { enter_count = exit_count = update_count = 0; }
};

} // namespace

TEST_CASE(StateMachine_InitialState) {
    StateA::reset(); StateB::reset();
    StateMachine<StateA, StateB> sm;
    ASSERT_FALSE(sm.is_state<StateA>());
    ASSERT_FALSE(sm.is_state<StateB>());
}

TEST_CASE(StateMachine_Transition) {
    StateA::reset(); StateB::reset();
    StateMachine<StateA, StateB> sm;

    sm.transition<StateA>();
    ASSERT_TRUE(sm.is_state<StateA>());
    ASSERT_FALSE(sm.is_state<StateB>());
    ASSERT_EQ(StateA::enter_count, 1);
}

TEST_CASE(StateMachine_TransitionCallsExit) {
    StateA::reset(); StateB::reset();
    StateMachine<StateA, StateB> sm;

    sm.transition<StateA>();
    sm.transition<StateB>();
    ASSERT_TRUE(sm.is_state<StateB>());
    ASSERT_EQ(StateA::exit_count, 1);
    ASSERT_EQ(StateB::enter_count, 1);
}

TEST_CASE(StateMachine_Update) {
    StateA::reset(); StateB::reset();
    StateMachine<StateA, StateB> sm;

    sm.transition<StateA>();
    sm.update(0.016f);
    sm.update(0.016f);
    ASSERT_EQ(StateA::update_count, 2);
    ASSERT_EQ(StateB::update_count, 0);
}
