cmake_minimum_required(VERSION 3.5)

project (tkDNN)
set(CORE_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src")
set(CORE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
include_directories(${CORE_INCLUDE_DIR})

set(tkDNN_ROOT_DIR "/tkDNN")
set(tkDNN_SOURCE_DIR "${tkDNN_ROOT_DIR}/src")
set(tkDNN_INCLUDE_DIR "${tkDNN_ROOT_DIR}/include")
include_directories("${tkDNN_INCLUDE_DIR}/tkDNN")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${tkDNN_ROOT_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Wno-deprecated-declarations -Wno-unused-variable")
message("CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
message("CMAKE_CXX_COMPILER_VERSION: " ${CMAKE_CXX_COMPILER_VERSION})

# project specific flags
if(DEBUG)
    add_definitions(-DDEBUG)
endif()

#-------------------------------------------------------------------------------
# CUDA
#-------------------------------------------------------------------------------
find_package(CUDA 9.0 REQUIRED)
SET(CUDA_SEPARABLE_COMPILATION ON)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} --maxrregcount=32)

# cuDNN
find_package(CUDNN REQUIRED)
include_directories(${CUDNN_INCLUDE_DIR})

# Boost
find_package(Boost 1.65 REQUIRED COMPONENTS filesystem program_options)
include_directories(${Boost_INCLUDE_DIRS})
message("Boost found: " ${Boost_FOUND})
message("Boost include dirs: " ${Boost_INCLUDE_DIRS})

# compile
file(GLOB tkdnn_CUSRC "${tkDNN_SOURCE_DIR}/kernels/*.cu" "${tkDNN_SOURCE_DIR}/sorting.cu")
cuda_include_directories(${tkDNN_INCLUDE_DIR} ${CORE_INCLUDE_DIR} ${CUDA_INCLUDE_DIRS})
cuda_add_library(kernels SHARED ${tkdnn_CUSRC})

#-------------------------------------------------------------------------------
# External Libraries
#-------------------------------------------------------------------------------
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(OpenCV REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENCV")

set(JSON_ROOT_DIR "${PROJECT_SOURCE_DIR}/nlohmann_json_cmake_fetchcontent/")
set(JSON_INCLUDE_DIR "${JSON_ROOT_DIR}/include")
add_subdirectory(${JSON_ROOT_DIR})
include_directories(${JSON_INCLUDE_DIR})
set(JSON_BuildTests OFF CACHE INTERNAL "")

# # Messagepack. Can't figure out how to actually get msgpack_DIR
# find_package(msgpack REQUIRED)

#-------------------------------------------------------------------------------
# Build Libraries
#-------------------------------------------------------------------------------
file(GLOB tkdnn_SRC "${tkDNN_SOURCE_DIR}/*.cpp")
set(tkdnn_LIBS kernels ${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES} ${CUDNN_LIBRARIES} ${OpenCV_LIBS} yaml-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${CORE_INCLUDE_DIR} ${tkDNN_ROOT_DIR}/include ${CUDA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${NVINFER_INCLUDES})
add_library(tkDNN SHARED ${tkdnn_SRC})
target_link_libraries(tkDNN ${tkdnn_LIBS})

set(CORE_SOURCE
"${CORE_SOURCE_DIR}/video.cpp"
"${CORE_SOURCE_DIR}/draw.cpp")
set(CORE_HEADERS "")

set(TRIMMER_SOURCE "${CORE_SOURCE}"
"${CORE_SOURCE_DIR}/filter_detections.cpp"
"${CORE_SOURCE_DIR}/trimmer.cpp")
set(TRIMMER_HEADERS "")

set(ZMQTEST_SOURCE "${CORE_SOURCE_DIR}/zmqtest.cpp" "${CORE_SOURCE_DIR}/Frame.cpp")
set(ZMQTEST_HEADERS "")

set(TRIMMER_APP "trimmer")
add_executable(${TRIMMER_APP} ${TRIMMER_SOURCE})
target_link_libraries(${TRIMMER_APP} nlohmann_json::nlohmann_json ${Boost_LIBRARIES} tkDNN)

set(ZMQTEST_APP "zmqtest")
add_executable(${ZMQTEST_APP} ${ZMQTEST_SOURCE})
target_link_libraries(${ZMQTEST_APP} nlohmann_json::nlohmann_json ${OpenCV_LIBS} "/usr/lib/x86_64-linux-gnu/libzmq.so" tkDNN)
