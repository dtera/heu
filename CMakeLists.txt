cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0104 NEW)
if (APPLE)
    project(heu VERSION 1.0 LANGUAGES C CXX)
    option(ENABLE_GPU "whether enable gpu" OFF)
else ()
    project(heu VERSION 1.0 LANGUAGES C CXX CUDA)
    option(ENABLE_GPU "whether enable gpu" ON)
    add_definitions(-DENABLE_GPAILLIER=true)

    find_package(CUDAToolkit REQUIRED)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif ()

option(ENABLE_IPCL "whether enable ipcl" OFF)
option(ENABLE_IC "whether enable ic" OFF)
add_definitions(-DMSGPACK_NO_BOOST -DUSE_CMAKE -DNO_USE_MSGPACK)

file(GLOB_RECURSE SRCS ${PROJECT_NAME}/library/*.cpp ${PROJECT_NAME}/library/*.cc ${PROJECT_NAME}/api/*.cc)
file(GLOB_RECURSE IPCL_SRCS ${PROJECT_NAME}/library/algorithms/paillier_ipcl/*.cc)
file(GLOB_RECURSE GPU_SRCS ${PROJECT_NAME}/library/algorithms/paillier_gpu/*.cc
        ${PROJECT_NAME}/library/algorithms/paillier_gpu/gpulib/*.cu)
file(GLOB_RECURSE PAILLIER_IC_SRCS ${PROJECT_NAME}/library/algorithms/paillier_ic/*.cc)
file(GLOB_RECURSE NP_SRCS ${PROJECT_NAME}/library/numpy/*.cc)
file(GLOB_RECURSE TEST_SRCS ${PROJECT_NAME}/library/*_test*.cc)
file(GLOB_RECURSE BENCH_SRCS ${PROJECT_NAME}/library/*_bench.cc)
list(REMOVE_ITEM SRCS ${TEST_SRCS} ${BENCH_SRCS} ${NP_SRCS})
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
    endif ()
endif ()

# include directories and link libraries
include_directories(. include third_party/yacl third_party third_party/include)
link_directories(third_party/lib)

set(ABSL_LIB absl_int128 absl_strings absl_symbolize absl_stacktrace absl_throw_delegate)
set(GPU_LIB)
if (ENABLE_GPU)
    add_library(${PROJECT_NAME}_GPU SHARED ${GPU_SRCS})
    target_link_libraries(${PROJECT_NAME}_GPU CUDA::cudart)
    list(APPEND GPU_LIB ${PROJECT_NAME}_GPU)
endif ()
# link libraries for the target
add_library(${PROJECT_NAME} SHARED ${SRCS})
#target_sources(${PROJECT_NAME} PRIVATE)
target_link_libraries(${PROJECT_NAME} yacl ${ABSL_LIB} ${GPU_LIB} # protobuf::libprotobuf protobuf::libprotobuf-lite
        tommath fmt blake3 sodium curve25519 gmp)
set(EXE_TARGETS)

# Building tests
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# Building bench
foreach (bench_file ${BENCH_SRCS})
    string(REPLACE ".cc" "" bench_name ${bench_file})
    string(REGEX REPLACE "(.*)?/" "" bench_name ${bench_name})
    add_executable(${PROJECT_NAME}_${bench_name} ${bench_file})
    target_link_libraries(${PROJECT_NAME}_${bench_name} ${PROJECT_NAME} benchmark gflags GTest::GTest)
    list(APPEND EXE_TARGETS ${PROJECT_NAME}_${bench_name})
endforeach ()

enable_testing()
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
target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME}
        GTest::GTest
)
file(GLOB TEST_SUITS ${PROJECT_NAME}/tests/*_test.cc)
target_sources(${PROJECT_NAME}_tests PRIVATE ${TEST_SUITS})
list(APPEND EXE_TARGETS ${PROJECT_NAME}_tests)

foreach (target ${EXE_TARGETS})
    message("target: ${target}")
endforeach ()
set_target_properties(${PROJECT_NAME} ${EXE_TARGETS} PROPERTIES
        CXX_STANDARD ${CMAKE_CXX_STANDARD}
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_ARCHITECTURES "35;50;72"
)
if (ENABLE_GPU)
    set_target_properties(${PROJECT_NAME}_GPU PROPERTIES
            CXX_STANDARD ${CMAKE_CXX_STANDARD}
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
            CUDA_SEPARABLE_COMPILATION ON
            CUDA_ARCHITECTURES "35;50;72"
    )
endif ()
