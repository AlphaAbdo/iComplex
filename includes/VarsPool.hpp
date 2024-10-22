#pragma once
#include <macros.hpp>
#include <vector>


#ifdef __CUDACC__s
inline bool CudaRender = true;
#endif

inline double avgLoopTime = 0.0;
inline double realLoopTime = 0.0;

inline int crunch = 512;
inline double scale = 3.2;
inline std::vector<unsigned int> SCR_SIZE = {1500, 900};
inline int SCRupscale = 1;



inline std::vector<double> cOff = {-0.5 - 0.5 * scale, -0.5 * SCR_SIZE[pY] * scale / SCR_SIZE[pX]};
inline std::vector<double> mOff = {0.0, 0.0};
inline std::vector<double> pimOff = {0.0, 0.0};
inline double dscale = 1.0;

inline int precisionMode = 0;

inline double animationFrame = 0;
inline int animationStep = 0;

inline fSTATE frameState = fSTATE::FULL_COMPUTE;

// inline Color colors(1, 0, 0, 0);
inline float color_factor[] = {1.0, 0.0, 0.0, 0.0};
inline float color_scaler = 20.0;
inline int numSMs = 0;
inline int version = 1;

inline bool ScreenSizeChanged = false;
inline std::vector<unsigned int> new_SCR_SIZE = SCR_SIZE;
inline std::vector<int> bufferOffset{0, 0};
inline int SCR_POS[2] = {0, 0};


inline bool showStripsOverlay = true;
inline bool isBackgroundAnchored = true;


