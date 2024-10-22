#pragma once
#include <macros.hpp>

#include <VarsPool.hpp>
#include <mainWindow.hpp>
#include <utilities.hpp>

#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <future>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#ifdef __CUDACC__s
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#endif


#ifdef __USE_IMGUI

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"

#include "external/implot/implot.h"
#include "external/implot/implot_internal.h"

namespace betaWindow{
	namespace DeferredResults{
		#ifdef __CUDACC__s
		inline void *cudaLib = nullptr;
		inline CleanupCUDA cleanupCUDA;
		inline executeCudaFrameProcessor ExecuteCudaFrameProcessor;
		inline preAllocateMemory PreAllocateMemory;
		inline InitializeCUDA initializeCUDA ;
		inline std::future<void> init_cuda_future = std::future<void>();
		#endif
	}

	inline bool enableIMGUI = true;
	inline ImGuiIO* p_io = nullptr;

	extern void launchUI();
	extern void init_betaWindow();
	extern void Render();
	extern void DerefferedAssign();;
	extern void Shutdown();
}
#endif
