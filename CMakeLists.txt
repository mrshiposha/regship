cmake_minimum_required(VERSION 3.19)
project(reg-ship)

option(BUILD_TESTS "Build tests for the project" ON)

file(GLOB SRC ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

if (BUILD_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/deps/catch2)
    add_subdirectory(tests)
endif(BUILD_TESTS)
