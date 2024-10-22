#include "main.hpp"
#include <gl_Callbacks.hpp>


volatile sig_atomic_t signal_received = 0;

void signal_handler(int signal) {
    signal_received = signal;
}
#ifdef __CUDACC__s
int findCudaDevice(int argc, const char **argv) {
    int deviceCount;
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);

    if (error_id != cudaSuccess) {
        std::cerr << "cudaGetDeviceCount returned " << static_cast<int>(error_id) << "\n-> " 
                  << cudaGetErrorString(error_id) << "\nResult = FAIL\n";
        exit(EXIT_FAILURE);
    }

    if (deviceCount == 0) {
        std::cerr << "There are no available device(s) that support CUDA\n";
        exit(EXIT_FAILURE);
    }

    int dev = 0;
    if (argc > 1) {
        dev = atoi(argv[1]);
        if (dev < 0 || dev >= deviceCount) {
            std::cerr << "Invalid device ID " << dev << " specified.\n";
            exit(EXIT_FAILURE);
        }
    }

    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, dev);
    std::cout << "Using CUDA Device [" << dev << "]: " << deviceProp.name << "\n";

    cudaSetDevice(dev);
    return dev;
}

#endif

void print_message(const std::string& message) {
    std::cout << message << std::endl;
}

int main(int argc, char* argv[])
{
	std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler); // Handle SIGINT (Ctrl+C) as well

	SetRandomColor();

	#ifdef __CUDACC__s
	{
		int dev = 0;
		dev = findCudaDevice(argc, (const char **)argv);

		cudaDeviceProp deviceProp;
		cudaGetDeviceProperties(&deviceProp, dev);
		version = deviceProp.major * 10 + deviceProp.minor;

		numSMs = deviceProp.multiProcessorCount;
		printf("Data initialization done.\n");
	}
	#endif
	
	try {
		mainWindow::init_mainWindow();
		mainWindow::init_Shaders();
	} catch (const std::exception& e) {
		// Code to handle the exception
		std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
		return -1;
	}

	#ifdef __CUDACC__s
	try {
		mainWindow::init_Cuda();
	} catch (const std::exception& e) {
		// Code to handle the exception
		CudaRender = false;
		std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
		std::cerr << "Switching-Context to State: \033[1;32mGlsl\033[0m" << std::endl;
	}
	#endif
	
	

	printf("Starting Loop\n");
	
	#ifdef __USE_IMGUI
	try {
		betaWindow::init_betaWindow();
	} catch (const std::exception& e) {
		// Code to handle the exception
		std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
		betaWindow::p_io = nullptr;
	}
	#endif

	{
			glfwGetWindowPos(mainWindow::window, SCR_POS + pX, SCR_POS + pY);
			bufferOffset[pX] = SCR_POS[pX];
			bufferOffset[pY] = SCR_POS[pY];
	}
	

	//main loop.
	while (!glfwWindowShouldClose(mainWindow::window) && !signal_received)
	{
		#ifdef __USE_IMGUI
		if (glfwGetWindowAttrib(mainWindow::window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}
		#endif
		glfwPollEvents();

		#ifdef __USE_IMGUI
		if(betaWindow::p_io && betaWindow::enableIMGUI){
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			IMGUI_Loop::HostCheckPoints::kickstartParallelProcessing();

		}
		#endif
		
		mainWindow::manageTitleBar();

		
		Computational_Loop::HostCheckPoints::verifyComputationCompletion();

		if (animationStep) {
			animationFrame -= 10* animationStep * (avgLoopTime? avgLoopTime:1);
		}
		if (dscale != 1.0) {

			if(dscale <1)
			{
				double s = (1-dscale)*scale / SCR_SIZE[pX];
				cOff[pX] +=  (static_cast<double>(lastx)) *s;
				cOff[pY] +=  (static_cast<double>(SCR_SIZE[pY] - lasty)) *s;
				// frameState = fSTATE::ZOOM_PRELOAD; //(optional)
				frameState = fSTATE::FULL_COMPUTE;

			}

			if(dscale >1){
				double s = SCR_SIZE[pY] * scale / SCR_SIZE[pX];
				cOff[pX] +=  (0.5)* (1-dscale)*scale;
				cOff[pY] +=  (0.5)* (1-dscale)*s;
				frameState = fSTATE::FULL_COMPUTE;
			}
			
			scale *= dscale;
			
		}
		else if ((mOff[pX] != 0.0) || (mOff[pY] != 0.0)) {
			cOff[pX] += mOff[pX];
			cOff[pY] += mOff[pY];
			frameState = fSTATE::FRAME_SHIFT; 
		}
		
		shouldAllocateSSBO = false;
		SSBO_bundle *tmp_rawSSBO = nullptr, *tmp_finalSSBO = nullptr;

		
		glfwGetWindowPos(mainWindow::window, bufferOffset.data() + pX, bufferOffset.data() + pY);
		bool isSamePos = !isBackgroundAnchored || (bufferOffset[pX] == SCR_POS[pX] && bufferOffset[pY] == SCR_POS[pY]);
		if((new_SCR_SIZE[pX]!=SCR_SIZE[pX] || new_SCR_SIZE[pY]!=SCR_SIZE[pY]) || !isSamePos)
		{
			glFinish();
			shouldAllocateSSBO = true;
			cOff[pX] += (bufferOffset[pX] - SCR_POS[pX]) * (scale / SCR_SIZE[pX]);
			cOff[pY] += (int(SCR_SIZE[pY]) - int(new_SCR_SIZE[pY]) - bufferOffset[pY] + SCR_POS[pY]) * (scale / SCR_SIZE[pX]);
			
			scale *= (double)new_SCR_SIZE[pX] / SCR_SIZE[pX] ;

			
			mainWindow::reAdjustMainWindow(SCR_POS);

			tmp_rawSSBO = new SSBO_bundle(new_SCR_SIZE, -1);
			tmp_finalSSBO = new SSBO_bundle(new_SCR_SIZE, -1);
		}
		
		SCR_POS[pX] = bufferOffset[pX];
		SCR_POS[pY] = bufferOffset[pY];

		Computational_Loop::HostCheckPoints::forceEffectCompletion();

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		mainWindow::screenRender();
		#ifdef __USE_IMGUI
		betaWindow::DerefferedAssign();
		#endif


		if(shouldAllocateSSBO){
			SCR_SIZE[pX] = new_SCR_SIZE[pX];
			SCR_SIZE[pY] = new_SCR_SIZE[pY];

			try{
				delete mainWindow::rawSSBO;
				mainWindow::rawSSBO = tmp_rawSSBO;
				mainWindow::rawSSBO->bind(0);
				
				delete mainWindow::finalSSBO;
				mainWindow::finalSSBO = tmp_finalSSBO;
				mainWindow::finalSSBO->bind(1);
			}
			catch(const std::exception& e){
				delete mainWindow::rawSSBO;
				mainWindow::rawSSBO = new SSBO_bundle(SCR_SIZE, 0);

				delete mainWindow::finalSSBO;
				mainWindow::finalSSBO = new SSBO_bundle(SCR_SIZE, 1);

				std::cout << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
			}
			

		}

		glViewport(0, 0, SCR_SIZE[pX], SCR_SIZE[pY]);
		
		Computational_Loop::HostCheckPoints::kickstartParallelProcessing();

		#ifdef __USE_IMGUI
		if(betaWindow::p_io && betaWindow::enableIMGUI){
			IMGUI_Loop::HostCheckPoints::forceUICompletion();

			betaWindow::Render();
		}
		#endif



		glfwSwapBuffers(mainWindow::window);

		std::this_thread::sleep_for(std::chrono::milliseconds(00));
	}
	
	printf("Exiting Main Loop\n");

	Computational_Loop::HostCheckPoints::triggerAbortionProtocol();
	#ifdef __USE_IMGUI
	IMGUI_Loop::HostCheckPoints::triggerAbortionProtocol();
	
	if(betaWindow::p_io){
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	#endif

	if(mainWindow::BlurShader){
		glDeleteProgram(mainWindow::BlurShader->ID);
		delete mainWindow::BlurShader;
		mainWindow::BlurShader = nullptr;
	}
	if(mainWindow::screenQuad){
		glDeleteProgram(mainWindow::screenQuad->ID);
		delete mainWindow::screenQuad;
		mainWindow::screenQuad = nullptr;
	}
	if(mainWindow::computeShader){
		glDeleteProgram(mainWindow::computeShader->ID);
		delete mainWindow::computeShader;
		mainWindow::computeShader = nullptr;
	}
	if(mainWindow::DisDepGenShader){
		glDeleteProgram(mainWindow::DisDepGenShader->ID);
		delete mainWindow::DisDepGenShader;
		mainWindow::DisDepGenShader = nullptr;
	}
	
	glfwTerminate();
	printf("deleting program\n");
	
	#ifdef __CUDACC__s
	mainWindow::cleanupCUDA();
    if (mainWindow::cudaLib) {
		#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(mainWindow::cudaLib);
		#else
		dlclose(mainWindow::cudaLib);
		#endif
	}
	printf("freeing cuda\n");
	delete mainWindow::rawSSBO;
	delete mainWindow::finalSSBO;
	
	#endif

	return EXIT_SUCCESS;
}
