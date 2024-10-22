#pragma once

#include <vector_types.h>
#include "macros.hpp"


double frameToframe_Offset[2] = {0, 0};
double uniqueprevcOff[] = {0.0, 0.0};

struct checkPointData {
    double *x = nullptr;
    double *y = nullptr;
    int *depth = nullptr;
    int2 bufferSize;
    checkPointData *nextBuffer;

    // Host constructor
    __host__ checkPointData(int2 newSize) : bufferSize(newSize), x(nullptr), y(nullptr), depth(nullptr), nextBuffer(nullptr) {
        cudaMalloc(&x, bufferSize.x * bufferSize.y * sizeof(double));
        cudaMalloc(&y, bufferSize.x * bufferSize.y * sizeof(double));
        cudaMalloc(&depth, newSize.x * newSize.y * sizeof(int));
    }

    // Host constructor with separate dimensions
    __host__ checkPointData(int newSizeX, int newSizeY) 
        : checkPointData(make_int2(newSizeX, newSizeY)) {}


    // Host-only method to free memory
    __host__ void freeMemory(bool totalfree = false) {
        if(totalfree && nextBuffer) {
            nextBuffer->freeMemory();
            delete nextBuffer;
            nextBuffer = nullptr;
        }
        if (x) cudaFree(x);
        if (y) cudaFree(y);
        if (depth) cudaFree(depth);
        x = y = nullptr;
        depth = nullptr;
    }
};
namespace contextVars{

    std::vector<unsigned int> *SCR_SIZE;
    std::vector<double> *cOff;

    int *pcrunch;
    double *pscale;
    fSTATE *frameState;
    int *pmode;
    int *pnumSMs;
    int* temppass;

    checkPointData *d_depthArray;
    checkPointData *d_depthArraySwap = nullptr;
    cudaStream_t preLoadingStream;

}

unsigned int AApass = 0;
const unsigned int maxAApass = 32;
bool preLoaded = false;
double oldscale = 1.0;
float dscale = 1;
    


// Use a functor to generate sequence and filter based on mask
struct generate_indices : public thrust::unary_function<thrust::tuple<int, int, int>, int> {
    __host__ __device__
    int operator()(const thrust::tuple<int, int, int>& t) {
        int mask = thrust::get<0>(t);
        int scanned = thrust::get<1>(t);
        int index = thrust::get<2>(t);
        return (mask == 1) ? index : -1;
    }
};

void ExecuteCudaFrameProcessor(int*,fSTATE);
extern "C" void ExecuteCudaFrameProcessor(int *dst)
{
    ExecuteCudaFrameProcessor(dst, fSTATE::UNKNOWN_STATE);
}
extern "C" void PreAllocateMemory(std::vector<unsigned int> new_SCR_SIZE, std::vector<int> bufferOffset, int SoftRender);



#define SAMPLE_SIZE 64
#include <random>
#include <ctime>
__constant__ double SAMPLE_ARRAY[SAMPLE_SIZE][2] = {
    {0.5488135,  0.71518937}, {0.60276338, 0.54488318}, {0.4236548,  0.64589411},
    {0.43758721, 0.891773  }, {0.96366276, 0.38344152}, {0.79172504, 0.52889492},
    {0.56804456, 0.92559664}, {0.07103606, 0.0871293 }, {0.0202184,  0.83261985},
    {0.77815675, 0.87001215}, {0.97861834, 0.79915856}, {0.46147936, 0.78052918},
    {0.11827443, 0.63992102}, {0.14335329, 0.94466892}, {0.52184832, 0.41466194},
    {0.26455561, 0.77423369}, {0.45615033, 0.56843395}, {0.0187898,  0.6176355 },
    {0.61209572, 0.616934  }, {0.94374808, 0.6818203 }, {0.3595079,  0.43703195},
    {0.6976312,  0.06022547}, {0.66676672, 0.67063787}, {0.21038256, 0.1289263 },
    {0.31542835, 0.36371077}, {0.57019677, 0.43860151}, {0.98837384, 0.10204481},
    {0.20887676, 0.16130952}, {0.65310833, 0.2532916 }, {0.46631077, 0.24442559},
    {0.15896958, 0.11037514}, {0.65632959, 0.13818295}, {0.19658236, 0.36872517},
    {0.82099323, 0.09710128}, {0.83794491, 0.09609841}, {0.97645947, 0.4686512 },
    {0.97676109, 0.60484552}, {0.73926358, 0.03918779}, {0.28280696, 0.12019656},
    {0.2961402,  0.11872772}, {0.31798318, 0.41426299}, {0.0641475,  0.69247212},
    {0.56660145, 0.26538949}, {0.52324805, 0.09394051}, {0.5759465,  0.9292962 },
    {0.31856895, 0.66741038}, {0.13179786, 0.7163272 }, {0.28940609, 0.18319136},
    {0.58651293, 0.02010755}, {0.82894003, 0.00469548}, {0.67781654, 0.27000797},
    {0.73519402, 0.96218855}, {0.24875314, 0.57615733}, {0.59204193, 0.57225191},
    {0.22308163, 0.95274901}, {0.44712538, 0.84640867}, {0.69947928, 0.29743695},
    {0.81379782, 0.39650574}, {0.8811032,  0.58127287}, {0.88173536, 0.69253159},
    {0.72525428, 0.50132438}, {0.95608363, 0.6439902 }, {0.42385505, 0.60639321},
    {0.0191932,  0.30157482}
};