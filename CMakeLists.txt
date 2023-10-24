cmake_minimum_required(VERSION 3.14)

project(heu VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRCS ${PROJECT_NAME}/library/*.cpp ${PROJECT_NAME}/library/*.cc)
file(GLOB_RECURSE IPCL_SRCS ${PROJECT_NAME}/library/algorithms/paillier_ipcl/*.cc)
#file(GLOB_RECURSE DGK_SRCS ${PROJECT_NAME}/library/algorithms/dgk/*.cc)
#file(GLOB_RECURSE DJ_SRCS ${PROJECT_NAME}/library/algorithms/dj/*.cc)
file(GLOB_RECURSE NP_SRCS ${PROJECT_NAME}/library/numpy/*.cc)
file(GLOB_RECURSE TEST_SRCS ${PROJECT_NAME}/library/*_test.cc)
file(GLOB_RECURSE BENCH_SRCS ${PROJECT_NAME}/library/*_bench.cc)
list(REMOVE_ITEM SRCS ${TEST_SRCS} ${BENCH_SRCS} ${IPCL_SRCS} ${NP_SRCS})
list(REMOVE_ITEM TEST_SRCS ${IPCL_SRCS} ${NP_SRCS})
list(REMOVE_ITEM BENCH_SRCS ${IPCL_SRCS} ${NP_SRCS})

find_package(OpenMP)
if (OPENMP_FOUND)
    message("OpenMP Found")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DWITH_OPENMP)
endif ()

# include directories and link libraries
include_directories(. third_party/include)
link_directories(third_party/lib)
set(EXTRA_LIB)
if (APPLE)
    find_package(benchmark REQUIRED)
    find_package(gflags REQUIRED)
    execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE BREW_libomp_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(${BREW_libomp_PREFIX}/include)
    link_libraries(${BREW_libomp_PREFIX}/lib/libomp.dylib)
    list(APPEND EXTRA_LIB benchmark::benchmark gflags)
endif ()

set(ABSL_LIB absl_int128 absl_strings absl_symbolize absl_stacktrace absl_throw_delegate)
# link libraries for the target
add_library(${PROJECT_NAME} SHARED ${SRCS})
#target_sources(${PROJECT_NAME} PRIVATE)
target_link_libraries(${PROJECT_NAME} yacl ${ABSL_LIB} # protobuf::libprotobuf protobuf::libprotobuf-lite
        tommath fmt ssl crypto blake3 sodium curve25519)
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
    target_link_libraries(${PROJECT_NAME}_${bench_name} ${PROJECT_NAME} ${EXTRA_LIB} GTest::GTest)
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
        )
