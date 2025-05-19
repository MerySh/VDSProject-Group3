// A minimalistic BDD library, following Wolfgang Kunz lecture slides
//
// Created by Markus Wedler 2014

#ifndef VDSPROJECT_MANAGER_H
#define VDSPROJECT_MANAGER_H

#include "ManagerInterface.h"
#include <unordered_map>
#include <vector>

namespace ClassProject {
    typedef struct {
      BDD_ID id;
      std::string label;
      BDD_ID high;
      BDD_ID low;
      BDD_ID topVar;
    } BDDNode;

    class Manager : public ManagerInterface {
    public:
        Manager()
        {
            BDD_uniqueTable.push_back({0, "False", 0, 0, 0});
            BDD_uniqueTable.push_back({1, "True", 1, 1, 1});
        }
        ~Manager() {}

        BDD_ID falseVar = 0;
        BDD_ID trueVar = 1;

        BDD_ID createVar(const std::string &label) override;
        const BDD_ID &True() override;
        const BDD_ID &False() override;
        bool isConstant(BDD_ID f) override;
        bool isVariable(BDD_ID x) override;
        BDD_ID topVar(BDD_ID f) override;
        BDD_ID ite(BDD_ID i, BDD_ID t, BDD_ID e) override;
        BDD_ID coFactorTrue(BDD_ID f, BDD_ID x) override;
        BDD_ID coFactorFalse(BDD_ID f, BDD_ID x) override;
        BDD_ID coFactorTrue(BDD_ID f) override;
        BDD_ID coFactorFalse(BDD_ID f) override;
        BDD_ID and2(BDD_ID a, BDD_ID b) override;
        BDD_ID or2(BDD_ID a, BDD_ID b) override;
        BDD_ID xor2(BDD_ID a, BDD_ID b) override;
        BDD_ID neg(BDD_ID a) override;
        BDD_ID nand2(BDD_ID a, BDD_ID b) override;
        BDD_ID nor2(BDD_ID a, BDD_ID b) override;
        BDD_ID xnor2(BDD_ID a, BDD_ID b) override;
        std::string getTopVarName(const BDD_ID &root) override;
        void findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) override;
        void findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) override;
        size_t uniqueTableSize() override;
        void visualizeBDD(std::string filepath, BDD_ID &root) override;

    private:
      std::vector<BDDNode> BDD_uniqueTable;
    };
}

#endif
