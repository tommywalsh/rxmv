cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.71 REQUIRED COMPONENTS filesystem regex)

add_executable(rxmv rxmv.cpp)

target_link_libraries(rxmv Boost::regex Boost::filesystem)

