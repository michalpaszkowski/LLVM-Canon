cmake_minimum_required(VERSION 3.13.4)
project(LLVM-Canon)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 14)

include_directories(${LLVM_INCLUDE_DIRS} include)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(LLVM-Canon src/Tool.cpp include/IRCanonicalizer.h src/IRCanonicalizer.cpp)

set_target_properties(LLVM-Canon PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(LLVM-Canon ${llvm_libs})