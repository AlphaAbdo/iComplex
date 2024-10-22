#include <externVars.hpp>
#include <gl_Callbacks.hpp>
#include <iomanip> // for precision
#include <queue>



void mainWindow::init_mainWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	window = glfwCreateWindow(SCR_SIZE[pX], SCR_SIZE[pY], "too", NULL, NULL);
	if (window == NULL)
	{
		throw std::runtime_error(reformErrorString (__FILE__ ,  __LINE__ , "Failed to create GLFW window"));
	}
	glfwMakeContextCurrent(window);

	glfwSetErrorCallback(glfw_error_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetCharCallback(window, char_callback);
	glfwSetKeyCallback(window, key_callback);

	// Set mouse button and cursor position callbacks
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);


	glfwSwapInterval(0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error(reformErrorString (__FILE__ ,  __LINE__ , "Failed to initialize GLAD"));
		// return -1;
	}
	screenQuad = new Shader("shaders/screenQuad.vs", "shaders/screenQuad.fs");
	{
		screenQuad->use();
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	rawSSBO = new SSBO_bundle(SCR_SIZE, 0);
	finalSSBO = new SSBO_bundle(SCR_SIZE, 1);
}

ComputeShader* mainWindow::unsafeLoad_DisDepGenShader()
{
	return new ComputeShader("shaders/DisDepGen.glsl","shaders/DefInc.glsl", "shaders/ModedFunctionDef.glsl");
}

void mainWindow::reloadDisDepGenShader(bool is_silent)
{
	if(DisDepGenShader)
	{	
		glDeleteProgram(DisDepGenShader->ID);
		delete DisDepGenShader;
		if(!is_silent) std::cout << "Reloading DisDepGen Shader" << std::endl;
	}
	try{
		DisDepGenShader = unsafeLoad_DisDepGenShader();
	}catch(std::exception& e){
		std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
	}
}

ComputeShader* mainWindow::unsafeLoad_BlurShader(std::string shaderTitle)
{
	std::ostringstream oss;
	oss << "shaders/blur/" << shaderTitle << ".glsl";
	return new ComputeShader(oss.str().c_str());
}

void mainWindow::reloadBlurShader(bool is_silent, std::string shaderTitle)
{
	if(BlurShader)
	{	
		glDeleteProgram(BlurShader->ID);
		delete BlurShader;
		if(!is_silent) std::cout << "Reloading Blur Shader" << std::endl;
	}
	try{
		BlurShader = unsafeLoad_BlurShader(shaderTitle);
		
	}catch(std::exception& e){
		std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
	}
}

void mainWindow::reloadComputeShader()
{
	if(computeShader)
	{	
		glDeleteProgram(computeShader->ID);
		delete computeShader;
		std::cout << "Reloading Compute Shader" << std::endl;
	}
	try{
		switch(precisionMode)
		{
			case 0:
				// computeShader = new  ComputeShader("shaders/FunctionCalculation_cs.glsl");
				computeShader = unsafeLoad_DisDepGenShader();
				break;
		}
	}catch(std::exception& e){
		std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
	}
}

void mainWindow::init_Shaders(){
	reloadComputeShader();
	reloadDisDepGenShader(false);
	reloadBlurShader(false);
}

void mainWindow::manageTitleBar() {
    // Static variables to maintain state between function calls
    static auto prevUpdateTime = std::chrono::high_resolution_clock::now();
    static auto prevFrameTime = std::chrono::high_resolution_clock::now();
    static int frameCount = 0;
    static const double timeFactor = 1e6;
    static const double updateInterval = 0.33 * timeFactor; // 0.33 seconds

    // Calculate the time elapsed since the last update
    auto currTime = std::chrono::high_resolution_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::microseconds>(currTime - prevUpdateTime).count();
    double frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(currTime - prevFrameTime).count() / timeFactor;
    frameCount++;

    // Check if the elapsed time exceeds the update interval
    if (timeSinceLastUpdate > updateInterval) {
        // Calculate the FPS (frames per second)
        double fps = (frameCount * timeFactor) / timeSinceLastUpdate;

        // Update the window title with the new FPS value
        glfwSetWindowTitle(window, GetFpsString(fps).c_str());

        prevUpdateTime = currTime;
        frameCount = 0;
    }
    prevFrameTime = currTime;
    realLoopTime = frameDuration;

    static std::queue<double> frameTimesQueue;
    static double frameTimeSum = 0;

    frameTimeSum += frameDuration;
    frameTimesQueue.push(frameDuration);
    while (frameTimeSum > 1 && frameTimesQueue.size() > 1) {
        frameTimeSum -= frameTimesQueue.front();
        frameTimesQueue.pop();
    }
    avgLoopTime = frameTimeSum / frameTimesQueue.size();
}

#ifdef __CUDACC__s

inline void checkCudaError(cudaError_t err, const char* file = __FILE__, int line = __LINE__) {
	if (err != cudaSuccess) {
		throw std::runtime_error(reformErrorString (file ,  line , "CUDA Error: " + std::string(cudaGetErrorString(err)) ));
	}
}

inline void checkDLError(const char* error, const char* file = __FILE__, int line = __LINE__) {
	if (error) {
		throw std::runtime_error(reformErrorString (file ,  line , "Error loading function: " + std::string(error)));
	}
}
#ifdef __USE_IMGUI
void mainWindow::init_ParallelCuda() {
	int result = std::system("cmake --build ./build --target build_cuda_lib  --parallel 4");
	if(result == 0)
	{
		printf("reached here\n");
		try{
			{
				#if defined(_WIN32) || defined(_WIN64)
					// Windows code - using LoadLibrary()
					betaWindow::DeferredResults::cudaLib = LoadLibrary("./build/libcuda_src.dll");
				#else
					// Linux code - dlopen() sequence with renaming

					// Static variable to store the temporary name
					static std::string tempLibName;

					// Variable for iterating the temporary file names
					static int renameCounter = 0;

					// Create a new temporary name using the counter
					std::string baseName = "./build/libcuda_src.so";
					tempLibName = "./build/tmp_" + std::to_string(renameCounter++) + "_libcuda_src.so";

					// Rename the original file to the temporary one
					if (rename(baseName.c_str(), tempLibName.c_str()) != 0) {
						std::cerr << "Error renaming the library file!" << std::endl;
						// Handle error (maybe return early or take appropriate action)
					}

					// Load the renamed library
					betaWindow::DeferredResults::cudaLib = dlopen(tempLibName.c_str(), RTLD_NOW | RTLD_LOCAL);
					
					// Check if loading succeeded
					if (!betaWindow::DeferredResults::cudaLib) {
						std::cerr << "Error loading library: " << dlerror() << std::endl;
						// Restore the original name before exiting
						rename(tempLibName.c_str(), baseName.c_str());
					} else {
						// Delete the temporary file after loading successfully
						if (rename(tempLibName.c_str(), baseName.c_str()) != 0) {
							std::cerr << "Error restoring the original library file name!" << std::endl;
						}
					}
				#endif
			}
			// // Load the shared library
			// #if defined(_WIN32) || defined(_WIN64)
			// 	betaWindow::DeferredResults::cudaLib = LoadLibrary("./build/libcuda_src.dll");
			// #else
			// 	betaWindow::DeferredResults::cudaLib = dlopen("./build/libcuda_src.so",  RTLD_NOW | RTLD_LOCAL);
			// #endif
			// if (!betaWindow::DeferredResults::cudaLib) {
			// 	checkDLError(dlerror(), __FILE__, __LINE__);
			// }
			printf("here\n");
			#if defined(_WIN32) || defined(_WIN64)
			betaWindow::DeferredResults::ExecuteCudaFrameProcessor = (executeCudaFrameProcessor)GetProcAddress(betaWindow::DeferredResults::cudaLib, "ExecuteCudaFrameProcessor");
			checkDLError(GetLastError(), __FILE__, __LINE__);

			betaWindow::DeferredResults::PreAllocateMemory = (preAllocateMemory)GetProcAddress(betaWindow::DeferredResults::cudaLib, "PreAllocateMemory");
			checkDLError(GetLastError(), __FILE__, __LINE__);

			betaWindow::DeferredResults::initializeCUDA = (InitializeCUDA)GetProcAddress(betaWindow::DeferredResults::cudaLib, "InitializeCUDA");
			checkDLError(GetLastError(), __FILE__, __LINE__);

			betaWindow::DeferredResults::cleanupCUDA = (CleanupCUDA)GetProcAddress(betaWindow::DeferredResults::cudaLib, "CleanupCUDA");
			checkDLError(GetLastError(), __FILE__, __LINE__);
		#else
			betaWindow::DeferredResults::ExecuteCudaFrameProcessor = (executeCudaFrameProcessor)dlsym(betaWindow::DeferredResults::cudaLib, "ExecuteCudaFrameProcessor");
			checkDLError(dlerror(), __FILE__, __LINE__);

			betaWindow::DeferredResults::PreAllocateMemory = (preAllocateMemory)dlsym(betaWindow::DeferredResults::cudaLib, "PreAllocateMemory");
			checkDLError(dlerror(), __FILE__, __LINE__);

			betaWindow::DeferredResults::initializeCUDA = (InitializeCUDA)dlsym(betaWindow::DeferredResults::cudaLib, "InitializeCUDA");
			checkDLError(dlerror(), __FILE__, __LINE__);

			betaWindow::DeferredResults::cleanupCUDA = (CleanupCUDA)dlsym(betaWindow::DeferredResults::cudaLib, "CleanupCUDA");
			checkDLError(dlerror(), __FILE__, __LINE__);
		#endif



			betaWindow::DeferredResults::initializeCUDA(&SCR_SIZE  , &cOff,
							&crunch, &scale, &frameState, &precisionMode ,&temppass);

		}catch (std::exception& e){
			#if defined(_WIN32) || defined(_WIN64)
				if (betaWindow::DeferredResults::cudaLib) {
					FreeLibrary(betaWindow::DeferredResults::cudaLib);
					betaWindow::DeferredResults::cudaLib = nullptr;
				}
			#else
				if (betaWindow::DeferredResults::cudaLib) {
					dlclose(betaWindow::DeferredResults::cudaLib);
					betaWindow::DeferredResults::cudaLib = nullptr;
				}
			#endif
			std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
		}

	
	}
}
#endif

#endif

#ifdef __CUDACC__s
void mainWindow::init_Cuda() {
	// Load the shared library
	#if defined(_WIN32) || defined(_WIN64)
    	cudaLib = LoadLibrary("./build/libcuda_src..dll");
	#else
		cudaLib = dlopen("./build/libcuda_src.so", RTLD_NOW | RTLD_LOCAL);
	#endif
	if (!cudaLib) {
		checkDLError(dlerror(), __FILE__, __LINE__);
	}

	#if defined(_WIN32) || defined(_WIN64)
    ExecuteCudaFrameProcessor = (executeCudaFrameProcessor)GetProcAddress(cudaLib, "ExecuteCudaFrameProcessor");
    checkDLError(GetLastError(), __FILE__, __LINE__);

    PreAllocateMemory = (preAllocateMemory)GetProcAddress(cudaLib, "PreAllocateMemory");
    checkDLError(GetLastError(), __FILE__, __LINE__);

    initializeCUDA = (InitializeCUDA)GetProcAddress(cudaLib, "InitializeCUDA");
    checkDLError(GetLastError(), __FILE__, __LINE__);

    cleanupCUDA = (CleanupCUDA)GetProcAddress(cudaLib, "CleanupCUDA");
    checkDLError(GetLastError(), __FILE__, __LINE__);
#else
    ExecuteCudaFrameProcessor = (executeCudaFrameProcessor)dlsym(cudaLib, "ExecuteCudaFrameProcessor");
    checkDLError(dlerror(), __FILE__, __LINE__);

    PreAllocateMemory = (preAllocateMemory)dlsym(cudaLib, "PreAllocateMemory");
    checkDLError(dlerror(), __FILE__, __LINE__);

    initializeCUDA = (InitializeCUDA)dlsym(cudaLib, "InitializeCUDA");
    checkDLError(dlerror(), __FILE__, __LINE__);

    cleanupCUDA = (CleanupCUDA)dlsym(cudaLib, "CleanupCUDA");
    checkDLError(dlerror(), __FILE__, __LINE__);
#endif



	initializeCUDA(&SCR_SIZE  , &cOff,
					&crunch, &scale, &frameState, &precisionMode ,&temppass);

}
#endif
#include <array>

void DrawLineFromPoints(const std::vector<std::array<float, 2>>& points, float lineWidth, bool DiffusionSTrip = false) {
    static GLuint program = 0;
    static GLuint vao = 0, vbo = 0;

    // Vertex Shader Source
    const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 position;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
    }
    )";

    // Fragment Shader Source
    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
	uniform bool DiffusionStrip;
    void main() {
		if(!DiffusionStrip)
        	FragColor = vec4(0.0, 0.0, 0.0, 0.4);
		else
			FragColor = vec4(1.0, 1.0, 1.0, 0.5);
    }
    )";

    // Only compile and link shaders once
    if (program == 0) {
        // Compile Vertex Shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        // Compile Fragment Shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        // Link Shaders into Program
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Clean up shaders (no longer needed after linking)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Create VAO and VBO
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        // Bind VAO and VBO
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Set vertex attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader program
    glUseProgram(program);

    // Bind VAO and VBO, upload vertex data
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(std::array<float, 2>), points.data(), GL_STATIC_DRAW);
	
	//set up the uniform
	glUniform1i(glGetUniformLocation(program, "DiffusionStrip"), DiffusionSTrip);


    // Set the line width (this is a state change, not a uniform)
    glLineWidth(lineWidth);


    // Draw the line
    glDrawArrays(GL_LINES, 0, points.size());

    // Unbind VAO
    glBindVertexArray(0);
}


void mainWindow::screenRender(){
	{
		mainWindow::screenQuad->use();

		mainWindow::screenQuad->setInt("SCR_WIDTH", SCR_SIZE[pX]);
		mainWindow::screenQuad->setInt("SCR_HEIGHT", SCR_SIZE[pY]);
		mainWindow::screenQuad->setInt("crunch",crunch);
		mainWindow::screenQuad->setVec3("colors",color_factor[0]*color_scaler,color_factor[1]*color_scaler,color_factor[2]*color_scaler);
		mainWindow::screenQuad->setFloat("animationFrame",animationFrame);

		glBindVertexArray(mainWindow::quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		static unsigned int VAO = 0, VBO = 0;
		if(!VAO)
		{
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		}

		if(showStripsOverlay)
		{
			std::vector<double> cOff_NDC = {
				-2*cOff[pX]/scale - 1,
				-2 *cOff[pY]/scale * static_cast<double>(SCR_SIZE[pX]) / SCR_SIZE[pY] - 1.0
			};
			double stripThinkness = 3.0f;
			
			DrawLineFromPoints({{(float)cOff_NDC[pX], -1.0f} ,{(float)cOff_NDC[pX],1.0f},
								{-1.0f, (float)cOff_NDC[pY]} ,{1.0f,(float)cOff_NDC[pY]}}, stripThinkness +2 , true);
			DrawLineFromPoints({{-1.0f, (float)cOff_NDC[pY]} ,{1.0f,(float)cOff_NDC[pY]},
								{(float)cOff_NDC[pX], -1.0f} ,{(float)cOff_NDC[pX],1.0f}}, stripThinkness);

			std::vector<std::array<float, 2>> integerPoints;
			for(int i = int(cOff[pX]) ; -(cOff[pX] - i)/scale < 1 ; i++)
			{
				if(i == 0) continue;
				double i_cOffX_NDC =  -2*(cOff[pX] - i)/scale - 1 ;
				integerPoints.push_back({(float)i_cOffX_NDC, -1.0f});
				integerPoints.push_back({(float)i_cOffX_NDC, 1.0f});
			}
			DrawLineFromPoints(integerPoints, 1.0f, true);
		}

		glBindVertexArray(0);
	}
}

void mainWindow::reAdjustMainWindow(int SCR_POS[2]){
	
	#ifdef __CUDACC__s
	PreAllocateMemory(new_SCR_SIZE, std::vector<int>{bufferOffset[pX] - SCR_POS[pX], - bufferOffset[pY] + SCR_POS[pY]}, CudaRender);
	
	checkCudaError(cudaGetLastError(), __FILE__, __LINE__);
	#endif
}

const std::string mainWindow::GetFpsString(const double& FPS) {
	int precision = 2;
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(precision) << FPS;
	std::string fpsString = stream.str();

	std::string result = "<Parameter Space: ";
	result += fpsString + "\tFPS>";

	return result;
}

void mainWindow::useComputePipeline(){
		
	DisDepGenShader->use();
	double s = scale / (SCR_SIZE[pX]);
	DisDepGenShader->setVec2("Offset", cOff[pX], cOff[pY]);
	DisDepGenShader->setDouble("scale", s);
	DisDepGenShader->setInt("SCR_WIDTH",SCR_SIZE[pX]);
	DisDepGenShader->setInt("SCR_HEIGHT",SCR_SIZE[pY]);
	DisDepGenShader->setInt("crunch",crunch);
	DisDepGenShader->setBool("HasEffect",selectedBlurShaderID>0);

	glDispatchCompute((unsigned int) (SCR_SIZE[pX] +15)/16, (unsigned int)(SCR_SIZE[pY] +15)/16, 1);
	GLenum err;

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	while ((err = glGetError()) != GL_NO_ERROR) {
		// Handle or log the error
		std::cerr << "--------------------------OpenGL error: " << err << std::endl;
	}

}
void mainWindow::useEffectPipeline(){
	if(BlurShader == nullptr) return;
	BlurShader->use();
	BlurShader->setInt("SCR_WIDTH",SCR_SIZE[pX]);
	BlurShader->setInt("SCR_HEIGHT",SCR_SIZE[pY]);

	glDispatchCompute((unsigned int) (SCR_SIZE[pX] +15)/16, (unsigned int)(SCR_SIZE[pY] +15)/16, 1);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		// Handle the error
		std::cerr << "OpenGL Error: " << error << std::endl;
	}

}
