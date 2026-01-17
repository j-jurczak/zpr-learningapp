cmake_minimum_required(VERSION 3.16)
project(LearningApp LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Multimedia)

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

add_subdirectory(src)
add_subdirectory(tests)