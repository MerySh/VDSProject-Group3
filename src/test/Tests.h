//
// Created by tobias on 21.11.16.
//

#ifndef VDSPROJECT_TESTS_H
#define VDSPROJECT_TESTS_H

#include <gtest/gtest.h>
#include "../Manager.h"

namespace ClassProject {

// Google Test fixture class for testing the Manager class
class ManagerTest : public ::testing::Test  {
protected:
    // Instance of the Manager class to be used across all test cases
    Manager manager;

    // IDs for constant Boolean values
    BDD_ID false_id = manager.False();
    BDD_ID true_id = manager.True();

    // IDs for created Boolean variables
    BDD_ID a_id = manager.createVar("a");
    BDD_ID b_id = manager.createVar("b");
    BDD_ID c_id = manager.createVar("c");
    BDD_ID d_id = manager.createVar("d");

    // IDs for the negation of some variables
    BDD_ID neg_a_id = manager.neg(a_id);
    BDD_ID neg_b_id = manager.neg(b_id);

    // IDs for basic Boolean operations
    BDD_ID a_and_b_id = manager.and2(a_id, b_id);
    BDD_ID a_or_b_id = manager.or2(a_id, b_id);

    BDD_ID a_nand_b_id = manager.nand2(a_id, b_id);
    BDD_ID a_nor_b_id = manager.nor2(a_id, b_id);
    BDD_ID a_xor_b_id = manager.xor2(a_id, b_id);
    BDD_ID a_xnor_b_id = manager.xnor2(a_id, b_id);

    // AND operation between other variables
    BDD_ID c_and_d_id = manager.and2(c_id, d_id);

    // Compound expression: (a ∨ b) ∧ (c ∧ d)
    BDD_ID f_id = manager.and2(a_or_b_id, c_and_d_id);
};

// Test: createVar should return consistent IDs
TEST_F(ManagerTest, createVarTest) {
    EXPECT_EQ(manager.createVar("a"), a_id);
    EXPECT_EQ(manager.createVar("b"), b_id);
    EXPECT_EQ(manager.createVar("c"), c_id);
    EXPECT_EQ(manager.createVar("d"), d_id);
}

// Test: check if True constant is correctly returned
TEST_F(ManagerTest, TrueTest) {
    EXPECT_EQ(manager.True(), true_id);
}

// Test: check if False constant is correctly returned
TEST_F(ManagerTest, FalseTest) {
    EXPECT_EQ(manager.False(), false_id);
}

// Test: check if a node is a constant
TEST_F(ManagerTest, isConstantTest) {
    EXPECT_TRUE(manager.isConstant(true_id));
    EXPECT_TRUE(manager.isConstant(false_id));
    EXPECT_FALSE(manager.isConstant(a_id)); // a is a variable, not constant
}

// Test: check if a node is a variable
TEST_F(ManagerTest, isVariableTest) {  
    EXPECT_FALSE(manager.isVariable(true_id));
    EXPECT_FALSE(manager.isVariable(false_id));
    EXPECT_TRUE(manager.isVariable(a_id));
    EXPECT_FALSE(manager.isVariable(a_and_b_id)); // result of AND is not a variable
}

// Test: verify the size of the unique table
TEST_F(ManagerTest, uniqueTableSizeTest){
    EXPECT_EQ(manager.uniqueTableSize(), 17); // 2 constants + 9 op of F + 6 ite validations
}

// Test: retrieve the top variable of a node
TEST_F(ManagerTest, topVarTest) {
    EXPECT_EQ(manager.topVar(a_id), a_id);
    EXPECT_EQ(manager.topVar(b_id), b_id);
    EXPECT_EQ(manager.topVar(a_and_b_id), a_id); // top variable of a ∧ b is a
}

// Test: negation operation
TEST_F(ManagerTest, negTest) { 
    EXPECT_EQ(manager.neg(a_id), neg_a_id);
}

// Test: if-then-else (ITE) operations and their logical equivalences
TEST_F(ManagerTest, iteTest) {
    EXPECT_EQ(manager.ite(a_id, true_id, false_id), a_id); // a ? 1 : 0 => a

    BDD_ID ite_aORb = manager.ite(a_id, 1, b_id); // a ∨ b +1
    EXPECT_EQ(ite_aORb, a_or_b_id);

    BDD_ID ite_bANDa = manager.ite(b_id, a_id, 0); // b ∧ a +2
    EXPECT_EQ(ite_bANDa, a_and_b_id);

    BDD_ID ite_bNANDa = manager.neg(ite_bANDa); // NAND(a, b) +3
    EXPECT_EQ(ite_bNANDa, a_nand_b_id);

    BDD_ID ite_aNORb = manager.neg(ite_aORb); // NOR(a, b) +4
    EXPECT_EQ(ite_aNORb, a_nor_b_id);

    BDD_ID ite_aXORb = manager.ite(a_id, manager.neg(b_id), b_id); // XOR(a, b) +5
    EXPECT_EQ(ite_aXORb, a_xor_b_id);

    BDD_ID ite_aXNORb = manager.neg(ite_aXORb); // XNOR(a, b) +6
    EXPECT_EQ(ite_aXNORb, a_xnor_b_id);

    // Additional logical expressions using ITE
    EXPECT_EQ(manager.ite(b_id, a_id, false_id), a_and_b_id);
    EXPECT_EQ(manager.ite(b_id, a_id, true_id), manager.or2(a_and_b_id, manager.neg(b_id)));
}

// Test: coFactorTrue behavior
TEST_F(ManagerTest, coFactorTrueTest) {
    EXPECT_EQ(manager.coFactorTrue(a_id, a_id), true_id);
    EXPECT_EQ(manager.coFactorTrue(a_id, true_id), a_id);
    EXPECT_EQ(manager.coFactorTrue(true_id, a_id), true_id);
    EXPECT_EQ(manager.coFactorTrue(b_id, a_id), b_id);
    EXPECT_EQ(manager.coFactorTrue(d_id, c_id), d_id);
    EXPECT_EQ(manager.coFactorTrue(a_or_b_id, a_id), true_id);
    EXPECT_EQ(manager.coFactorTrue(a_or_b_id, b_id), true_id);
    EXPECT_EQ(manager.coFactorTrue(c_and_d_id, a_id), c_and_d_id);
    EXPECT_EQ(manager.coFactorTrue(c_and_d_id, c_id), d_id);
    EXPECT_EQ(manager.coFactorTrue(a_and_b_id, b_id), a_id);
}

// Test: coFactorFalse behavior
TEST_F(ManagerTest, coFactorFalseTest) {
    EXPECT_EQ(manager.coFactorFalse(a_id, a_id), false_id);
    EXPECT_EQ(manager.coFactorFalse(a_id, false_id), a_id);
    EXPECT_EQ(manager.coFactorFalse(false_id, a_id), false_id);
    EXPECT_EQ(manager.coFactorFalse(b_id, a_id), b_id);
    EXPECT_EQ(manager.coFactorFalse(d_id, c_id), d_id);
    EXPECT_EQ(manager.coFactorFalse(a_or_b_id, a_id), b_id);
    EXPECT_EQ(manager.coFactorFalse(c_and_d_id, a_id), c_and_d_id);
    EXPECT_EQ(manager.coFactorFalse(c_and_d_id, c_id), false_id);
    EXPECT_EQ(manager.coFactorFalse(f_id, c_id), false_id);
}

// Test: find all reachable nodes from a root node
TEST_F(ManagerTest, findNodesTest) {
    std::set<BDD_ID> nodes_of_root, predefined_nodes_of_root;
    predefined_nodes_of_root = {0, 1, 5, 14, 15, 16}; // Expected set of node IDs
    manager.findNodes(f_id, nodes_of_root);

    EXPECT_EQ(predefined_nodes_of_root, nodes_of_root);

    std::set<BDD_ID> true_nodes, a_and_b_nodes;
    manager.findNodes(a_and_b_id, a_and_b_nodes);
    manager.findNodes(true_id, true_nodes);

    EXPECT_EQ(true_nodes.size(), 1);
    EXPECT_TRUE(a_and_b_nodes.find(true_id) != true_nodes.end());
}

// Test: find all variables used in a BDD
TEST_F(ManagerTest, findVarsTest) {
    std::set<BDD_ID> vars_of_root, predefined_vars_of_root;
    predefined_vars_of_root = {2, 3, 4, 5}; // Variables used in f_id
    manager.findVars(f_id, vars_of_root);

    EXPECT_EQ(predefined_vars_of_root, vars_of_root);

    std::set<BDD_ID> a_and_b_vars;
    manager.findVars(a_and_b_id, a_and_b_vars);

    EXPECT_EQ(a_and_b_vars.size(), 2);
    EXPECT_TRUE(a_and_b_vars.find(a_id) != a_and_b_vars.end());
    EXPECT_TRUE(a_and_b_vars.find(b_id) != a_and_b_vars.end());
}

// Test: visualize a BDD to a DOT file
TEST_F(ManagerTest, visualizeBDDTest) {
    std::string filepath = "bdd.dot";
    manager.visualizeBDD(filepath, f_id); // Output to DOT format for graph visualization
}

// Test: retrieve the name of the top variable of a node
TEST_F(ManagerTest, getTopVarNameTest) {
    EXPECT_EQ(manager.getTopVarName(a_and_b_id), "a");
}

} // namespace ClassProject

#endif
