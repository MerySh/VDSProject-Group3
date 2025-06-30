#ifndef VDSPROJECT_REACHABILITY_H
#define VDSPROJECT_REACHABILITY_H

#include "ReachabilityInterface.h"
#include "../Manager.h"

namespace ClassProject {
    class Reachability : public ReachabilityInterface {
        public:
            Reachability(unsigned int stateSize, unsigned int inputSize = 0);

            const std::vector<BDD_ID> &getStates() const;
            const std::vector<BDD_ID> &getInputs() const;
            bool isReachable(const std::vector<bool> &stateVector);
            int stateDistance(const std::vector<bool> &stateVector);
            void setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions);
            void setInitState(const std::vector<bool> &stateVector);

        private:
            std::vector<BDD_ID> stateVars;
            std::vector<BDD_ID> nextStateVars;
            std::vector<BDD_ID> inputVars;
            std::vector<BDD_ID> transitionFunctions;
            std::vector<bool> initState;

            bool calc = false;
            BDD_ID C_R = False();
    };
}

#endif
