#ifndef VDSPROJECT_REACHABILITY_TESTS_H
#define VDSPROJECT_REACHABILITY_TESTS_H

#include <gtest/gtest.h>
#include "Reachability.h"

using namespace ClassProject;

struct ReachabilityTest : testing::Test {

    std::unique_ptr<ClassProject::ReachabilityInterface> fsm2 = std::make_unique<ClassProject::Reachability>(2);

    std::vector<BDD_ID> stateVars2 = fsm2->getStates();
    std::vector<BDD_ID> transitionFunctions;


    std::unique_ptr<ClassProject::ReachabilityInterface> fsm_test = std::make_unique<ClassProject::Reachability>(2, 2);

    std::vector<BDD_ID> stateVars_test = fsm_test->getStates();
    std::vector<BDD_ID> inputVars_test = fsm_test->getInputs();
    std::vector<BDD_ID> transitionFunctions_test;
};

TEST_F(ReachabilityTest, HowTo_Example) { /* NOLINT */

    BDD_ID s0 = stateVars2.at(0);
    BDD_ID s1 = stateVars2.at(1);

    transitionFunctions.push_back(fsm2->neg(s0)); // s0' = not(s0)
    transitionFunctions.push_back(fsm2->neg(s1)); // s1' = not(s1)
    fsm2->setTransitionFunctions(transitionFunctions);

    fsm2->setInitState({false,false});

    ASSERT_TRUE(fsm2->isReachable({false, false}));
    ASSERT_FALSE(fsm2->isReachable({false, true}));
    ASSERT_FALSE(fsm2->isReachable({true, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true}));
}

TEST_F(ReachabilityTest, test){

    BDD_ID s1 = stateVars_test.at(0);
    BDD_ID s2 = stateVars_test.at(1);

    BDD_ID x1 = inputVars_test.at(0);
    BDD_ID x2 = inputVars_test.at(1);
    
    // s0' = not(x1) AND (s1 OR s2)
    transitionFunctions_test.push_back(fsm_test->and2(fsm_test->neg(x1), fsm_test->or2(s1, s2)));
    // s1' = x2 AND ((x1 OR s1) OR s2)
    transitionFunctions_test.push_back(fsm_test->and2(x2, fsm_test->or2(fsm_test->or2(x1, s1), s2)));

    fsm_test->setTransitionFunctions(transitionFunctions_test);

    ASSERT_EQ(fsm_test->stateDistance({true, true}),  2);
    ASSERT_EQ(fsm_test->stateDistance({false, false}), 0);

    ASSERT_TRUE(fsm_test->isReachable({true, true}));
    ASSERT_TRUE(fsm_test->isReachable({true, false}));
    
    ASSERT_TRUE(fsm_test->isReachable({false, true}));
    ASSERT_TRUE(fsm_test->isReachable({false, false}));

}

TEST_F(ReachabilityTest, ToggleBit_Test) {
    
    BDD_ID s0 = stateVars_test.at(0);
    BDD_ID s1 = stateVars_test.at(1);

    BDD_ID x1 = inputVars_test.at(0);

    // s0' = s0 XOR x1
    transitionFunctions_test.push_back(fsm_test->xor2(s0, x1));
    // s1' = s1
    transitionFunctions_test.push_back(s1);

    fsm_test->setTransitionFunctions(transitionFunctions_test);

    fsm_test->setInitState({false, false}); // initial state: {00}

    // reachable states are {00, 10}
    ASSERT_TRUE(fsm_test->isReachable({false, false}));
    ASSERT_TRUE(fsm_test->isReachable({true, false}));

    ASSERT_FALSE(fsm_test->isReachable({false, true}));
    ASSERT_FALSE(fsm_test->isReachable({true, true}));

    ASSERT_EQ(fsm_test->stateDistance({false, false}), 0);
    ASSERT_EQ(fsm_test->stateDistance({true, false}), 1);
    ASSERT_EQ(fsm_test->stateDistance({true, true}), -1);
}

TEST_F(ReachabilityTest, ConditionalPropagation_Test) {
    
    BDD_ID s0 = stateVars_test.at(0);
    BDD_ID s1 = stateVars_test.at(1);

    BDD_ID x1 = inputVars_test.at(0);
    BDD_ID x2 = inputVars_test.at(1);

    // s0' = s0 OR x1
    transitionFunctions_test.push_back(fsm_test->or2(s0, x1));
    // s1' = s1 OR (s0 AND x2)
    transitionFunctions_test.push_back(fsm_test->or2(s1, fsm_test->and2(s0, x2)));

    fsm_test->setTransitionFunctions(transitionFunctions_test);

    fsm_test->setInitState({true, false}); // initial state: {10}

    // reachable states are {10, 11}
    ASSERT_TRUE(fsm_test->isReachable({true, false}));
    ASSERT_TRUE(fsm_test->isReachable({true, true}));

    ASSERT_FALSE(fsm_test->isReachable({false, false}));
    ASSERT_FALSE(fsm_test->isReachable({false, true}));

    ASSERT_EQ(fsm_test->stateDistance({true, false}), 0);
    ASSERT_EQ(fsm_test->stateDistance({true, true}), 1);
    ASSERT_EQ(fsm_test->stateDistance({false, false}), -1);
    ASSERT_EQ(fsm_test->stateDistance({false, true}), -1);
}


#endif
