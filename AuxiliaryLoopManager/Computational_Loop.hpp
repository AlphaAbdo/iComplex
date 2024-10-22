#pragma once
#include <externVars.hpp>

namespace Computational_Loop{
    namespace HostCheckPoints{
        void verifyComputationCompletion();
        void forceEffectCompletion();
        void kickstartParallelProcessing();
        void triggerAbortionProtocol();
    }
}