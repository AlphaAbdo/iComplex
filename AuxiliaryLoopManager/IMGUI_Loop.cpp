#include <IMGUI_Loop.hpp>

#ifdef __USE_IMGUI
extern bool shouldAllocateSSBO;
namespace IMGUI_Loop{

	inline std::thread *pp_Loop = nullptr;
    inline std::mutex mtx;
    inline std::condition_variable cv;

    enum class LoopState{
        START_IMGUI,
        CONTINUE_MAIN_LOOP,
        ABORT

    };
    inline LoopState ready = LoopState::CONTINUE_MAIN_LOOP;

    void parallelLoop();
}

void IMGUI_Loop::parallelLoop() {
    while(true) {

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready!=LoopState::CONTINUE_MAIN_LOOP ; });
        
        if(ready == LoopState::ABORT) 
            break;

        betaWindow::launchUI();
        
        ready = LoopState::CONTINUE_MAIN_LOOP;
        cv.notify_one();
    }
}

void IMGUI_Loop::HostCheckPoints::kickstartParallelProcessing()
{
    if(!pp_Loop) pp_Loop = new std::thread(parallelLoop);
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = LoopState::START_IMGUI;
        }
        cv.notify_one();
    }
}

void IMGUI_Loop::HostCheckPoints::forceUICompletion()
{
    if(pp_Loop)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready==LoopState::CONTINUE_MAIN_LOOP ; });
    }
}

void IMGUI_Loop::HostCheckPoints::triggerAbortionProtocol()
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait until the condition is met
        cv.wait(lock, [] { return ready == LoopState::CONTINUE_MAIN_LOOP; });
        
        // After the condition is met, update the state
        ready = LoopState::ABORT;
    }
    cv.notify_one();
    pp_Loop->join();
    pp_Loop = nullptr;
}
#endif