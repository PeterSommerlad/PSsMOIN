# on macos needs to be used with -DCMAKE_CXX_COMPILER=g++ and modern gcc in path, 20210419 XCode does not support concepts

cmake_minimum_required(VERSION 3.10)

project(PSsMoinWithStruct)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("No build type specified, defaulting to Debug (no optimizations, include debug symbols), use -DCMAKE_BUILD_TYPE=RelWithDebInfo or =Release to build a release)")
endif()



file(GLOB SOURCES src/*.cpp)

add_executable(PSsMoinWithStruct ${SOURCES} )

target_include_directories(PSsMoinWithStruct PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/cute)

enable_testing()

add_test(NAME test COMMAND PSsMoinWithStruct)


message("")
message("configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("will be built in:      ${CMAKE_BINARY_DIR}")
message("CMAKE_C_COMPILER:      ${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER:    ${CMAKE_CXX_COMPILER}")
message("will install in:       ${CMAKE_INSTALL_PREFIX}")
message("\n")