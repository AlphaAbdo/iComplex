#pragma once
#include <macros.hpp>

#include <VarsPool.hpp>
#include <mainWindow.hpp>

#ifdef __CUDACC__s
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#endif


struct SSBO_bundle{
	GLuint ssbo = 0;
	int bindPoint = -1;
	#ifdef __CUDACC__s
	struct cudaGraphicsResource *cuda_ssbo_resource = nullptr;
	void* dSSBO  = nullptr;
	size_t num_bytes = 0;
	#endif

	SSBO_bundle(){};


    SSBO_bundle(std::vector<uint> SIZE, int bindPoint_);

	template <typename T>
	SSBO_bundle(T SIZE, int bindPoint_){
        SSBO_bundle(std::vector(SIZE[pX],SIZE[pY]), bindPoint_);
    }

    
	~SSBO_bundle();
	void bind(int bindPoint_);

};

extern std::string reformErrorString(const std::string& fileName, const int lineID, const std::string& thisString);
extern void SetRandomColor();
