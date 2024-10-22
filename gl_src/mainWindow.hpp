#pragma once
#include <macros.hpp>

#include <includes/Shader.hpp>
#include <includes/ComputeShader.hpp>
#include "external/GLFW/glfw3.h"

struct SSBO_bundle;

using CleanupCUDA = void(*)(void);
using executeCudaFrameProcessor = void(*)(int*);
using preAllocateMemory = void(*)(std::vector<unsigned int>, std::vector<int> bufferOffset, int CudaRender);
using InitializeCUDA = void(*)(std::vector<unsigned int>* , std::vector<double>*,
                                int* ,double*, fSTATE*, int* ,int*);

struct BlurShaderInformation{
	std::string title;
};
namespace mainWindow{
	inline GLFWwindow* window{nullptr};

	inline Shader* screenQuad{nullptr};
	inline ComputeShader* computeShader{nullptr};
	inline ComputeShader* DisDepGenShader{nullptr};
	inline ComputeShader* BlurShader{nullptr};
	inline std::vector<BlurShaderInformation> BLurShaderMapper =   {{"No-Effect"}, 
																	{"Discret3x3GaussianBlur"}, 
																	{"DirectionalBlur"}, 
																	{"BilateralFilter"} };
	inline int selectedBlurShaderID = 1;
   
	inline int temppass = 0;

	inline unsigned int quadVAO;
	inline unsigned int quadVBO;
	inline SSBO_bundle* rawSSBO = nullptr;
	inline SSBO_bundle* finalSSBO = nullptr;


	#ifdef __CUDACC__s
	inline void *cudaLib = nullptr;
	inline CleanupCUDA cleanupCUDA;
	inline executeCudaFrameProcessor ExecuteCudaFrameProcessor;
	inline preAllocateMemory PreAllocateMemory;
	inline InitializeCUDA initializeCUDA ;
	#endif


	extern void init_mainWindow();
	extern ComputeShader* unsafeLoad_DisDepGenShader();
	extern void reloadDisDepGenShader(bool is_silent);
	extern ComputeShader* unsafeLoad_BlurShader(std::string shaderTitle);
	extern void reloadBlurShader(bool is_silent, std::string shaderTitle);
	inline void reloadBlurShader(bool is_silent)
	{
		if(selectedBlurShaderID > 0 && selectedBlurShaderID < (int)BLurShaderMapper.size())
			reloadBlurShader(is_silent, BLurShaderMapper[selectedBlurShaderID].title);
	}
	extern void reloadComputeShader();
	extern void init_Shaders();
	extern void manageTitleBar();
	#ifdef __CUDACC__s
	
	extern void init_Cuda();
	extern void init_ParallelCuda();
	#endif

	extern void screenRender();

	extern void reAdjustMainWindow(int SCR_POS[2]);
	
	extern const std::string GetFpsString(const double& FPS);
	extern void useComputePipeline();
	extern void useEffectPipeline();
}