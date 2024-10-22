#pragma once
#include <externVars.hpp>
#ifdef __USE_IMGUI
namespace IMGUI_Loop{
    namespace HostCheckPoints{
        void kickstartParallelProcessing();
        void forceUICompletion();
        void forceRenderCompletion();
        void triggerAbortionProtocol();

    }
}
#endif