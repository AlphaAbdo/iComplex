#include <beta_helper.hpp>

#ifdef __USE_IMGUI

void HandleSecondaryWindowDocking()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    dockingState.size.y = viewport->Size.y;

    ImVec2 windowPos;
    if (dockingState.position == DockingPosition::Left) {
        windowPos = viewport->Pos;
    } else if (dockingState.position == DockingPosition::Right) {
        windowPos = ImVec2(viewport->Pos.x + viewport->Size.x - dockingState.size.x, viewport->Pos.y);
    } else {
        windowPos = dockingState.customPosition;
    }

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dockingState.size, ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;

    ImGui::Begin("Sticky Window", nullptr, window_flags);

    ImGui::BeginGroup();
    // Custom title bar for dragging
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
    if (ImGui::Button("Drag Here", ImVec2(ImGui::GetWindowWidth() - 16, 20))) {}
    ImGui::PopStyleColor();

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (dockingState.position != DockingPosition::Custom) {
            dockingState.position = DockingPosition::Custom;
        }
        dockingState.customPosition.x += ImGui::GetIO().MouseDelta.x;
        dockingState.customPosition.y += ImGui::GetIO().MouseDelta.y;
    }

    ImGui::SetNextItemWidth(75);
    if (ImGui::BeginCombo("Docking Position", DockingPositionToString(dockingState.position))) {
        if (ImGui::Selectable("Left", dockingState.position == DockingPosition::Left)) {
            dockingState.position = DockingPosition::Left;
        }
        if (ImGui::Selectable("Right", dockingState.position == DockingPosition::Right)) {
            dockingState.position = DockingPosition::Right;
        }
        if (ImGui::Selectable("Custom", dockingState.position == DockingPosition::Custom)) {
            dockingState.position = DockingPosition::Custom;
            dockingState.customPosition = ImGui::GetWindowPos();
        }
        ImGui::EndCombo();
    }
    ImGui::EndGroup();

    
    // Update size after potential resizing
    ImVec2 newSize = ImGui::GetWindowSize();
    if (newSize.x != dockingState.size.x) {
        dockingState.size.x = newSize.x;
        if (dockingState.position == DockingPosition::Right) {
            dockingState.customPosition.x = viewport->Pos.x + viewport->Size.x - dockingState.size.x;
        }
    }
    dockingState.size.y = newSize.y;

}

inline void SecondaryWindow() {
    
    HandleSecondaryWindowDocking();

    ImGui::Separator();
    RenderFPSPlot();

    ImGui::SeparatorText("general manipulation");
    gManipulation();
   



    ImGui::SeparatorText("Color Management");

    ColorPicker();


    ImGui::SeparatorText("CheckPoints");

    CheckPoints();

    ImGui::SeparatorText("Optimization layers");

    if (ImGui::BeginMenu("Blur Shader")) {
        for(int i = 0; i< (int) mainWindow::BLurShaderMapper.size(); i++)
        {
            if (ImGui::MenuItem(mainWindow::BLurShaderMapper[i].title.c_str(), NULL, mainWindow::selectedBlurShaderID == i))
            {  
                mainWindow::selectedBlurShaderID = (mainWindow::selectedBlurShaderID !=i)? i: 0;
            }
        }

        ImGui::EndMenu();
    }
    #ifdef __CUDACC__s
    ImGui::Checkbox("Render with Cuda", &CudaRender);
    if(CudaRender){
        // ImGui::SameLine(0, 10);
        if(ImGui::Button("Refresh", ImVec2(75, 0)))
        {
            betaWindow::DeferredResults::frameState = fSTATE::FULL_COMPUTE;
        }
        ImGui::SameLine(0, 10);
        ImGui::Checkbox("Cuda-Optimization", &betaWindow::OptimizeCUDA);
        
    }
    #endif
    
    ImGui::SeparatorText("Extra Features");
    
    if (ImGui::BeginMenu("reload Components")) {
        if(ImGui::Selectable("reload DisDepGen.glsl"))
        {
            betaWindow::DeferredResults::reloadDisDepGenShader = true;
        }
        #ifdef __CUDACC__s
        if(ImGui::Selectable("reload cuda library"))
        {
            if(!betaWindow::DeferredResults::init_cuda_future.valid())
            {
                betaWindow::DeferredResults::init_cuda_future = std::async(std::launch::async, mainWindow::init_ParallelCuda);
            }
        } 
        #endif
        ImGui::EndMenu();
    }

    //check bo for showStripsOverlay
    ImGui::Checkbox("Show Strips Overlay", &showStripsOverlay);

    ImGui::Checkbox("Anchor background", &isBackgroundAnchored);

    

    ImGui::End();
}

void betaWindow::launchUI(){
    SecondaryWindow();
}

void betaWindow::init_betaWindow() {
    using namespace betaWindow;

    const char* glsl_version = "#version 430";

    // Check ImGui version
    IMGUI_CHECKVERSION();

    // Create ImGui context
    ImGui::CreateContext();
    p_io = &ImGui::GetIO();
    if (!p_io) {
        ImGui::DestroyContext(); // Cleanup if context creation fails
        throw std::runtime_error(reformErrorString (__FILE__, __LINE__, "Failed to create ImGui context."));
    }
    ImPlot::CreateContext();

    // Configure ImGui flags
    p_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    p_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    p_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    // Set style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (p_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Initialize platform/renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(mainWindow::window, true)) {
        ImPlot::DestroyContext();
        ImGui::DestroyContext(); // Cleanup ImGui context
        throw std::runtime_error(reformErrorString (__FILE__, __LINE__, "Failed to initialize ImGui GLFW backend."));
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        ImGui_ImplGlfw_Shutdown();  // Cleanup GLFW backend
        ImPlot::DestroyContext();
        ImGui::DestroyContext();    // Cleanup ImGui context
        throw std::runtime_error(reformErrorString (__FILE__, __LINE__, "Failed to initialize ImGui OpenGL3 backend."));
    }
}

void qRenderPlatformWindowsDefault(void* platform_render_arg, void* renderer_render_arg)
{
    // Skip the main viewport (index 0), which is always fully handled by the application!
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int i = 1; i < platform_io.Viewports.Size; i++)
    {
        ImGuiViewport* viewport = platform_io.Viewports[i];
        if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
            continue;
        if (platform_io.Platform_RenderWindow) platform_io.Platform_RenderWindow(viewport, platform_render_arg);
		printf("Rendering viewport %d\n", i);
        if (platform_io.Renderer_RenderWindow) platform_io.Renderer_RenderWindow(viewport, renderer_render_arg);
    }
    for (int i = 1; i < platform_io.Viewports.Size; i++)
    {
        ImGuiViewport* viewport = platform_io.Viewports[i];
        if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
            continue;
        if (platform_io.Platform_SwapBuffers) platform_io.Platform_SwapBuffers(viewport, platform_render_arg);
        if (platform_io.Renderer_SwapBuffers) platform_io.Renderer_SwapBuffers(viewport, renderer_render_arg);
    }
}

void betaWindow::Render()
{
    using namespace betaWindow;
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    if (betaWindow::p_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void betaWindow::DerefferedAssign()
{   
    if(betaWindow::DeferredResults::tmp_scale != 0.0)
    {
        if(scale == 0.0) scale = 1.0;
        else{
            double dscale = pow(1.005, (betaWindow::DeferredResults::tmp_scale));
            double s = SCR_SIZE[pY] * scale / SCR_SIZE[pX];
            cOff[pX] +=  (0.5)* (1-dscale)*scale;
            cOff[pY] +=  (0.5)* (1-dscale)*s;
            scale *= dscale;
        }
        
        betaWindow::DeferredResults::tmp_scale = 0;
        frameState = fSTATE::FULL_COMPUTE;
    }
    if(betaWindow::DeferredResults::tmp_cOff[pX])
    {
        cOff[pX] += (betaWindow::DeferredResults::tmp_cOff[pX]) * (scale/SCR_SIZE[pX]);
        betaWindow::DeferredResults::tmp_cOff[pX] = 0;
        frameState = fSTATE::FRAME_SHIFT;
    }
    if(betaWindow::DeferredResults::tmp_cOff[pY])
    {
        cOff[pY] += (betaWindow::DeferredResults::tmp_cOff[pY]) * (scale/SCR_SIZE[pY]);
        betaWindow::DeferredResults::tmp_cOff[pY] = 0;
        frameState = fSTATE::FRAME_SHIFT;
    }
    if(!betaWindow::OptimizeCUDA)
    {
        betaWindow::DeferredResults::frameState = fSTATE::FULL_COMPUTE;
    }

    if(betaWindow::DeferredResults::precisionMode != -1)
    {
        ::precisionMode = betaWindow::DeferredResults::precisionMode;
        betaWindow::DeferredResults::precisionMode = -1;
        frameState = fSTATE::FULL_COMPUTE;
    }
    if(betaWindow::DeferredResults::temp_depth)
    {
        crunch = static_cast<int>(betaWindow::DeferredResults::temp_depth);
        betaWindow::DeferredResults::temp_depth = 0;
        frameState = fSTATE::FULL_COMPUTE;
    }


    if(betaWindow::DeferredResults::cOff_modified){
        frameState = fSTATE::FULL_COMPUTE;
    }
    if(betaWindow::DeferredResults::frameState != fSTATE::UNKNOWN_STATE){
        frameState = betaWindow::DeferredResults::frameState;
        betaWindow::DeferredResults::frameState = fSTATE::UNKNOWN_STATE;
    }

    static auto old_selectedBlurShaderID = mainWindow::selectedBlurShaderID;


    if(mainWindow::selectedBlurShaderID && old_selectedBlurShaderID != mainWindow::selectedBlurShaderID){
        printf("Reloading Blur Shader %d\n", mainWindow::selectedBlurShaderID);
        try{
            
            if(mainWindow::BlurShader)
            {
                glDeleteProgram(mainWindow::BlurShader->ID);
                delete mainWindow::BlurShader;
                mainWindow::BlurShader= nullptr;

            }
            mainWindow::BlurShader = mainWindow::unsafeLoad_BlurShader(mainWindow::BLurShaderMapper[mainWindow::selectedBlurShaderID].title);
            
        }catch(std::exception& e){
            std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
        }
        old_selectedBlurShaderID = mainWindow::selectedBlurShaderID;

        frameState = fSTATE::FULL_COMPUTE;
    }
    else if(old_selectedBlurShaderID != mainWindow::selectedBlurShaderID){ 
        if(mainWindow::BlurShader)
        {
            glDeleteProgram(mainWindow::BlurShader->ID);
            delete mainWindow::BlurShader;
            mainWindow::BlurShader= nullptr;

        }
        old_selectedBlurShaderID = mainWindow::selectedBlurShaderID;
        frameState = fSTATE::FULL_COMPUTE;
    }
    if(betaWindow::DeferredResults::reloadDisDepGenShader){
        try{
            mainWindow::reloadDisDepGenShader(false);
        }
        catch(std::exception& e){
            std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
        }
        betaWindow::DeferredResults::reloadDisDepGenShader = false;
    }
    #ifdef __CUDACC__s
    static int Supercounter = 0;
    if(betaWindow::DeferredResults::init_cuda_future.valid()) {
        Supercounter++;
        auto status = betaWindow::DeferredResults::init_cuda_future.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            try {
                betaWindow::DeferredResults::init_cuda_future.get();
                #if defined(_WIN32) || defined(_WIN64)
                FreeLibrary(mainWindow::cudaLib);
                #else
                dlclose(mainWindow::cudaLib);
                #endif
                // printf("kek!\n");
                // dlclose(betaWindow::DeferredResults::cudaLib);
                mainWindow::cudaLib = betaWindow::DeferredResults::cudaLib;
                betaWindow::DeferredResults::cudaLib = nullptr;

                mainWindow::cleanupCUDA = betaWindow::DeferredResults::cleanupCUDA;
                betaWindow::DeferredResults::cleanupCUDA = nullptr;

                mainWindow::ExecuteCudaFrameProcessor = betaWindow::DeferredResults::ExecuteCudaFrameProcessor;
                betaWindow::DeferredResults::ExecuteCudaFrameProcessor = nullptr;

                mainWindow::PreAllocateMemory = betaWindow::DeferredResults::PreAllocateMemory;
                betaWindow::DeferredResults::PreAllocateMemory = nullptr;

                mainWindow::initializeCUDA = betaWindow::DeferredResults::initializeCUDA;
                betaWindow::DeferredResults::initializeCUDA = nullptr;

                dlclose(mainWindow::cudaLib);
                
                frameState = fSTATE::FULL_COMPUTE;

            } catch (const std::exception& e) {
                std::cerr << "Exception\033[1;31m caught \033[0m(" << __FILE__ << "::\033[1;35m" << __LINE__ << "\033[0m)<-" << e.what() << std::endl;
            }
            betaWindow::DeferredResults::init_cuda_future = std::future<void>();
        }
    } 
    else if(Supercounter > 0){
        printf("Skipped %d frame to reset cuda lib\n", Supercounter);
        Supercounter = 0;
    }
    #endif
}

void betaWindow::Shutdown()
{
    using namespace betaWindow;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}
#endif