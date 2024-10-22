#include <thrust/device_vector.h>
#include <thrust/scan.h>
#include <thrust/copy.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/tuple.h>
#include <iostream>
#include <vector>
#include "libcuda_src.h"
#include "DS_.hpp"
#include "CalcIndex.cuh"
#include "kernels.cu"
#include "helper.cuh"
#include <memory>

template <fSTATE frame>
inline void handleFrameState(int*);

template<>
inline void handleFrameState<fSTATE::PERFECT_FORWARD>(int* dst);

template <>
inline void handleFrameState<fSTATE::FULL_COMPUTE>(int* dst) {
    using namespace contextVars;
    dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
        
    dim3 grid(iDivUp(d_depthArray->bufferSize.x  , BLOCKDIM_X), 
                iDivUp(d_depthArray->bufferSize.y, BLOCKDIM_Y));
    genericCaller<ImageFrameProcessor><<<grid, threads>>>(*pmode, dst, *d_depthArray, 
        *pcrunch, (*cOff)[pX], (*cOff)[pY], *pscale / (*SCR_SIZE)[pX]);
}

template <>
inline void handleFrameState<fSTATE::FORWARD_UPDATE>(int* dst) {
    using namespace contextVars;

    int size = SCR_SIZE->at(pX) * SCR_SIZE->at(pY);   
    static thrust::device_vector<int> d_compressedIndices;
    static int totalNonZero = 0;
    if (!AApass) {
        thrust::device_vector<int> d_mask(size);
        handleFrameState<fSTATE::PERFECT_FORWARD>(dst);

        dim3 threads1(BLOCKDIM_X, BLOCKDIM_Y);
        dim3 grid1(iDivUp((*SCR_SIZE)[pX], BLOCKDIM_X), iDivUp((*SCR_SIZE)[pY], BLOCKDIM_Y));
        genericCaller<MaskFrameProcessor><<<grid1, threads1>>>(*pmode, dst, *d_depthArray, 
            (*SCR_SIZE)[pX], (*SCR_SIZE)[pY], 0, 0, SCR_SIZE->at(pX), SCR_SIZE->at(pY), 
            thrust::raw_pointer_cast(d_mask.data()), *temppass);
        cudaDeviceSynchronize();

        // Perform prefix sum and count non-zero elements
        thrust::device_vector<int> d_scanned(size);
        thrust::exclusive_scan(d_mask.begin(), d_mask.end(), d_scanned.begin());
        totalNonZero = thrust::reduce(d_mask.begin(), d_mask.end());
        
        // Create compressed array of indices
        d_compressedIndices = thrust::device_vector<int>(totalNonZero);
        auto begin = thrust::make_zip_iterator(thrust::make_tuple(d_mask.begin(), d_scanned.begin(), thrust::counting_iterator<int>(0)));
        auto end = thrust::make_zip_iterator(thrust::make_tuple(d_mask.end(), d_scanned.end(), thrust::counting_iterator<int>(size)));
        thrust::copy_if(
            thrust::make_transform_iterator(begin, generate_indices()),
            thrust::make_transform_iterator(end, generate_indices()),
            d_compressedIndices.begin(),
            [] __device__ (int x) { return x != -1; }
        );
    }

    if (AApass < maxAApass) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, SAMPLE_SIZE - 1);
        int randomIndex = dist(rng) % SAMPLE_SIZE;

        int threadsPerBlock = 128;
        int blocksPerGrid = (totalNonZero + threadsPerBlock - 1) / threadsPerBlock;

        cudaStreamSynchronize(contextVars::preLoadingStream);
        genericCaller<processPixels><<<blocksPerGrid, threadsPerBlock, threadsPerBlock * sizeof(int), preLoadingStream>>> (
            *pmode, thrust::raw_pointer_cast(d_compressedIndices.data()), totalNonZero,
            *d_depthArray, (*SCR_SIZE)[pX], (*SCR_SIZE)[pY], 0, 0, *pcrunch, (*cOff)[pX], (*cOff)[pY], 
            *pscale / (*SCR_SIZE)[pX], *d_depthArraySwap, 
            AApass, AApass + 1 == maxAApass, randomIndex
        );
        
        AApass++;
        if(AApass == maxAApass) cudaStreamSynchronize(contextVars::preLoadingStream);
    }

    else if (AApass == maxAApass) {
        
        handleFrameState<fSTATE::PERFECT_FORWARD>(dst);
        AApass++;
        cudaDeviceSynchronize();
    }
}

template <>
inline void handleFrameState<fSTATE::FRAME_SHIFT>(int* dst) {
    using namespace contextVars;
    double s = *pscale / (*SCR_SIZE)[pX];
    frameToframe_Offset[pX] += (-(*cOff)[pX] + uniqueprevcOff[pX]) / s;
    frameToframe_Offset[pY] += (-(*cOff)[pY] + uniqueprevcOff[pY]) / s;

    dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
    dim3 grid(iDivUp((*SCR_SIZE)[pX] - std::abs(int(frameToframe_Offset[pX])), BLOCKDIM_X), 
                iDivUp((*SCR_SIZE)[pY] - std::abs(int(frameToframe_Offset[pY])), BLOCKDIM_Y));
    
    if (grid.x && grid.y) {
        genericCaller<SwapDataBuffers><<<grid, threads>>>(
            *pmode, dst, *d_depthArray, *d_depthArraySwap, (*SCR_SIZE)[pX], (*SCR_SIZE)[pY], 
            frameToframe_Offset[pX], frameToframe_Offset[pY]
        );
        
        frameToframe_Offset[pX] = LinearAdder(frameToframe_Offset[pX]);
        frameToframe_Offset[pY] = LinearAdder(frameToframe_Offset[pY]);
        unsigned int threado = 1024;
        unsigned int len1 = std::abs(int(frameToframe_Offset[pY])) * (*SCR_SIZE)[pX];
        unsigned int len2 = std::abs(int(frameToframe_Offset[pX])) * std::max(0u, (*SCR_SIZE)[pY] - std::abs(int(frameToframe_Offset[pY])));
        unsigned int blocks = iDivUp(len1 + len2, threado);

        if (blocks > 0) {
            genericCaller<PartialFrameProcessor><<<blocks, threado>>>(
                *pmode, dst, *d_depthArraySwap, (*SCR_SIZE)[pX], (*SCR_SIZE)[pY], 
                frameToframe_Offset[pX], frameToframe_Offset[pY], len1, len2, 
                *pcrunch, (*cOff)[pX], (*cOff)[pY], s
            );
        }

        std::swap(d_depthArray, d_depthArraySwap);

        frameToframe_Offset[pX] -= int(frameToframe_Offset[pX]);
        frameToframe_Offset[pY] -= int(frameToframe_Offset[pY]);

        cudaDeviceSynchronize();
    }
    else throw std::runtime_error("Invalid frame shift occured {out of scope}:soft reset performed");

}

template <>
inline void handleFrameState<fSTATE::ZOOM_PRELOAD>(int* dst) {
    using namespace contextVars;
    float newdscale = *pscale / oldscale;
    if (dscale != newdscale) {
        preLoaded = false;
    }
    dscale = newdscale;

    if (!preLoaded) {
        dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
        dim3 grid(iDivUp((*SCR_SIZE)[pX], BLOCKDIM_X), 
                    iDivUp((*SCR_SIZE)[pY], BLOCKDIM_Y));
        genericCaller<ImageFrameProcessor><<<grid, threads>>>(
            *pmode, dst, *d_depthArray,
            *pcrunch, cOff->at(pX), cOff->at(pY), *pscale / (*SCR_SIZE)[pX]
        );
        preLoaded = true;
    }
    else {
        double cOffpX = (*cOff)[pX] + ((*cOff)[pX] - uniqueprevcOff[pX]) * dscale;
        double cOffpY = (*cOff)[pY] + ((*cOff)[pY] - uniqueprevcOff[pY]) * dscale;
        dim3 threads1(BLOCKDIM_X, BLOCKDIM_Y);
        cudaStreamSynchronize(contextVars::preLoadingStream);
        dim3 grid1(iDivUp((*SCR_SIZE)[pX], BLOCKDIM_X), 
                    iDivUp((*SCR_SIZE)[pY], BLOCKDIM_Y));
        genericCaller<ImageFrameProcessor><<<grid1, threads1, 0, preLoadingStream>>>(
            *pmode, *d_depthArray,
            *pcrunch, cOffpX, cOffpY, (*pscale / (*SCR_SIZE)[pX]) * dscale
        );
        *frameState = fSTATE::FORWARD_UPDATE;

        std::swap(d_depthArray, d_depthArraySwap);

        dim3 threads(1024);
        dim3 grid(iDivUp((*SCR_SIZE)[pX] * (*SCR_SIZE)[pY], 1024));
        genericCaller<ForwardUpdating><<<grid, threads>>>(
            *pmode, dst, *d_depthArray);
        cudaDeviceSynchronize();
    }
}

template<>
inline void handleFrameState<fSTATE::PERFECT_FORWARD>(int* dst) {
    using namespace contextVars;
    dim3 threads(1024);
    dim3 grid(iDivUp((*SCR_SIZE)[pX] * (*SCR_SIZE)[pY], 1024));
    genericCaller<ForwardUpdating><<<grid, threads>>>(*pmode, dst, *d_depthArray);
}