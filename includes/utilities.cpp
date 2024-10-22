#include <utilities.hpp>
#include <random>


SSBO_bundle::SSBO_bundle(std::vector<uint> SIZE, int bindPoint_) : bindPoint(bindPoint_) {
	if(ssbo){
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glDeleteBuffers(1, &ssbo);
	}
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE[pY]*SIZE[pX]*sizeof(int), nullptr, GL_DYNAMIC_DRAW);
	if(bindPoint != -1) {
		printf("Binding SSBO to bindPoint %d\n", bindPoint);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
	}
	#ifdef __CUDACC__s
	if(cuda_ssbo_resource) cudaGraphicsUnregisterResource(cuda_ssbo_resource);
	cudaGraphicsGLRegisterBuffer(&cuda_ssbo_resource, ssbo, cudaGraphicsMapFlagsWriteDiscard);

	cudaGraphicsMapResources(1, &cuda_ssbo_resource, 0);
	cudaGraphicsResourceGetMappedPointer(&dSSBO, &num_bytes, cuda_ssbo_resource);
	cudaGraphicsUnmapResources(1, &cuda_ssbo_resource, 0);
	#endif
}

SSBO_bundle::~SSBO_bundle(){
	#ifdef __CUDACC__s
	if (cuda_ssbo_resource) {
		cudaGraphicsUnregisterResource(cuda_ssbo_resource);
		cuda_ssbo_resource = nullptr;
	}
	#endif

	// Delete the OpenGL buffer
	if (ssbo) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, 0);
		glDeleteBuffers(1, &ssbo);
		ssbo = 0;
	}
		bindPoint = -1;
}
void SSBO_bundle::bind(int bindPoint_){
	bindPoint = bindPoint_;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	#ifdef __CUDACC__s
	if (!cuda_ssbo_resource) {
		throw std::runtime_error("CUDA resource not initialized");
	}
	#endif
}


std::string reformErrorString(const std::string& fileName, const int lineID, const std::string& thisString)
{
	std::ostringstream oss;
	oss << lineID;
	
	return "(" + fileName + "::\033[1;35m" + oss.str() + "\033[0m) - : " + thisString;
}


void SetRandomColor() {
    std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 255);
	
	color_factor[0] = (dis(gen))/255.0; //r 
	color_factor[1] = (dis(gen))/255.0; //g
	color_factor[2] = (dis(gen))/255.0; //b
}
