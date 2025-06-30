#include "Reachability.h"

namespace ClassProject{
    Reachability::Reachability(unsigned int stateSize, unsigned int inputSize)
        : ReachabilityInterface(stateSize, inputSize)
    {
        if (!stateSize) {
            throw std::runtime_error("stateSize can not be zero!");
        }
    
        for (unsigned int i = 0; i < stateSize; ++i) {
            // 1. Create variables for the current and next state s0, s1, s0' and s1'
            stateVars.push_back(createVar("s" + std::to_string(i)));
            nextStateVars.push_back(createVar("s" + std::to_string(i) + "'")); 
            // 2. Compute the BDD for d:  d0 = _s0, d1 = _s1
            transitionFunctions.push_back(stateVars[i]);
            initState.push_back(False());
        }
    
        for (unsigned int i = 0; i < inputSize; ++i) {
            inputVars.push_back(createVar("i" + std::to_string(i)));
        }
    }

    const std::vector<BDD_ID> &Reachability::getStates() const
    {
        return stateVars;
    }

    const std::vector<BDD_ID> &Reachability::getInputs() const
    {
        return inputVars;  
    }

    bool Reachability::isReachable(const std::vector<bool> &stateVector)
    {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("Size does not match with number of state bits");
        }

        if (calc) {
            BDD_ID reachable = C_R;
            for (unsigned int i = 0; i < stateVector.size(); ++i) {
                if (stateVector[i] == true) {
                    reachable = coFactorTrue(reachable, stateVars[i]);
                } else {
                    reachable = coFactorFalse(reachable, stateVars[i]);
                }
           }
          return reachable != False();
        }
        else {
            return stateDistance(stateVector) != -1;
        }  
    }

    int Reachability::stateDistance(const std::vector<bool> &stateVector)
    {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("Size does not match with number of state bits");
        }
    
        if (stateVector == initState) {
            return 0;
        }

        // 3. Compute the BDD for the transition relation T = (s0'd0 + _s0'_d0) * (s1'd1 + _s1'_d1)
        BDD_ID T = True();
        for (unsigned int i = 0; i < stateVars.size(); ++i) {
            T = and2(T, xnor2(nextStateVars[i], transitionFunctions[i]));
        }

        // 4. Compute the BDD for the characteristic function of the initial state (0,0):
        //  c_s = (s0 == 0) i* (s1 == 0) = _(s0 XOR 0) * _(s1 XOR 0) 
        BDD_ID c_s = True();
        for (unsigned int i = 0; i < stateVars.size(); i++) {
            c_s = and2(c_s, xnor2(stateVars[i], initState[i]));
        }

        // 5. c_Rit = c_s
        BDD_ID c_Rit = c_s;

        BDD_ID c_R, nextImg, currentImg;
        int distance = 0;
        BDD_ID reachable = False();

        do {
            // 6. c_R = c_Rit
            c_R = c_Rit;
    
            // 7. Compute the BDD for img(s0', s1') = E_s0E_s1(c_R*T) by using the Manager functions:
            // temp1 = c_R * T
            // temp2 = coFactorTrue(temp1, s1) + coFactorFalse(temp1, s1)
            // img(s0', s1') = coFactorTrue(temp2, s0) + coFactorFalse(temp2, s0)
            nextImg = and2(c_R, T);
            for (int i = stateVars.size()-1; i >= 0; --i) {
                nextImg = or2(coFactorTrue(nextImg, stateVars[i]), coFactorFalse(nextImg, stateVars[i]));
            }

            // 8. Compute img(s0, s1) = E_s0'E_s1'(s0 == s0') * (s1 == s1') * img(s0', s1')
            // temp1 = _(s0 XOR s0') * _(s1 XOR s1') * img(s0', s1')
            // temp2 = coFactorTrue(temp1, s1') + coFactorFalse(temp1, s1')
            // img(s0, s1) = coFactorTrue(temp2, s0') + coFactorFalse(temp2, s0')
            currentImg = nextImg;
            for (int i = 0; i < stateVars.size(); i++) {
                currentImg = and2(currentImg, xnor2(stateVars[i], nextStateVars[i]));
            }
            for (int i = nextStateVars.size()-1; i >= 0; i--) {
                currentImg = or2(coFactorTrue(currentImg, nextStateVars[i]), coFactorFalse(currentImg, nextStateVars[i]));
            }
            for (int i = 0; i < inputVars.size(); i++) {
                currentImg = or2(coFactorTrue(currentImg, inputVars[i]), coFactorFalse(currentImg, inputVars[i]));
            }


            // 9. Compute the BDD for the new c_Rit = c_R + img(s0, s1)
            c_Rit = or2(c_R, currentImg);
            if (reachable != True()) {
                distance++;
            }

            // Check if the stateVector is reachable at this distance
            reachable = c_Rit;
            for (unsigned int i = 0; i < stateVector.size(); ++i) {
                if (stateVector[i] == true) {
                    reachable = coFactorTrue(reachable, stateVars[i]);
                } else {
                    reachable = coFactorFalse(reachable, stateVars[i]);
                }
            }
            if (reachable == True() && calc) {
                return distance;
            }

        // 10. Check if c_Rit == c_R in the first iteration, c_R consists of (0,0) and (1,1), whereas c_Rit consists of (1,1).
        // Therefore, it is not a fixed point and we have to go back to step 6 and perform a second iteration. 
        } while (c_R != c_Rit);
    
        calc = true;
        C_R = c_R;
        if (reachable == True()){
            return distance;
        }

        // If the state is not reachable
        return -1;
    }

    void Reachability::setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions)
    {
        if (transitionFunctions.size() != stateVars.size()) {
            throw std::runtime_error("Size does not match with number of state bits");
        }
    
        for (unsigned int i = 0; i < transitionFunctions.size(); ++i) {
            if (transitionFunctions[i] >= uniqueTableSize()) {
                throw std::runtime_error("An unknown ID is provided");
            }
        }
    
        this->transitionFunctions = transitionFunctions;
        calc = false;
    }

    void Reachability::setInitState(const std::vector<bool> &stateVector)
    {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("Size does not match with number of state bits");
        }
        
        for (unsigned int i = 0; i < stateVector.size(); ++i) {
            if (stateVector[i] == true) {
                initState[i] = True();
            } else {
                initState[i] = False();
            }
        }
    
        calc = false;
    }
}
