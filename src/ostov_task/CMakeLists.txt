cmake_minimum_required(VERSION 3.5)
project(ostov_task LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

find_package(OpenMP REQUIRED)

add_executable(ostov_task main.cpp)

target_link_libraries(ostov_task PRIVATE nlohmann_json::nlohmann_json OpenMP::OpenMP_CXX)