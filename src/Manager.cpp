#include "Manager.h"

namespace ClassProject {
    /**
     * Creates a new variable with the given label and returns its ID.
     *
     * @param  label: The name of the variable.
     * @return The BDD node ID of the created variable.
     */    
    BDD_ID Manager::createVar(const std::string &label)
    {
        for (const auto &node : BDD_uniqueTable) {
            if (node.label == label) {
                return node.id;
            }
        }

        BDDNode newNode;
        newNode.id = static_cast<BDD_ID>(Manager::uniqueTableSize());
        newNode.label = label;
        newNode.high = this->True();
        newNode.low = this->False();
        newNode.topVar = static_cast<BDD_ID>(Manager::uniqueTableSize());
        BDD_uniqueTable.push_back(newNode);

        Triplet key = {newNode.high, newNode.low, newNode.topVar};
        optimizedTable[key] = newNode.id;

        return newNode.id;
    }

    /**
     * Returns the ID of the True node.
     *
     * @param  None.
     * @return Reference to the BDD ID representing logical true.
     */    
    const BDD_ID &Manager::True()
    {
        return trueVar;
    }
    
    /**
     * Returns the ID of the False node.
     *
     * @param  None.
     * @return Reference to the BDD ID representing logical false.
     */    
    const BDD_ID &Manager::False()
    {
        return falseVar;
    }
    
    /**
     * Returns true, if the given ID represents a leaf node.
     *
     * @param  f: The ID of BDD needs to be checked.
     * @return True if f is a constant.
     */    
    bool Manager::isConstant(BDD_ID f)
    {
        return (f == trueVar || f == falseVar);
    }
    
    /**
     * Returns true, if the given ID represents a variable.
     *
     * @param  x: The BDD variable ID needs to be checked.
     * @return True if x is a valid variable.
     */    
    bool Manager::isVariable(BDD_ID x)
    {
        return !(isConstant(x)) && (topVar(x) == x);
    }
    
    /**
     * Returns the top variable ID of the given node.
     *
     * @param  f: The ID of BDD needs to be checked.
     * @return The BDD ID of the top variable.
     */    
    BDD_ID Manager::topVar(BDD_ID f)
    {
        return BDD_uniqueTable[f].topVar;
    }
    
    /**
     * Implements the if-then-else (ITE) algorithm.
     *
     * Takes three BDD_IDs and returns the result of the ITE operation.
     * - Handles terminal cases first.
     * - Determines the top variable among the three inputs.
     * - Recursively computes low and high successors using CoFactorFalse and CoFactorTrue.
     * - If both successors are equal, returns that value.
     * - Otherwise, checks if the result exists in the BDD table.
     * - If not, creates a new node in the table and returns its ID.
     *
     * @param  i: Condition BDD ID.
     * @param  t: Then-case BDD ID.
     * @param  e: Else-case BDD ID.
     * @return Existing or new BDD ID that represents the given expression.
     */
    BDD_ID Manager::ite(BDD_ID i, BDD_ID t, BDD_ID e)
    {
        // terminal case
        if (i == 1) {
            return t;
        }
        
        if (i == 0) {
            return e;
        }
    
        if (t == e) {
            return t;
        }
    
        if (t == 1 && e == 0) {
            return i;
        }

        Triplet tri{i,t,e};
        auto checking = computedTable.find(tri);
        if (checking != computedTable.end()){
            return checking->second;
        }

        BDD_ID x = Manager::False();
        if (!isConstant(i)) {
            x = topVar(i);
        }
        if (!isConstant(t)) {
            x = std::min(topVar(t), x);
        }
        if (!isConstant(e)) {
            x = std::min(topVar(e), x);
        }

        BDD_ID ct_f = Manager::coFactorTrue(i, x), ct_g = Manager::coFactorTrue(t, x), ct_h = Manager::coFactorTrue(e, x);
        BDD_ID T = ite(ct_f, ct_g, ct_h);
        computedTable[{ct_f, ct_g, ct_h}] = T;


        BDD_ID cf_f = Manager::coFactorFalse(i, x), cf_g = Manager::coFactorFalse(t, x), cf_h = Manager::coFactorFalse(e, x);
        BDD_ID E = ite(cf_f, cf_g, cf_h);

        if (T == E) {
            return T;
        }
        Triplet key = {T,E,x};
        auto it = optimizedTable.find(key);
        if (it != optimizedTable.end()) {
            return it->second;
        }

        BDDNode R;
        R.id = static_cast<BDD_ID>(Manager::uniqueTableSize());
        R.label = "id" + std::to_string(R.id);
        R.high = T;
        R.low = E;
        R.topVar = x;
        BDD_uniqueTable.emplace_back(R);
        computedTable[tri] = R.id;
        optimizedTable[key] = R.id;

        return R.id;
    }

    /**
     * Returns the positive co-factor of a BDD function.
     * Example: f = a+(b*c) with alphabetical variable order
     * coFactorTrue(f) = 1 = coFactorTrue(f,a)
     * coFactorTrue(f,c) = a+b
     *
     * @param  f: The BDD node ID of the function.
     * @param  x: The variable with respect to which the co-factor is taken (optional).
     * @return The BDD node ID of the positive co-factor.
     */    
    BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x)
    {
        // top variable of f comes after x in the variable order
        if (isConstant(f) || isConstant(x) || topVar(f) > x) {
            // x does not appear in f
            return f;
        } else {
            // x is the top variable of f
            if (topVar(f) == x) { 
                return BDD_uniqueTable[f].high;
            }
            // x appears below the top variable of f
            return ite(topVar(f), coFactorTrue(BDD_uniqueTable[f].high, x), coFactorTrue(BDD_uniqueTable[f].low, x));
        }
    }

    /**
     * Returns the negative co-factor of a BDD function.
     * Example: f = a+(b*c) with alphabetical variable order
     * coFactorFalse(f) = b*c = coFactorFalse(f,a)
     * coFactorFalse(f,c) = a
     *
     * @param  f: The BDD node ID of the function.
     * @param  x: The variable with respect to which the co-factor is taken (optional).
     * @return The BDD node ID of the negative co-factor.
     */    
    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x)
    {
        // top variable of f comes after x in the variable order
        if (isConstant(f) || isConstant(x) || topVar(f) > x) {
            // x does not appear in f
            return f; 
        } else {
            // x is the top variable of f
            if (topVar(f) == x) { 
            return BDD_uniqueTable[f].low;
            }
            // x appears below the top variable of f
            return ite(topVar(f), coFactorFalse(BDD_uniqueTable[f].high, x), coFactorFalse(BDD_uniqueTable[f].low, x));
        }
    }

    /**
     * Returns the positive co-factor of a BDD function.
     *
     * @param  f: The BDD node ID of the function.
     * @return The BDD node ID of the positive co-factor.
     */    
    BDD_ID Manager::coFactorTrue(BDD_ID f)
    {
        return BDD_uniqueTable[f].high;
    }

    /**
     * Returns the negative co-factor of a BDD function.
     *
     * @param  f: The BDD node ID of the function.
     * @return The BDD node ID of the negative co-factor.
     */    
    BDD_ID Manager::coFactorFalse(BDD_ID f)
    {
        return BDD_uniqueTable[f].low;
    }

    /**
     * Returns the number of nodes currently existing in the unique table of the Manager class.
     *
     * @param  None.
     * @return BDD unique table size.
     */    
    size_t Manager::uniqueTableSize()
    {
      return Manager::BDD_uniqueTable.size();
    }

    /**
     * Returns the ID representing the negation of the given function.
     *
     * @param  a: The BDD node ID to negate.
     * @return The BDD node ID representing the logical NOT.
     */    
    BDD_ID Manager::neg(BDD_ID a)
    {
        BDD_ID aNot = ite(a, Manager::False(), Manager::True());
        BDD_uniqueTable[aNot].label = "~" + BDD_uniqueTable[a].label;

        return aNot;
    }

    /**
     * Returns the ID representing the resulting function of a*b.
     *
     * @param  a: The first BDD node ID.
     * @param  b: The second BDD node ID.
     * @return The BDD node ID representing (a AND b).
     */    
    BDD_ID Manager::and2(BDD_ID a, BDD_ID b)
    {
        BDD_ID abAND = ite(a, b, Manager::False());
        BDD_uniqueTable[abAND].label = "(" + BDD_uniqueTable[a].label + " * " + BDD_uniqueTable[b].label + ")";

        return abAND;
    }

    /**
     * Returns the ID representing the resulting function of a+b.
     *
     * @param  a: The first BDD node ID.
     * @param  b: The second BDD node ID.
     * @return The BDD node ID representing (a OR b).
     */    
    BDD_ID Manager::or2(BDD_ID a, BDD_ID b)
    {
        BDD_ID abOR = ite(a, Manager::True(), b);
        BDD_uniqueTable[abOR].label = "(" + BDD_uniqueTable[a].label + " + " + BDD_uniqueTable[b].label + ")";

        //strncpy(BDD_uniqueTable[abOR].label, ("(" + std::string(BDD_uniqueTable[a].label) + " + " + std::string(BDD_uniqueTable[b].label) + ")").c_str(), sizeof(BDD_uniqueTable[abOR].label));
        
        return abOR;
    }

    /**
     * Returns the ID representing the resulting function of a^b.
     *
     * @param  a: The first BDD node ID.
     * @param  b: The second BDD node ID.
     * @return The BDD node ID representing (a XOR b).
     */    
    BDD_ID Manager::xor2(BDD_ID a, BDD_ID b)
    {
        BDD_ID abXOR = ite(a, Manager::neg(b), b);
        BDD_uniqueTable[abXOR].label = "(" + BDD_uniqueTable[a].label + " ^ " + BDD_uniqueTable[b].label + ")";

        return abXOR;
    }

    /**
     * Returns the ID representing the resulting function of NOT(a*b).
     *
     * @param  a: The first BDD node ID.
     * @param  b: The second BDD node ID.
     * @return The BDD node ID representing (a NAND b).
     */    
    BDD_ID Manager::nand2(BDD_ID a, BDD_ID b)
    {
        BDD_ID abNAND = Manager::neg(and2(a, b));
        BDD_uniqueTable[abNAND].label = "(" + BDD_uniqueTable[a].label + " * " + BDD_uniqueTable[b].label + ")";

        return abNAND;
    }

    /**
     * Returns the ID representing the resulting function of NOT(a+b).
     *
     * @param  a: The first BDD node ID.
     * @param  b: The second BDD node ID.
     * @return The BDD node ID representing (a NOR b).
     */    
    BDD_ID Manager::nor2(BDD_ID a, BDD_ID b)
    {
        BDD_ID abNOR = Manager::neg(or2(a, b));
        BDD_uniqueTable[abNOR].label = "(" + BDD_uniqueTable[a].label + " + " + BDD_uniqueTable[b].label + ")";

        return abNOR;
    }

    /**
     * Returns the ID representing the resulting function of NOT(a^b).
     *
     * @param  a: The first BDD node ID.
     * @param  b: The second BDD node ID.
     * @return The BDD node ID representing (a XNOR b).
     */    
    BDD_ID Manager::xnor2(BDD_ID a, BDD_ID b)
    {
        BDD_ID abXNOR = Manager::neg(xor2(a, b));
        BDD_uniqueTable[abXNOR].label = "(" + BDD_uniqueTable[a].label + " ^ " + BDD_uniqueTable[b].label + ")";

        return abXNOR;
    }

    /**
     * Returns the label of the top variable of root.
     *
     * @param  root: The BDD node ID whose top variable's name is to be retrieved.
     * @return The label of the top variable.
     */    
    std::string Manager::getTopVarName(const BDD_ID &root)
    {
        return BDD_uniqueTable[topVar(root)].label;
    }

    /**
     * Takes a node root and an empty set nodes_of_root and returns th set of all nodes which are reachable from root including itself.
     *
     * @param  root: The root node ID of the BDD from which traversal begins.
     * @param  nodes_of_root: A reference to a set that will be filled with all nodes reachable from root.
     * @return None.
     */    
    void Manager::findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root)
    {
        if (nodes_of_root.find(root) != nodes_of_root.end()) {
            return;
        }

        nodes_of_root.insert(BDD_uniqueTable[root].id);
        findNodes(BDD_uniqueTable[root].high, nodes_of_root);
        findNodes(BDD_uniqueTable[root].low, nodes_of_root);
    }

    /**
     * Takes a node root and an empty set vars_of_root and returns the set of all variables which are reachable from root including itself if root is a variable.
     *
     * @param  root: The root node ID of the BDD from which traversal begins.
     * @param  nodes_of_root: A reference to a set that will be filled with all reachable variable nodes.
     * @return None.
     */    
    void Manager::findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root)
    {
        std::set<BDD_ID> nodes_of_root;
        findNodes(root, nodes_of_root);
        for (const auto &node : nodes_of_root) {
            if (node != 0 && node != 1) {
                vars_of_root.insert(topVar(node));
            }
        }
    }

    /**
     * Writes the Binary Decision Diagram (BDD) to a DOT file for visualization.
     * - Each internal BDD node is represented by a circle with its top variable as a label.
     * - Terminal nodes (0 and 1) are represented by boxes.
     * - Edges to low (false) children are drawn as dotted lines.
     * - Edges to high (true) children are drawn as solid lines.
     *
     * @param filepath The output path for the DOT file.
     * @param root     The root BDD node ID to visualize.
     * @return None.
     */    
    void Manager::visualizeBDD(std::string filepath, BDD_ID &root)
    {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filepath << std::endl;
            return;
        }

        file << "digraph BDD {" << std::endl;
        file << "    node [shape=circle];" << std::endl;

        std::set<BDD_ID> nodes;
        BDD_ID high,low;
        findNodes(root, nodes);

        for (const auto &node : nodes) {
            if (node != 0 && node != 1) {
                file << "    " << node << " [label=\"" << BDD_uniqueTable[BDD_uniqueTable[node].topVar].label << "\"];" << std::endl;
                file << "    " << node << " -> " << high << " [style=solid];" << std::endl;
                file << "    " << node << " -> " << low << " [style=dotted];" << std::endl;
            }
        }

        file << "    0 [shape=box, label=\"0\"];" << std::endl;
        file << "    1 [shape=box, label=\"1\"];" << std::endl;

        file << "}" << std::endl;
        file.close();
    }
}
