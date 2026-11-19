#pragma once

#include <cstdio>
#include <exception>

#include <cuda_runtime.h>

#define CUDA_CHECK(call)                                                       \
  do {                                                                         \
    const cudaError_t error = (call);                                          \
    if (error != cudaSuccess) {                                                \
      std::fprintf(stderr,                                                     \
                   "CUDA error: %s\n"                                          \
                   "Call: %s\n"                                                \
                   "File: %s:%d\n",                                            \
                   cudaGetErrorString(error), #call, __FILE__, __LINE__);      \
      std::terminate();                                                        \
    }                                                                          \
  } while (false)