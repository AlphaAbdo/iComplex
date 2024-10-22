#pragma once
// Templated global function with variadic arguments
template <typename FuncWrapper, typename... Args>
__global__ void genericCaller(int mode, Args... args) {
    switch (mode) {
    default:
    case 0:
        FuncWrapper::template selfCall<float>(args...);
        break;
    case 1:
        FuncWrapper::template selfCall<double_float_r<float>>(args...);
        break;
    case 2:
        FuncWrapper::template selfCall<double>(args...);
        break;
    } 
}


constexpr int iDivUp(int a, int b) {
  return ((a % b) != 0) ? (a / b + 1) : (a / b);
}


template <typename T>
constexpr inline double LinearAdder(T x) {
    // Calculate the absolute value of x
    double absVal = std::abs(x);
    
    // Add 1 to the absolute value
    absVal += 1.0;
    
    // Restore the sign of x to the new absolute value
    double result = std::copysign(absVal, x);
    
    return result;
}


void getLastCudaError(const char *errorMessage)
{
    cudaError_t err = cudaPeekAtLastError();//cudaGetLastError();

    if (cudaSuccess != err)
    {
        fprintf(stderr, "%s: %s\n", errorMessage, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}
