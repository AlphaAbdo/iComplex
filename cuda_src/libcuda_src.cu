#include <thrust/device_vector.h>
#include <thrust/scan.h>
#include <thrust/copy.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/tuple.h>
#include <iostream>
#include <vector>
#include <algorithm> // uses std::min
#include "libcuda_src.h"
#include "DS_.hpp"
#include "CalcIndex.cuh"

#include "kernels.cu"
#include <memory>
#include "frameState.cu"
#include "helper.cuh"
// The Mandelbrot CUDA GPU thread function

extern "C" void InitializeCUDA( std::vector<unsigned int> *SCR_SIZE  , std::vector<double> *cOff,
                                int* pcrunch,double* pscale, fSTATE* frameState, int* pmode , int *temppass){

    contextVars::SCR_SIZE = SCR_SIZE;
    contextVars::cOff = cOff;
    contextVars::temppass = temppass;
    contextVars::pcrunch = pcrunch;
    contextVars::pscale = pscale;
    contextVars::frameState = frameState;
    *contextVars::frameState = fSTATE::FULL_COMPUTE;
    //to be set later to float for context switching.
    contextVars::pmode = pmode;
    // Perform any other initialization if needed

    //Allocating memory for depthArrays

    contextVars::d_depthArray = new checkPointData((*SCR_SIZE)[pX] , (*SCR_SIZE)[pY]);
    contextVars::d_depthArraySwap = new checkPointData((*SCR_SIZE)[pX] , (*SCR_SIZE)[pY]);

    // Create streams
    cudaStreamCreate(&contextVars::preLoadingStream);
    // cudaStreamCreate(&contextVars::stream2);
}

extern "C" void CleanupCUDA() {
    printf("Cleaning up\n");
    // Free any resources allocated in InitializeCUDA
    using namespace contextVars;
    cudaDeviceSynchronize();
    if(d_depthArray){
        d_depthArray->freeMemory(true);
        delete d_depthArray;
        d_depthArray = nullptr;
    }
    if(d_depthArraySwap){
        d_depthArraySwap->freeMemory(true);
        delete d_depthArraySwap;
        d_depthArraySwap = nullptr;
    }
    // Destroy streams
    cudaStreamDestroy(preLoadingStream);
}

// double frameToframe_Offset[2] = {0,0};
void PreAllocateMemory(std::vector<unsigned int> new_SCR_SIZE, std::vector<int> bufferOffset, int SoftRender){
    using namespace contextVars;

    if(SoftRender){

        d_depthArray->nextBuffer = new checkPointData(new_SCR_SIZE[pX], new_SCR_SIZE[pY]);

        d_depthArraySwap->nextBuffer = new checkPointData(new_SCR_SIZE[pX], new_SCR_SIZE[pY]);

        //calling OutwardForwardUpdating
        dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
        dim3 grid(iDivUp(new_SCR_SIZE[pX], BLOCKDIM_X), 
                    iDivUp(new_SCR_SIZE[pY], BLOCKDIM_Y));
        genericCaller<OutwardForwardUpdating><<<grid, threads,0 ,preLoadingStream>>>
                (*pmode, *d_depthArray, *(d_depthArray->nextBuffer),
                make_int2(bufferOffset[pX], bufferOffset[pY]),
                *pcrunch, (*cOff)[pX], (*cOff)[pY], *pscale / new_SCR_SIZE[pX]);
    }
    else{
        if(d_depthArray) {
            d_depthArray->freeMemory();
            d_depthArray = nullptr;
        }
        
        if(d_depthArraySwap) {
            d_depthArraySwap->freeMemory();
            d_depthArraySwap = nullptr;
        }
    }
    
}
#include <chrono>
void ExecuteCudaFrameProcessor(int *dst, fSTATE constantframe) {
    using namespace contextVars;

    if(!d_depthArray){
        d_depthArray = new checkPointData((*SCR_SIZE)[pX] , (*SCR_SIZE)[pY]);
    }
    if(!d_depthArraySwap){
        d_depthArraySwap = new checkPointData((*SCR_SIZE)[pX] , (*SCR_SIZE)[pY]);
    }
    
    const fSTATE curren_frameState = (constantframe != fSTATE::UNKNOWN_STATE) ? constantframe : *frameState; 
    // Calculate scale and offsets
    double s = *pscale / (*SCR_SIZE)[pX];
    double x = (*cOff)[pX];
    double y = (*cOff)[pY];

    if (constantframe  == fSTATE::UNKNOWN_STATE) {
        if (curren_frameState != fSTATE::FORWARD_UPDATE) {
            AApass = 0;
        }
        if (curren_frameState != fSTATE::FORWARD_UPDATE && curren_frameState != fSTATE::ZOOM_PRELOAD) {
            cudaStreamSynchronize(contextVars::preLoadingStream);
            if (preLoaded) {
                preLoaded = false;
            }
        }
    }
    if(constantframe == fSTATE::UNKNOWN_STATE && d_depthArray->nextBuffer)
    {
        // *frameState = 0;
        cudaStreamSynchronize(preLoadingStream);
        getLastCudaError("Resize Entrance Failure Accured\n");

        //internal swap
        auto d_NextdepthArray = d_depthArray->nextBuffer;
        auto d_NextdepthArraySwap = d_depthArraySwap->nextBuffer;

        //delete nextBuffer
        d_depthArray->freeMemory(false);
        delete d_depthArray;
        d_depthArray = d_NextdepthArray;

        d_depthArraySwap->freeMemory(false);
        delete d_depthArraySwap;
        d_depthArraySwap = d_NextdepthArraySwap;

        if(*frameState == fSTATE::FULL_COMPUTE){
            *frameState = fSTATE::FORWARD_UPDATE;
            dim3 threads(1024);
            dim3 grid(iDivUp((d_depthArray->bufferSize.x) * (d_depthArray->bufferSize.y), 1024));
            genericCaller<ForwardUpdating><<<grid, threads>>>(*pmode, dst, *d_depthArray);
        }
        else {
            handleFrameState<fSTATE::FULL_COMPUTE>(dst);
            *frameState = fSTATE::FORWARD_UPDATE;
            cudaDeviceSynchronize();
        }
        AApass = 0;
        
    }
    else if (curren_frameState == fSTATE::FULL_COMPUTE) {
        handleFrameState<fSTATE::FULL_COMPUTE>(dst);
        *frameState = fSTATE::FORWARD_UPDATE;
        cudaDeviceSynchronize();
    }
    else if (curren_frameState == fSTATE::FORWARD_UPDATE) {
        handleFrameState<fSTATE::FORWARD_UPDATE>(dst);
    }
    
    else if (curren_frameState == fSTATE::FRAME_SHIFT) {
        try{
            handleFrameState<fSTATE::FRAME_SHIFT>(dst);
        }
        catch(const std::exception& e){
            handleFrameState<fSTATE::FULL_COMPUTE>(dst);
            cudaDeviceSynchronize();
        }
        *frameState = fSTATE::FORWARD_UPDATE;
        
    }
    else if (curren_frameState == fSTATE::ZOOM_PRELOAD) {
        handleFrameState<fSTATE::ZOOM_PRELOAD>(dst);
        *frameState = fSTATE::FORWARD_UPDATE;
    }




    if (curren_frameState != fSTATE::FORWARD_UPDATE) {
        getLastCudaError("Mandelbrot0 kernel execution failed.\n On State: " + (int)(curren_frameState));
    }
    
    if (constantframe == fSTATE::UNKNOWN_STATE) {
        uniqueprevcOff[pX] = (*cOff)[pX];
        uniqueprevcOff[pY] = (*cOff)[pY];
        oldscale = *pscale;
    }

}
