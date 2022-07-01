cmake_minimum_required(VERSION 3.5)

project (tkDNN)
set(CORE_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src")
set(CORE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
include_directories(${CORE_INCLUDE_DIR})

set(TKDNN_ROOT_DIR "/root/Development/tkDNN")
set(TKDNN_SOURCE_DIR "${TKDNN_ROOT_DIR}/src")
set(TKDNN_INCLUDE_DIR "${TKDNN_ROOT_DIR}/include")
include_directories("${TKDNN_INCLUDE_DIR}/tkDNN")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${TKDNN_ROOT_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-deprecated-declarations -Wno-unused-variable")
set(CMAKE_CXX_STANDARD 14)
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
find_package(Boost REQUIRED COMPONENTS filesystem program_options)
include_directories(${Boost_INCLUDE_DIRS})
message("Boost found: " ${Boost_FOUND})
message("Boost include dirs: " ${Boost_INCLUDE_DIRS})

# compile
file(GLOB TKDNN_CUSRC "${TKDNN_SOURCE_DIR}/kernels/*.cu" "${TKDNN_SOURCE_DIR}/sorting.cu" "${TKDNN_SOURCE_DIR}/pluginsRT/*.cpp")
cuda_include_directories(${TKDNN_INCLUDE_DIR} ${CORE_INCLUDE_DIR} ${CUDA_INCLUDE_DIRS})
cuda_add_library(kernels SHARED ${TKDNN_CUSRC})

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

#-------------------------------------------------------------------------------
# Build Libraries
#-------------------------------------------------------------------------------
file(GLOB TKDNN_SRC "${TKDNN_SOURCE_DIR}/*.cpp")
set(TKDNN_LIBS kernels ${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES} ${CUDNN_LIBRARIES} ${OpenCV_LIBS} yaml-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${CORE_INCLUDE_DIR} ${TKDNN_ROOT_DIR}/include ${CUDA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${NVINFER_INCLUDES})
add_library(tkDNN SHARED ${TKDNN_SRC})
target_link_libraries(tkDNN ${TKDNN_LIBS})

set(CORE_SOURCE
"${CORE_SOURCE_DIR}/video.cpp"
"${CORE_SOURCE_DIR}/draw.cpp")
set(CORE_HEADERS "")

set(TRIMMER_SOURCE "${CORE_SOURCE}"
"${CORE_SOURCE_DIR}/filter_detections.cpp"
"${CORE_SOURCE_DIR}/trimmer.cpp")
set(TRIMMER_HEADERS "")

set(TRIMMER_APP "trimmer")
add_executable(${TRIMMER_APP} ${TRIMMER_SOURCE})
target_link_libraries(${TRIMMER_APP} nlohmann_json::nlohmann_json ${Boost_LIBRARIES} tkDNN)
