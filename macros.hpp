#pragma once

//debug headers
#define __CUDACC__s
#ifdef DISABLE_CUDA

#undef __CUDACC__s

#endif


#define __USE_IMGUI

#define pX 0
#define pY 1

// Frame states
enum class fSTATE {
    UNKNOWN_STATE = -1,
    FULL_COMPUTE,
    FORWARD_UPDATE,
    FRAME_SHIFT,
    ZOOM_PRELOAD,
    PERFECT_FORWARD
};

#define NEGATIVE_OFFSET -5


