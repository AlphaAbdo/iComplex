#pragma once 
#include <iostream>
#include "libcuda_src.h"

// The dimensions of the thread block
#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16
//
#if 1
template <class T>
__device__ __forceinline__ int CalcIndex(T* xPos, T* yPos, const int crunch) {
  T xTemp, yTemp, xSquared, ySquared, xStart, yStart;

  xStart = *xPos;
  yStart = *yPos;
  yTemp = 0;
  xTemp = 0;
  
  ySquared = yTemp * yTemp;
  xSquared = xTemp * xTemp;

  int iteration = 0;

  while (iteration++ < crunch && (xSquared + ySquared < T(4.0))) {
    yTemp = xTemp * yTemp * T(2.0) + yStart;
    xTemp = xSquared - ySquared + xStart;
    ySquared = yTemp * yTemp;
    xSquared = xTemp * xTemp;
  }
  *xPos = xTemp;
  *yPos = yTemp;
  return iteration;
}

#else
template <class T>
__device__ __forceinline__ int CalcIndex(T* xPos, T* yPos, const int crunch);


// template <class T>
// __device__ __forceinline__ int CalcIndex(T* xPos, T* yPos, const int crunch) {
//     const T ESCAPE_RADIUS = 1e7; // Radius beyond which we consider the trajectory unbounded
//     const int STABILITY_CHECK_START = 1000; // Start checking for stability after this many iterations
//     const int STABILITY_CHECK_INTERVAL = 100; // Check stability every this many iterations
//     const T STABILITY_THRESHOLD = 1e-6; // Threshold for detecting significant changes

//     T prev[2] = { *xPos, *yPos };
//     T current[2] = { 0, 0 };
//     T stability_check[2] = { 0, 0 };
//     int iteration = 0;
//     bool is_stable = true;

//     while (iteration++ < crunch) {
//         T pi_an = M_PI * prev[pX];
//         T expo_bn = exp(-M_PI * prev[pY]);
//         T tmp[2] = { cos(pi_an), sin(pi_an) };
//         current[pX] = (7*prev[pX] + 2 - expo_bn * (tmp[pX]*(5*prev[pX] + 2) - 5*tmp[pY]*prev[pY]) )/4;
//         current[pY] = (7*prev[pY]     - expo_bn * (tmp[pY]*(5*prev[pX] + 2) + 5*tmp[pX]*prev[pY])     )/4;

//         // Check for escape (unbounded trajectory)
//         if (current[pX]*current[pX] + current[pY]*current[pY] > ESCAPE_RADIUS*ESCAPE_RADIUS) {
//             break;
//         }

//         // Check for stability
//         if (iteration > STABILITY_CHECK_START && iteration % STABILITY_CHECK_INTERVAL == 0) {
//             if (fabs(current[pX] - stability_check[pX]) > STABILITY_THRESHOLD || 
//                 fabs(current[pY] - stability_check[pY]) > STABILITY_THRESHOLD) {
//                 is_stable = false;
//                 break;
//             }
//             stability_check[pX] = current[pX];
//             stability_check[pY] = current[pY];
//         }

//         prev[pX] = current[pX];
//         prev[pY] = current[pY];
//     }

//     *xPos = current[pX];
//     *yPos = current[pY];
//     return iteration;
// }



template <class T>
__device__ __forceinline__ int CalcIndex(T* xPos, T* yPos, const int crunch) {
  T prev[2] = { *xPos, *yPos };
  T current[2] = { 0, 0 };
  int iteration = 0;
  const long int X_threshold = 1e15;
  bool isBounded = true;
  while (iteration++ < crunch && isBounded) {
    T pi_an = M_PI * prev[pX];
    T expo_bn = exp(-(T) M_PI * prev[pY]);
    if(isnan(expo_bn) || isinf(expo_bn)) {
      iteration++;
      break;
    }
    T tmp[2] = { cos(pi_an), sin(pi_an) };
    T c2ond = expo_bn * (tmp[pX]*(5*prev[pX] + 2) - 5*tmp[pY]*prev[pY]);
    T c1ond = expo_bn * (tmp[pY]*(5*prev[pX] + 2) + 5*tmp[pX]*prev[pY]);
    // if(isnan(c1ond) || isnan(c2ond) || isinf(c1ond) || isinf(c2ond)) {
    //   break;
    // }
    current[pX] = (7*prev[pX] + 2 - c2ond)/4;
    current[pY] = (7*prev[pY]     - c1ond     )/4;

    isBounded = fabs(current[pX]) < T(X_threshold);
    isBounded |= (current[pY]) < T(X_threshold);
    // isBounded = sqrt(current[pX]*current[pX] + current[pY]*current[pY]) < X_threshold;
    // if(fabs(c1ond) < 0.001 && fabs(c2ond) < 0.001) {
    //   break;
    // }
    prev[pX] = current[pX];
    prev[pY] = current[pY];
  }
  *xPos = current[pX];
  *yPos = current[pY];
  return iteration;
}



// template <class T>
// __device__ __forceinline__ int CalcIndex(T* xPos, T* yPos, const int crunch) {
//   T prev[2] = { *xPos, *yPos };
//   T current[2] = { 0, 0 };

//   int convergence = 0;
//   T maxRadiusReached = -1;

//   int iteration = 0;
//   while (iteration++ < crunch) {
//     T pi_an = M_PI * prev[pX];
//     T expo_bn = __expf(-M_PI * prev[pY]);
//     T tmp[2] = { __cosf(pi_an), __sinf(pi_an) };
//     T c2ond = expo_bn * (tmp[pX]*(5*prev[pX] + 2) - 5*tmp[pY]*prev[pY]) ;
//     current[pX] = (7*prev[pX] + 2 - c2ond)/4;
//     T c1ond = expo_bn * (tmp[pY]*(5*prev[pX] + 2) + 5*tmp[pX]*prev[pY]);
//     current[pY] = (7*prev[pY]     - c1ond     )/4;

//     // if(iteration > 5)
//     { 
//       // T currentRadius = (current[pX]*current[pX] + current[pY]*current[pY]);
//       T currentRadius = fabs(current[pY]);
//       if(currentRadius > maxRadiusReached) {
//         maxRadiusReached = currentRadius;
//         convergence++;
//       }
//       // else if(currentRadius<0) printf("fucked up %f",currentRadius);
//     }
   


//     prev[pX] = current[pX];
//     prev[pY] = current[pY];
//   }
//   // if(maxRadiusReached == -1) printf("{%f ,%f}: %f\n",*xPos,*yPos,current[pX]);
//   *xPos = current[pX];
//   *yPos = current[pY];
//   return convergence;
// }




template <>
__device__ __forceinline__ int CalcIndex(double_float_r<float>* xPos, double_float_r<float>* yPos, const int crunch) {
  return 0;
}
#endif
