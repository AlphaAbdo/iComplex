#include <Computational_Loop.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>


extern bool shouldAllocateSSBO;
namespace Computational_Loop{

	inline std::thread *pp_Loop = nullptr;
    inline std::mutex mtx;
    inline std::condition_variable cv;

    enum class LoopState{
        LAUNCH_CUDA,
        CONTINUE_LOOP,
        EXIT_THREAD,
        LAUNCH_EFFECT
    };
    inline Computational_Loop::LoopState ready = Computational_Loop::LoopState::CONTINUE_LOOP;

    void parallelLoop();
}

void Computational_Loop::parallelLoop() {
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow* sharedWindow = glfwCreateWindow(1, 1, "", NULL, mainWindow::window);
	glfwMakeContextCurrent(sharedWindow);
	mainWindow::reloadDisDepGenShader(true);
	mainWindow::reloadBlurShader(true);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mainWindow::rawSSBO->ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mainWindow::finalSSBO->ssbo);

    while(true) {

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready!=LoopState::CONTINUE_LOOP ; });

		// int BlurFlag = mainWindow::selectedBlurShaderID
		if(ready == LoopState::EXIT_THREAD) 
				break;

		if(shouldAllocateSSBO){
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mainWindow::rawSSBO->ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mainWindow::finalSSBO->ssbo);
			frameState = fSTATE::FULL_COMPUTE;
		
		}

		// Compute the next frame
		#ifdef __CUDACC__s
		if(!CudaRender){
		#endif
			if(mainWindow::computeShader)
			{
				GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				mainWindow::useComputePipeline();
				GLenum waitResult = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1e9);

				glDeleteSync(sync);
			}
		#ifdef __CUDACC__s
		}
		else {
			SSBO_bundle* targetSSBO = (mainWindow::selectedBlurShaderID)? mainWindow::rawSSBO :mainWindow::finalSSBO;
			//unsafe to uncomment mapping resources.
			cudaGraphicsMapResources(1,&targetSSBO->cuda_ssbo_resource, 0);
			mainWindow::ExecuteCudaFrameProcessor(static_cast<int*>(targetSSBO->dSSBO));
			cudaGraphicsUnmapResources(1, &targetSSBO->cuda_ssbo_resource, 0);
		}
		#endif
		ready = LoopState::LAUNCH_EFFECT;
        cv.notify_one();

			// Ensure compute shader writes are visible to subsequent operations
		
		if(mainWindow::selectedBlurShaderID)
			mainWindow::useEffectPipeline();
		
		if(shouldAllocateSSBO){
			glFinish();
		}

        ready = LoopState::CONTINUE_LOOP;
        cv.notify_one();
	}
	printf("Exiting Parallel Loop\n");
}


void Computational_Loop::HostCheckPoints::verifyComputationCompletion()
{
    if(pp_Loop)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready!=LoopState::LAUNCH_CUDA ; });
    }
}

void Computational_Loop::HostCheckPoints::forceEffectCompletion()
{
    if(pp_Loop)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready==LoopState::CONTINUE_LOOP ; });
    }
}
//
void Computational_Loop::HostCheckPoints::kickstartParallelProcessing()
{
    if(!pp_Loop) pp_Loop = new std::thread(parallelLoop);
		{
			{
				std::lock_guard<std::mutex> lock(mtx);
				ready = LoopState::LAUNCH_CUDA;
			}
			cv.notify_one();
		}
}




void Computational_Loop::HostCheckPoints::triggerAbortionProtocol()
{
    {
		std::unique_lock<std::mutex> lock(mtx);
		
		// Wait until the condition is met
		cv.wait(lock, [] { return ready == LoopState::CONTINUE_LOOP; });
		
		// After the condition is met, update the state
		ready = LoopState::EXIT_THREAD;
	}
    cv.notify_one();
	pp_Loop->join();
	pp_Loop = nullptr;
}