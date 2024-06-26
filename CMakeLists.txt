cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0104 NEW)
if (APPLE)
    project(heu VERSION 1.0 LANGUAGES C CXX)
    option(ENABLE_GPU "whether enable gpu" OFF)
else ()
    set(CMAKE_CUDA_ARCHITECTURES 70 72 75 80 86)
    project(heu VERSION 1.0 LANGUAGES C CXX CUDA)
    option(ENABLE_GPU "whether enable gpu" ON)
    add_definitions(-DENABLE_GPAILLIER=true)

    find_package(CUDAToolkit REQUIRED)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --std=c++17")
    set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
    set(nvcc_cxx "$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>")
endif ()

option(ENABLE_IPCL "whether enable ipcl" OFF)
option(ENABLE_IC "whether enable ic" OFF)
add_definitions(-DMSGPACK_NO_BOOST -DUSE_CMAKE -DNO_USE_MSGPACK)

file(GLOB_RECURSE SRCS ${PROJECT_NAME}/library/*.c* ${PROJECT_NAME}/algorithms/*.c* ${PROJECT_NAME}/spi/*.c*
        ${PROJECT_NAME}/api/*.c*)
file(GLOB_RECURSE IPCL_SRCS ${PROJECT_NAME}/library/algorithms/paillier_ipcl/*.c*)
file(GLOB_RECURSE GPU_SRCS ${PROJECT_NAME}/library/algorithms/paillier_gpu/*.c*)
file(GLOB_RECURSE PAILLIER_IC_SRCS ${PROJECT_NAME}/library/algorithms/paillier_ic/*.c*)
file(GLOB_RECURSE NP_SRCS ${PROJECT_NAME}/library/numpy/*.c*)
file(GLOB_RECURSE TEST_SRCS ${PROJECT_NAME}/library/*_test*.c* ${PROJECT_NAME}/spi/*test*.c*)
file(GLOB_RECURSE BENCH_SRCS ${PROJECT_NAME}/library/*_bench.c*)
file(GLOB_RECURSE SEAL_GPU_SRCS ${PROJECT_NAME}/library/algorithms/seal_gpu/*.c*)
file(GLOB_RECURSE SEAL_FHE_GPU_SRCS ${PROJECT_NAME}/algorithms/seal_fhe/gpu/*.c*)
file(GLOB_RECURSE EXAMPLES_SRCS ${PROJECT_NAME}/examples/*.c*)
file(GLOB_RECURSE RM_SRCS ${PROJECT_NAME}/algorithms/seal_fhe/*.c*)

list(REMOVE_ITEM SRCS ${TEST_SRCS} ${BENCH_SRCS} ${NP_SRCS} ${SEAL_GPU_SRCS} ${SEAL_FHE_SRCS} ${RM_SRCS})
list(REMOVE_ITEM GPU_SRCS ${TEST_SRCS} ${BENCH_SRCS} ${NP_SRCS})
list(REMOVE_ITEM TEST_SRCS ${NP_SRCS})
list(REMOVE_ITEM BENCH_SRCS ${NP_SRCS})

if (NOT ENABLE_IPCL)
    list(REMOVE_ITEM SRCS ${IPCL_SRCS})
    list(REMOVE_ITEM TEST_SRCS ${IPCL_SRCS})
    list(REMOVE_ITEM BENCH_SRCS ${IPCL_SRCS})
endif ()
if (NOT ENABLE_GPU)
    list(REMOVE_ITEM SRCS ${GPU_SRCS})
    list(REMOVE_ITEM TEST_SRCS ${GPU_SRCS})
    list(REMOVE_ITEM BENCH_SRCS ${GPU_SRCS})
endif ()
if (NOT ENABLE_IC)
    list(REMOVE_ITEM SRCS ${PAILLIER_IC_SRCS})
    list(REMOVE_ITEM TEST_SRCS ${PAILLIER_IC_SRCS})
    list(REMOVE_ITEM BENCH_SRCS ${PAILLIER_IC_SRCS})
endif ()

find_package(OpenMP)
if (OPENMP_FOUND)
    message("OpenMP Found")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DWITH_OPENMP)
else ()
    if (APPLE)
        # Try again with extra path info; required for libomp 15+ from Homebrew
        execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include")
        set(OpenMP_C_LIB_NAMES omp)
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY ${LIBOMP_PREFIX}/lib/libomp.dylib)

        find_package(OpenMP REQUIRED)
        include_directories(${LIBOMP_PREFIX}/include)
        link_libraries(${LIBOMP_PREFIX}/lib/libomp.dylib)
        message("OpenMP Found")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(COMPILE_OPTIONS --expt-extended-lambda -lineinfo --Werror all-warnings)
    endif ()
endif ()

# include directories and link libraries
include_directories(. include third_party/yacl third_party third_party/include)
link_directories(third_party/lib)

# Building tests
enable_testing()
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

set(EXE_TARGETS)
set(LIB_TARGETS)
set(ABSL_LIB absl_int128 absl_strings absl_symbolize absl_stacktrace absl_throw_delegate)
set(GPU_LIB)
if (ENABLE_GPU)
    add_library(${PROJECT_NAME}_GPU SHARED ${GPU_SRCS})
    target_link_libraries(${PROJECT_NAME}_GPU)

    add_subdirectory(heu/library/algorithms/seal_gpu)
    add_library(${PROJECT_NAME}_seal_fhe_gpu SHARED ${SEAL_FHE_GPU_SRCS})
    target_link_libraries(${PROJECT_NAME}_seal_fhe_gpu seal_gpu)

    list(APPEND LIB_TARGETS ${PROJECT_NAME}_GPU ${PROJECT_NAME}_seal_fhe_gpu)
    #list(APPEND GPU_LIB ${PROJECT_NAME}_GPU)
endif ()

# add project lib
add_library(${PROJECT_NAME} SHARED ${SRCS})
# link libraries for the target
target_link_libraries(${PROJECT_NAME} yacl ${ABSL_LIB} ${GPU_LIB} # protobuf::libprotobuf protobuf::libprotobuf-lite
        tommath fmt blake3 sodium curve25519 gmp GTest::gtest GTest::gmock)
list(APPEND LIB_TARGETS ${PROJECT_NAME})

# Building examples
foreach (examples_file ${EXAMPLES_SRCS})
    if (NOT ${examples_file} MATCHES ".*\.cu" OR ENABLE_GPU)
        string(REGEX REPLACE "(.cc)|(.cpp)|(.cu)" "" examples_name ${examples_file})
        string(REGEX REPLACE "(.*)?/" "" examples_name ${examples_name})
        add_executable(${PROJECT_NAME}_${examples_name} ${examples_file})
        target_link_libraries(${PROJECT_NAME}_${examples_name} ${PROJECT_NAME} seal seal_gpu)
        list(APPEND EXE_TARGETS ${PROJECT_NAME}_${examples_name})
    endif ()
endforeach ()

# Building bench
foreach (bench_file ${BENCH_SRCS})
    string(REPLACE ".cc" "" bench_name ${bench_file})
    string(REGEX REPLACE "(.*)?/" "" bench_name ${bench_name})
    add_executable(${PROJECT_NAME}_${bench_name} ${bench_file})
    target_link_libraries(${PROJECT_NAME}_${bench_name} ${PROJECT_NAME} benchmark gflags GTest::GTest)
    list(APPEND EXE_TARGETS ${PROJECT_NAME}_${bench_name})
endforeach ()

# Building test
foreach (test_file ${TEST_SRCS})
    string(REPLACE ".cc" "" test_name ${test_file})
    string(REGEX REPLACE "(.*)?/" "" test_name ${test_name})
    string(REGEX REPLACE "/${test_name}.cc" "" module_name ${test_file})
    string(REGEX REPLACE "(.*)?/" "" module_name ${module_name})
    add_executable(${PROJECT_NAME}_${module_name}_${test_name} ${test_file})
    target_link_libraries(${PROJECT_NAME}_${module_name}_${test_name} ${PROJECT_NAME} GTest::GTest)
    list(APPEND EXE_TARGETS ${PROJECT_NAME}_${module_name}_${test_name})
endforeach ()

add_executable(${PROJECT_NAME}_tests ${PROJECT_NAME}/tests.cc)
target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME} ${PROJECT_NAME}_seal_fhe_gpu GTest::GTest)
file(GLOB TEST_SUITS ${PROJECT_NAME}/tests/*_test.c*)
target_sources(${PROJECT_NAME}_tests PRIVATE ${TEST_SUITS})
list(APPEND EXE_TARGETS ${PROJECT_NAME}_tests)

foreach (target ${EXE_TARGETS})
    message("target: ${target}")
endforeach ()
set_target_properties(${EXE_TARGETS} ${LIB_TARGETS} PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        CUDA_SEPARABLE_COMPILATION ON
)
