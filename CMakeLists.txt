cmake_minimum_required(VERSION 3.10)
project(iComplex LANGUAGES C CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Specify CUDA architectures
set(CMAKE_CUDA_ARCHITECTURES 60 61 70 75 80 86)

# Set policies to address warnings
cmake_policy(SET CMP0146 NEW)
cmake_policy(SET CMP0072 NEW)

# Set the output directory for executables to the source directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_package(CUDAToolkit)

# Check if CUDA is found

set(bENABLE_CUDA TRUE)

if (CUDAToolkit_FOUND)
    if (NOT ENABLE_CUDA)
        set(bENABLE_CUDA FALSE)
    endif()
else()
    set(bENABLE_CUDA FALSE)
endif()

if(bENABLE_CUDA)
    message(STATUS "CUDA found: ${CUDAToolkit_INCLUDE_DIRS}")
    enable_language(CUDA)
else()
    add_definitions(-DDISABLE_CUDA)
    message(STATUS "CUDA unused/found!")
endif()

if (glfw3_FOUND)
    message(STATUS "GLFW found: ${glfw3_INCLUDE_DIRS}")
    message(STATUS "GLFW libraries: ${glfw3_LIBRARIES}")
else()
    message(FATAL_ERROR "GLFW not found!")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/UI
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/implot
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_SOURCE_DIR}/shader
    ${CMAKE_SOURCE_DIR}/AuxiliaryLoopManager
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Common
    ${CMAKE_SOURCE_DIR}/cuda_src
    ${CMAKE_SOURCE_DIR}/gl_src
    ${CUDAToolkit_INCLUDE_DIRS}
)

# Add the RapidJSON directory as a subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/external/rapidjson)


# Add glad as a static library and ensure C language
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# Create ImGui static library
add_library(imgui STATIC
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
)

# Add source files for the executable
set(SOURCES
    main.cpp
    ${CMAKE_SOURCE_DIR}/gl_src/gl_Callbacks.cpp
    ${CMAKE_SOURCE_DIR}/gl_src/mainWindow.cpp
    

    ${CMAKE_SOURCE_DIR}/UI/betaWindow.cpp
    ${CMAKE_SOURCE_DIR}/UI/beta_helper.cpp
    ${CMAKE_SOURCE_DIR}/UI/Components/CheckPoints.cpp
    ${CMAKE_SOURCE_DIR}/UI/Components/gManipulation.cpp
    ${CMAKE_SOURCE_DIR}/UI/Components/plotFPS.cpp
    ${CMAKE_SOURCE_DIR}/UI/Components/ColorPicker.cpp

    ${CMAKE_SOURCE_DIR}/AuxiliaryLoopManager/Computational_Loop.cpp
    ${CMAKE_SOURCE_DIR}/AuxiliaryLoopManager/IMGUI_Loop.cpp

    ${CMAKE_SOURCE_DIR}/external/implot/implot.cpp
    ${CMAKE_SOURCE_DIR}/external/implot/implot_items.cpp
    
    ${CMAKE_SOURCE_DIR}/includes/utilities.cpp
)


# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wall -Wno-unused-parameter)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rapidjson/include
)
# Link libraries including glad and imgui
target_link_libraries(${PROJECT_NAME}
    glad
    imgui
    OpenGL::GL

    # glfw
    ${CMAKE_SOURCE_DIR}/external/GLFW/libglfw3.a
    CUDA::cudart
    ${CMAKE_DL_LIBS}
    m
    rt
    dl
)

if(bENABLE_CUDA)
    
    # Compile CUDA source
    add_library(cuda_src SHARED
        ${CMAKE_SOURCE_DIR}/cuda_src/libcuda_src.cu
    )

    # Add CUDA compilation flags
    set_target_properties(cuda_src PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        POSITION_INDEPENDENT_CODE ON
        CUDA_STANDARD 17
        CUDA_STANDARD_REQUIRED ON
    )

    if(WIN32)
        set_target_properties(cuda_src PROPERTIES
            LIBRARY_OUTPUT_NAME "libcuda_src")
    elseif(UNIX AND NOT APPLE)
        set_target_properties(cuda_src PROPERTIES
            LIBRARY_OUTPUT_NAME "cuda_src")
    endif()

    # Add the --extended-lambda flag for CUDA compilation
    target_compile_options(cuda_src PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:
    --extended-lambda
    -t0
    --use_fast_math
    >)
    target_include_directories(cuda_src PRIVATE ${CMAKE_SOURCE_DIR}/includes)

    target_link_libraries(${PROJECT_NAME} cuda_src)

    # Custom target for building CUDA library only
    add_custom_target(build_cuda_lib
        COMMAND +${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target cuda_src
        DEPENDS cuda_src
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building CUDA dynamic library only."
    )
endif()

# Custom target for cleaning
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/cuda_src
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
)

# Custom target for reloading (clean and build)
add_custom_target(reload
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
)
