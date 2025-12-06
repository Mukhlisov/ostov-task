cmake_minimum_required(VERSION 3.5)
project(ostov_task)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)
find_package(nlohmann_json QUIET)

add_executable(ostov_task main.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(ostov_task PRIVATE OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP C++ support not found!")
endif()

if(NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json не найдена. Установи пакет nlohmann-json.")
endif()