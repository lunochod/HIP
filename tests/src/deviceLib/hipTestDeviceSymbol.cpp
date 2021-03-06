/*
Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include<hip/hip_runtime.h>
#include<hip/hip_runtime_api.h>
#include<iostream>

#define NUM 1024
#define SIZE 1024*4

#ifdef __HIP_PLATFORM_HCC__
__attribute__((address_space(1))) int global[NUM];
#endif

#ifdef __HIP_PLATFORM_NVCC__
__device__ int global[NUM];
#endif

__global__ void Assign(hipLaunchParm lp, int* Out)
{
    int tid = hipThreadIdx_x + hipBlockIdx_x * hipBlockDim_x;
    Out[tid] = global[tid];
}

int main()
{
    int *A, *B, *Ad;
    A = new int[NUM];
    B = new int[NUM];
    for(unsigned i=0;i<NUM;i++) {
        A[i] = -1*i;
        B[i] = 0;
    }

    hipMalloc((void**)&Ad, SIZE);

    hipStream_t stream;
    hipStreamCreate(&stream);
    hipMemcpyToSymbolAsync(HIP_SYMBOL(global), A, SIZE, 0, hipMemcpyHostToDevice, stream);
    hipStreamSynchronize(stream);
    hipLaunchKernel(Assign, dim3(1,1,1), dim3(NUM,1,1), 0, 0, Ad);
    hipMemcpy(B, Ad, SIZE, hipMemcpyDeviceToHost);

    for(unsigned i=0;i<NUM;i++) {
        assert(A[i] == B[i]);
    }

    for(unsigned i=0;i<NUM;i++) {
        A[i] = -2*i;
        B[i] = 0;
    }

    hipMemcpyToSymbol(HIP_SYMBOL(global), A, SIZE, 0, hipMemcpyHostToDevice);
    hipLaunchKernel(Assign, dim3(1,1,1), dim3(NUM,1,1), 0, 0, Ad);
    hipMemcpy(B, Ad, SIZE, hipMemcpyDeviceToHost);
    for(unsigned i=0;i<NUM;i++) {
        assert(A[i] == B[i]);
    }
}
