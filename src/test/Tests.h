//
// Created by tobias on 21.11.16.
//

#ifndef VDSPROJECT_TESTS_H
#define VDSPROJECT_TESTS_H

#include <gtest/gtest.h>
#include "../Manager.h"

namespace ClassProject {

 class ManagerTest : public ::testing::Test  {
    protected:
        Manager manager;
    
    BDD_ID false_id = manager.False();
    BDD_ID true_id = manager.True();

    BDD_ID a_id = manager.createVar("a");
    BDD_ID b_id = manager.createVar("b");
    BDD_ID c_id = manager.createVar("c");
    BDD_ID d_id = manager.createVar("d");

    BDD_ID neg_a_id = manager.neg(a_id);
    BDD_ID neg_b_id = manager.neg(b_id);

    BDD_ID a_and_b_id = manager.and2(a_id, b_id);
    BDD_ID a_or_b_id = manager.or2(a_id, b_id);
    BDD_ID c_and_d_id = manager.and2(c_id, d_id);
    BDD_ID f_id = manager.and2(a_or_b_id, c_and_d_id);
};

TEST_F(ManagerTest, createVar) {
    EXPECT_EQ(manager.createVar("a"), a_id);
    EXPECT_EQ(manager.createVar("b"), b_id);
    EXPECT_EQ(manager.createVar("c"), c_id);
    EXPECT_EQ(manager.createVar("d"), d_id);
}

TEST_F(ManagerTest, True) {
    EXPECT_EQ(manager.True(), true_id);
}

TEST_F(ManagerTest, False) {
    EXPECT_EQ(manager.False(), false_id);
}

TEST_F(ManagerTest, isConstant) {
    EXPECT_TRUE(manager.isConstant(true_id));
    EXPECT_TRUE(manager.isConstant(false_id));
    EXPECT_FALSE(manager.isConstant(a_id));
}

TEST_F(ManagerTest, isVariable) {  
    EXPECT_FALSE(manager.isVariable(true_id));
    EXPECT_FALSE(manager.isVariable(false_id));
    EXPECT_TRUE(manager.isVariable(a_id));
    EXPECT_FALSE(manager.isVariable(a_and_b_id));
}

TEST_F(ManagerTest, uniqueTableSize){
    EXPECT_EQ(manager.uniqueTableSize(), 13);
}

TEST_F(ManagerTest, topVar) {
    EXPECT_EQ(manager.topVar(a_id), a_id);
    EXPECT_EQ(manager.topVar(b_id), b_id);
    EXPECT_EQ(manager.topVar(a_and_b_id), a_id);
}

TEST_F(ManagerTest, neg) { 
    EXPECT_EQ(manager.neg(a_id), neg_a_id);
}

TEST_F(ManagerTest, ite) {
    BDD_ID ite_aORb = manager.ite(a_id, 1, b_id);
    EXPECT_EQ(ite_aORb, a_or_b_id);
    BDD_ID ite_bANDa = manager.ite(b_id, a_id, 0);
    EXPECT_EQ(ite_bANDa, a_and_b_id);

    EXPECT_EQ(manager.ite(b_id, a_id, false_id), a_and_b_id);
    EXPECT_EQ(manager.ite(b_id, a_id, true_id), manager.or2(a_and_b_id, manager.neg(b_id)));
}

TEST_F(ManagerTest, coFactorTrue) {
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

TEST_F(ManagerTest, coFactorFalse) {
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

TEST_F (ManagerTest, findNodes) {
    std::set<BDD_ID> nodes_of_root, predefined_nodes_of_root;
    predefined_nodes_of_root = {0, 1, 5, 10, 11, 12};
    manager.findNodes(f_id, nodes_of_root);

    EXPECT_EQ(predefined_nodes_of_root, nodes_of_root);
    std::cout << "second "<< std::endl;
    std::set<BDD_ID> true_nodes, a_and_b_nodes;
    manager.findNodes(a_and_b_id, a_and_b_nodes);
    std::cout << "third "<< std::endl;
    manager.findNodes(true_id, true_nodes);

    EXPECT_EQ(true_nodes.size(), 1);
    EXPECT_TRUE(a_and_b_nodes.find(true_id) != true_nodes.end());
}

TEST_F (ManagerTest, findVars) {

    std::set<BDD_ID> vars_of_root, predefined_vars_of_root;
    predefined_vars_of_root = {2,3 , 4, 5};
    manager.findVars(f_id, vars_of_root);

    EXPECT_EQ(predefined_vars_of_root, vars_of_root);

    std::set<ClassProject::BDD_ID> true_nodes, a_and_b_nodes;
    manager.findVars(a_and_b_id, a_and_b_nodes);

     EXPECT_EQ(a_and_b_nodes.size(), 2);
     EXPECT_TRUE(a_and_b_nodes.find(a_id) != a_and_b_nodes.end());
     EXPECT_TRUE(a_and_b_nodes.find(b_id) != a_and_b_nodes.end());
}

TEST_F (ManagerTest, visualizeBDD) {
    std::string filepath = "bdd.dot";
    manager.visualizeBDD(filepath, f_id);
}

TEST_F(ManagerTest, getTopVarName) {   
    EXPECT_EQ(manager.getTopVarName(a_and_b_id), "a");
}


}

#endif
