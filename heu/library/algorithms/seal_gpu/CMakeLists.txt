file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp *.cuh)
file(GLOB_RECURSE CURRENT_SOURCES *.cpp *.cu *.c)

source_group("Include" FILES ${CURRENT_HEADERS})
source_group("Source" FILES ${CURRENT_SOURCES})

add_library(seal_gpu SHARED ${CURRENT_HEADERS} ${CURRENT_SOURCES})
set_target_properties(seal_gpu PROPERTIES CUDA_SEPERABLE_COMPILATION ON)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
set(nvcc_cxx "$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>")

target_compile_options(seal_gpu INTERFACE
        "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-fPIC>>"
        "$<${nvcc_cxx}:$<BUILD_INTERFACE:--compiler-options=-fPIC>>"
)

install(TARGETS seal_gpu
        EXPORT sealTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

file(GLOB MAIN_DIR_HEADERS *.h *.hpp *.cuh)
file(GLOB UTILS_HEADERS utils/*.h *.hpp *.cuh)

install(FILES ${MAIN_DIR_HEADERS} DESTINATION include/seal_gpu)
install(FILES ${UTILS_HEADERS} DESTINATION include/seal_gpu/utils)