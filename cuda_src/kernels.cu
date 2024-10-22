#pragma once
#include <iostream>
#include "libcuda_src.h"
#include "CalcIndex.cuh"
#include "DS_.hpp"
// #include "kernels.cuh"


// Templated wrapper struct with templated call function
struct ImageFrameProcessor {
    template <typename T>
    __device__ static void selfCall(int *dst, checkPointData d_depthArray,
                                        const int crunch, const T xOff, const T yOff, const T scale) {
        // Calculate thread indices
        int ix = blockDim.x * blockIdx.x + threadIdx.x;
        int iy = blockDim.y * blockIdx.y + threadIdx.y;
        
        if ((ix < d_depthArray.bufferSize.x) && (iy < d_depthArray.bufferSize.y)) {

            int pixel = d_depthArray.bufferSize.x * iy + ix;
            
            // Calculate the location
            T xPos = (T)ix * scale + xOff;
            T yPos = (T)iy * scale + yOff;

            // Calculate the Mandelbrot index for the current location
            int m = CalcIndex<T>(&xPos, &yPos, crunch) -1;
            
            // d_depthArray.x[pixel]   = xPos;
            // d_depthArray.y[pixel]   = yPos;
            d_depthArray.depth[pixel] = m;
            if(dst != nullptr) dst[pixel] = m;
        }
    }
    
    template <typename T>
    __device__ static void selfCall(checkPointData d_depthArray,
                                        const int crunch, const T xOff, const T yOff,
                                        const T scale){
        selfCall<T>(nullptr, d_depthArray, crunch, xOff, yOff, scale);
    }
};

struct MaskFrameProcessor {
    template <typename T>
    __device__ static void selfCall(int *dst, const checkPointData d_depthArray,
                                        const int SCR_WIDTH, const int SCR_HEIGHT,
                                    const int SCR_WIDTH_Offset, const int SCR_HEIGHT_Offset,
                                    const int subSCR_WIDTH, const int subSCR_HEIGHT , int* mask, int temppass) {
        int offsetX = BLOCKDIM_X * blockIdx.x + SCR_WIDTH_Offset;
        int offsetY = BLOCKDIM_Y * blockIdx.y + SCR_HEIGHT_Offset;
        
        if((offsetX > ((SCR_WIDTH_Offset>=0)? SCR_WIDTH_Offset : 0) -BLOCKDIM_X) && (offsetX < SCR_WIDTH) && (offsetX < subSCR_WIDTH) &&
           (offsetY > ((SCR_HEIGHT_Offset>=0)? SCR_HEIGHT_Offset :0) -BLOCKDIM_Y) && (offsetY < SCR_HEIGHT) && (offsetY < subSCR_HEIGHT))
        {
            int tx = threadIdx.x;
            int ty = threadIdx.y;
            
            __shared__ int DepthData[(BLOCKDIM_Y+2)][(BLOCKDIM_X+2)];

                
            int id = threadIdx.y * blockDim.x + threadIdx.x;
            for(; id < (BLOCKDIM_X+2)*(BLOCKDIM_Y+2); id += blockDim.x * blockDim.y){
                int idx = id%(BLOCKDIM_X+2);
                int idy = id/(BLOCKDIM_X+2);

                bool withingBuffer = (offsetY +idy > 0) && (offsetY +idy < SCR_HEIGHT+1) && (offsetX + idx > 0) && (offsetX + idx < SCR_WIDTH+1);
                DepthData[idy][idx] = 
                    ((withingBuffer)? (d_depthArray.depth[SCR_WIDTH * (offsetY + idy - 1) + offsetX + idx - 1]) : -1);
            
            }
            __syncthreads();


            int ix = offsetX + tx;
            int iy = offsetY + ty;
            
            if ((ix>= 0 && ix < min(SCR_WIDTH, subSCR_WIDTH)) && (iy>= 0 && iy < min(SCR_HEIGHT, subSCR_HEIGHT))) {    
                int pixel = SCR_WIDTH * iy + ix;
                int depth = DepthData[ty +1][tx +1];

                if(temppass)
                {
                    
                if(depth == -1) depth = -1;
                else if (DepthData[ty][tx] != depth) depth = -1;
                else if (DepthData[ty][tx+2] != depth) depth = -1;
                else if (DepthData[ty+2][tx] != depth) depth = -1;
                else if (DepthData[ty+2][tx+2] != depth) depth = -1;
                else if (DepthData[ty][tx+1] != depth) depth = -1;
                else if (DepthData[ty+1][tx] != depth) depth = -1;
                else if (DepthData[ty+1][tx+2] != depth) depth = -1;
                else if (DepthData[ty+2][tx+1] != depth) depth = -1;
                }
                else{
                if(depth == -1) depth = -1;
                else 
                {
                    int score = 0;
                    if (DepthData[ty][tx] != depth) score++;
                    if (DepthData[ty][tx+2] != depth) score++;
                    if (DepthData[ty+2][tx] != depth) score++;
                    if (DepthData[ty+2][tx+2] != depth) score++;
                    if (DepthData[ty][tx+1] != depth) score++;
                    if (DepthData[ty+1][tx] != depth) score++;
                    if (DepthData[ty+1][tx+2] != depth) score++;
                    if (DepthData[ty+2][tx+1] != depth) score++;
                    if(score > 4) depth = -1;
                }
                }
                mask[pixel] = (depth==-1);
            }
        }
    }
};

struct processPixels {
    template <typename T>
    __device__ static void selfCall(int* compressedIndices, int numCompressed,
                                        checkPointData d_depthArray,
                                        const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT,
                                        const int SCR_WIDTH_Offset, const int SCR_HEIGHT_Offset,
                                        const int crunch, const T xOff, const T yOff,
                                        const T scale,
                                        checkPointData d_depthArraySwap, int AApass, bool terminalpass, int randomIndex) {
        short tx = threadIdx.x;
        int idx = blockIdx.x * blockDim.x + tx;
        extern __shared__ int sharedMem[];
        if (idx < numCompressed) {
            sharedMem[tx] = (AApass)? d_depthArraySwap.depth[idx] : 0;
            __syncthreads();
            int pixelIndex = compressedIndices[idx];
            short ix = pixelIndex % SCR_WIDTH;
            short iy = pixelIndex / SCR_WIDTH;

            int pixel = SCR_WIDTH * iy + ix;
            
            T xPos = (T)(ix) * scale + (T)(SAMPLE_ARRAY[randomIndex][pX]) * scale + xOff;
            T yPos = (T)(iy) * scale + (T)(SAMPLE_ARRAY[randomIndex][pY]) * scale + yOff;

            int m = CalcIndex<T>(&xPos, &yPos, crunch) -1 + sharedMem[tx];
            if(!terminalpass) 
            {
                __syncthreads();
                d_depthArraySwap.depth[idx] = m;
            }
            if(terminalpass){
                m /= (AApass+1);
                __syncthreads();
                d_depthArray.depth[pixel] = m;
                

            } 
        }
    }
};

template <typename T>
__device__ inline T offseteR(T& depth)
{
    return (depth < NEGATIVE_OFFSET)? -depth + NEGATIVE_OFFSET : depth;
}


struct ForwardUpdating {
    template <typename T>
    __device__ static void selfCall(int *dst, checkPointData d_depthArray) {
        
        short localID = threadIdx.x;
        const int iX = blockDim.x * blockIdx.x + localID;

        __shared__ int prevDepth[1024];

        if (iX < d_depthArray.bufferSize.x * d_depthArray.bufferSize.y) {
            prevDepth[localID]= d_depthArray.depth[iX];


            dst[iX] = prevDepth[localID];
        }
    }

};

struct OutwardForwardUpdating {
    template <typename T>
    __device__ static void selfCall(const checkPointData d_depthArray, checkPointData d_NextdepthArray,
                                        const int2 bufferSize,
                                        const int crunch, const T xOff, const T yOff,
                                        const T scale) {
                                            
        int ix = blockDim.x * blockIdx.x + threadIdx.x;
        int iy = blockDim.y * blockIdx.y + threadIdx.y;
        
        int m = -1;
        if ((ix + bufferSize.x < d_depthArray.bufferSize.x) && (iy + bufferSize.y < d_depthArray.bufferSize.y) &&
            (ix + bufferSize.x >= 0) && (iy + bufferSize.y >= max(0, d_NextdepthArray.bufferSize.y - d_depthArray.bufferSize.y))
            ) {
            __shared__ int prevDepth[BLOCKDIM_X * BLOCKDIM_Y];
            int tmp =  - (d_NextdepthArray.bufferSize.y - d_depthArray.bufferSize.y);
            int localID = threadIdx.y * blockDim.x + threadIdx.x;
            prevDepth[localID] = d_depthArray.depth[d_depthArray.bufferSize.x * (iy + bufferSize.y + tmp) + ix + bufferSize.x];
            __syncthreads();
            m = prevDepth[localID];
        }
        else if(ix < d_NextdepthArray.bufferSize.x && iy < d_NextdepthArray.bufferSize.y) {
            
            T xPos = (T)ix * scale + xOff;
            T yPos = (T)iy * scale + yOff;

            m = CalcIndex<T>(&xPos, &yPos, crunch) -1;
            
            
        }
        
        if(ix < d_NextdepthArray.bufferSize.x && iy < d_NextdepthArray.bufferSize.y) {
            int pixel = d_NextdepthArray.bufferSize.x * iy + ix;
            d_NextdepthArray.depth[pixel] = m;
        }
    }
};
struct SwapDataBuffers {
    template <typename T>
    __device__ static void selfCall(int *dst, checkPointData d_depthArray, checkPointData d_depthArraySwap,
                                    const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT,
                                    const int SCR_WIDTH_Offset, const int SCR_HEIGHT_Offset) {
        const int idx = blockDim.x * blockIdx.x + threadIdx.x;
        const int idy = blockDim.y * blockIdx.y + threadIdx.y;

        const int ix = max(0, SCR_WIDTH_Offset)  + idx;
        const int iy = max(0, SCR_HEIGHT_Offset) + idy;

        __shared__ int prevDepth[BLOCKDIM_X * BLOCKDIM_Y];

        if ((ix < SCR_WIDTH + min(0, SCR_WIDTH_Offset)) && (iy < SCR_HEIGHT + min(0, SCR_HEIGHT_Offset))) {

            int offsetPixel = SCR_WIDTH * (iy - SCR_HEIGHT_Offset) + (ix - SCR_WIDTH_Offset);

            int localID = threadIdx.y * blockDim.x + threadIdx.x;
            prevDepth[localID] = d_depthArray.depth[offsetPixel];

            __syncthreads();

            int pixel = SCR_WIDTH * iy + ix;

            d_depthArraySwap.depth[pixel] = prevDepth[localID];
            dst[pixel] = d_depthArraySwap.depth[pixel];
        }
    }
};

struct PartialFrameProcessor {
    template <typename T>
    __device__ static void selfCall(int *dst, checkPointData d_depthArray,
                                        const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT,
                                        const int SCR_WIDTH_Offset, const int SCR_HEIGHT_Offset,
                                        const unsigned int len1, const unsigned int len2,

                                        const int crunch, const T xOff, const T yOff,
                                        const T scale) {
       
        unsigned short idx = 0;
        unsigned short idy = 0;
        unsigned int id = threadIdx.x + blockDim.x * blockIdx.x;

        if(id < len1){
            idx = id / abs(SCR_HEIGHT_Offset) + 0;
            idy = id % abs(SCR_HEIGHT_Offset) + ((SCR_HEIGHT_Offset<0)? (SCR_HEIGHT + SCR_HEIGHT_Offset) : 0);
        }
        else if(id < len1 + len2 && SCR_WIDTH_Offset){
            idx = (id-len1) % abs(SCR_WIDTH_Offset) + ((SCR_WIDTH_Offset<0)? (SCR_WIDTH + SCR_WIDTH_Offset) : 0);
            idy = (id-len1) / abs(SCR_WIDTH_Offset) + ((SCR_HEIGHT_Offset<0)? 0 : SCR_HEIGHT_Offset);
        }
        if(id < len1 + len2){
            int pixel = SCR_WIDTH * idy + idx;

            T xPos = (T)idx * scale + xOff;
            T yPos = (T)idy * scale + yOff;

            int m = CalcIndex<T>(&xPos, &yPos, crunch) -1;
            
            d_depthArray.depth[pixel] = m;
            dst[pixel] = m;

        }
        if (idx >= SCR_WIDTH || idy >= SCR_HEIGHT)
        {
            printf("%u:\tidx: %d, idy: %d, len1: %d, len2: %d, SCR_WIDTH_Offset: %d, SCR_HEIGHT_Offset: %d\n",id, idx, idy, len1, len2, SCR_WIDTH_Offset, SCR_HEIGHT_Offset);
        }
    }
};