# iComplex
A complex-plane interface for visualizing fractals, with a focus on high performance and usability.
# Description
This project is a **high-performance** real-time fractal visualization tool that allows you to explore the complex plane with unprecedented speed and flexibility. Designed with a focus on efficiency, it leverages parallel computation using CUDA and OpenGL for smooth and responsive rendering. The tool is highly modular, enabling dynamic loading of CUDA kernels and a seamless, interactive interface powered by IMGUI. Its architecture is optimized to minimize frame time, making it suitable for both casual exploration and in-depth fractal research.

# Features

## Core Capabilities
1. Real-Time Fractal Visualization

Dynamic exploration of fractals within the complex plane.
Highly responsive interface with smooth zooming and panning for a detailed view of fractals.

2. High-Performance Rendering with OpenGL

Uses OpenGL shaders for efficient rendering.
Shader Storage Buffer Objects (SSBOs) are employed for fast data exchange between shaders, enabling real-time updates without significant computational overhead.
Dynamic Color Management: Colors are determined and applied directly in the OpenGL pipeline for maximum performance.

3. CUDA-Accelerated Computation (Optional, NVIDIA-Enabled CUDA Devices)

Provides CUDA support for high-performance fractal calculations, significantly improving computation time on NVIDIA CUDA-enabled devices.
Modular CUDA Kernel System: CUDA kernels can be dynamically loaded and unloaded at runtime, allowing for flexible updates without recompiling the entire application.
CUDA is optional and falls back to a standard computational path if CUDA is not available, ensuring compatibility on all hardware.

## Parallelization & Architecture
4. Parallelized Loops for Smooth Performance

Implements three core loops (Main Loop, Computational Loop, and IMGUI Loop) that run concurrently to ensure minimal latency and maximum performance.
This architecture ensures that heavy computational tasks do not interfere with UI responsiveness or frame rendering.

5. Double Pillow Optimization Technique

The Double Pillow method is used to synchronize the three parallel loops (main, computational, and IMGUI), ensuring they fill each other's gaps when stalled or delayed.
In the worst-case scenario (extreme computational load), other tasks like UI rendering and environment updates still happen concurrently, optimizing system resources.

6. IMGUI-Based Lightweight User Interface

IMGUI is employed to provide a responsive, real-time user interface with minimal impact on frame rate.
Optimized to run with negligible overhead—especially important for high frame rate scenarios (approaching 1ms per frame).
The IMGUI loop runs in parallel with other loops, ensuring UI responsiveness even under heavy load.

## Modularity & Flexibility
7. Dynamic System with Modular Components

Modular CUDA and OpenGL components: Both the CUDA and GLSL pipelines are highly modular, allowing for easy expansion or replacement of individual components without reworking the entire system.
The system handles dynamic reloading of CUDA kernels at runtime without stalling the main program or OpenGL rendering.

8. Dynamic Runtime Reloading

Shader Hot-Reloading: All blur shaders and the DistDepGenShader can be reloaded at runtime, allowing instant application of modifications without program restart.
CUDA Module Live Updates: The CUDA module (libcuda_src) can be compiled and reloaded during execution, seamlessly integrating changes with minimal disruption:

- Reloading process only skips frames, avoiding system stalls.
- New module is plugged in automatically upon successful compilation.

This feature significantly enhances development efficiency and allows for rapid prototyping of visual and computational elements.

9. Optional CUDA & Compute Shader Integration

NVIDIA CUDA-enabled GPUs benefit from enhanced performance by offloading calculations to the GPU through a sequence of kernel launches (explained later), but the application works without CUDA, using standard computational methods if the device lacks CUDA support.

## Additional Features
10. Frame Rate Stability (Optimized for Low-Budget GPUs)

Frame rates maintained above 700 FPS on low-budget GPUs (e.g., GTX 1650) with default configuration, ensuring smooth visuals.
The system is optimized to minimize GPU and CPU overhead, ensuring consistent performance even when approaching the 1ms per frame threshold.
The parallelization architecture ensures that when CUDA or computational tasks are heavily loaded, IMGUI and other non-computational tasks remain fluid.

11. Efficient Resource Management

Optimized to minimize the transfer overhead between different GPU resources. For instance, color management and mapping are performed in a unified pipeline, reducing the need for separate operations between CUDA and OpenGL components.
Low overhead UI rendering, especially when computational loops are idle or under high load.

12. Platform Compatibility

Cross-platform support for a wide range of systems that supports opengl 4.3+, with seamless fallback to non-CUDA execution paths on non-NVIDIA devices.

# Installation

Follow these steps to clone, build, and run the project. Make sure to set up all required dependencies.

## Prerequisites

You will need the following installed:

- **[CMake](https://cmake.org/)** (version 3.10 or higher)
- **[OpenGL](https://www.opengl.org/)** (for rendering)
- **[GLFW 3.4+](https://www.glfw.org/)** (for window management and input handling)
- **[CUDA Toolkit](https://developer.nvidia.com/cuda-toolkit)** (if enabling CUDA support)
- A **C++17** compatible compiler

> **Note**: You can either install GLFW using your system's package manager, or compile and link it as a static `.a` library (as done in this project). For more information, visit the [GLFW website](https://www.glfw.org/).

## Clone the repository

To get started, clone the repository and navigate to the project directory:

```bash
git clone https://github.com/AlphaAbdo/iComplex.git
cd iComplex
```

## Building the project

### Step 1: Create a build directory

```bash
mkdir build
```

### Step 2: Configure the project

Use `CMake` to configure the project:

```bash
cmake -S . -B build
```

### Step 3: Build the project

Now build the project:

```bash
cmake --build ./build --parallel 4
```

### Platform-specific instructions

#### Linux

Ensure you have the following dependencies:

```bash
sudo apt-get install build-essential cmake libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev
```

- **Optional**: Install [CUDA](https://developer.nvidia.com/cuda-toolkit) if you want to enable GPU acceleration.

To configure and build the project on Linux with CUDA enabled:

```bash
mkdir build
cmake -S . -B build -DENABLE_CUDA=ON
cmake --build ./build --parallel 4
```

#### Windows

For Windows, ensure you have the following:

- **Visual Studio** (with CMake integration)
- **CUDA Toolkit** (if using CUDA)

Open a terminal with Visual Studio environment variables and run:

```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build .
```

If you don't want to enable CUDA, configure with the flag:

```bash
cmake -S . -B build -DENABLE_CUDA=OFF
```
> **Note**: Due to CMAKE caching mechanism ,I highly recommend to to use -DENABLE_CUDA=ON or -DENABLE_CUDA=OFF *ONLY* on cuda enabled systems

## Running the project

Once the build is complete, you can run the executable:

```bash
./iComplex  # on Linux
iComplex.exe  # on Windows
```
> **Warning**: For systems with multiple GPUs (e.g., an iGPU from AMD and a discrete NVIDIA GPU), it's important to ensure that the OpenGL context matches the GPU that supports CUDA. By the nature of the project, there is no inter-GPU data transfer. The same SSBO object (and mapped memory address) is periodically and concurrently used by both OpenGL and CUDA without data being transferred between different GPUs.
>
> **Windows:** The program can be manually forced to run on the CUDA-enabled GPU.
>
> **Linux:** You can use the following command to force the program to run on the NVIDIA GPU:
>
```bash
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./iComplex  # on Linux
```

> By **default**, if the program is compiled with CUDA support but CUDA is inaccessible, it will automatically fall back to using compute shaders.

## Custom Build Targets

- **Build only the CUDA library**:
  
  ```bash
  cmake --build ./build --target build_cuda_lib
  ```

- **Clean and rebuild the project**:
  
  ```bash
  cmake --build ./build --target reload
  ```
