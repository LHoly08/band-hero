cmake_minimum_required(VERSION 3.31)
project(BandHeroSongProcessor LANGUAGES C CXX)

option(SONG_PROCESSOR_ENABLE_CUDA "Build CUDA neural network commands" ON)

if(SONG_PROCESSOR_ENABLE_CUDA)
    enable_language(CUDA)
    find_package(CUDAToolkit REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(song_processor_cpu_nn STATIC
    src/CPU-NN/CrossEntropy.cpp
    src/CPU-NN/Linear.cpp
    src/CPU-NN/Matmul.cpp
    src/CPU-NN/ReLU.cpp
    src/CPU-NN/Softmax.cpp
    src/CPU-NN/Tensor.cpp
    src/CPU-NN/Transpose.cpp
)
target_include_directories(song_processor_cpu_nn PUBLIC include/CPU-NN)

add_library(song_processor_kissfft STATIC
    third_party/kissfft/kiss_fft.c
    third_party/kissfft/kiss_fftr.c
)
target_include_directories(song_processor_kissfft PUBLIC third_party/kissfft)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    # Preserve the upstream KissFFT floating-point behavior.
    target_compile_options(song_processor_kissfft PRIVATE -ffast-math -fomit-frame-pointer)
endif()
if(UNIX)
    target_link_libraries(song_processor_kissfft PRIVATE m)
endif()

add_library(song_processor_music STATIC
    src/music/Audio.cpp
    src/music/DetectOnsets.cpp
    src/music/FFT.cpp
    src/music/Spectrogram.cpp
    src/music/SpectralFlux.cpp
)
target_include_directories(song_processor_music
    PUBLIC include/music
    PRIVATE third_party/dr_wav
)
target_link_libraries(song_processor_music PUBLIC song_processor_cpu_nn PRIVATE song_processor_kissfft)

add_executable(song_processor src/main.cpp)
target_link_libraries(song_processor PRIVATE song_processor_music)

set(song_processor_cpp_targets song_processor song_processor_cpu_nn song_processor_music)

if(SONG_PROCESSOR_ENABLE_CUDA)
    add_library(song_processor_cuda_nn STATIC
        src/CUDA-NN/CrossEntropyCuda.cu
        src/CUDA-NN/LinearCuda.cu
        src/CUDA-NN/MatmulCuda.cu
        src/CUDA-NN/MNIST.cpp
        src/CUDA-NN/MnistCommands.cpp
        src/CUDA-NN/ModelCuda.cu
        src/CUDA-NN/ReLUCuda.cu
        src/CUDA-NN/SoftmaxCuda.cu
        src/CUDA-NN/TensorCuda.cpp
        src/CUDA-NN/TransposeCuda.cu
    )
    set_target_properties(song_processor_cuda_nn PROPERTIES
        CUDA_STANDARD 20
        CUDA_STANDARD_REQUIRED ON
        CUDA_EXTENSIONS OFF
    )
    target_include_directories(song_processor_cuda_nn PUBLIC include/CUDA-NN)
    target_link_libraries(song_processor_cuda_nn PUBLIC song_processor_cpu_nn CUDA::cudart)
    target_compile_definitions(song_processor PRIVATE SONG_PROCESSOR_ENABLE_CUDA)
    target_link_libraries(song_processor PRIVATE song_processor_cuda_nn)
    list(APPEND song_processor_cpp_targets song_processor_cuda_nn)
endif()

foreach(target IN LISTS song_processor_cpp_targets)
    if(MSVC)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)
    else()
        target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic>")
    endif()
endforeach()

if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(song_processor PRIVATE stdc++exp)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(song_processor_spectral_flux_test tests/SpectralFluxTest.cpp)
    target_link_libraries(song_processor_spectral_flux_test PRIVATE song_processor_music)
    add_test(NAME song_processor_spectral_flux COMMAND song_processor_spectral_flux_test)
endif()
