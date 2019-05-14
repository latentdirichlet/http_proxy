cmake_minimum_required(VERSION 3.10)
project(http_proxy)

# include directories
include_directories(include)

# include boost
find_package(Boost COMPONENTS system)
IF (Boost_FOUND)
include_directories(${Boost_INCLUDE_DIR})
endif()

# include thread
find_package (Threads)

file(GLOB SOURCES "src/*.cpp")
add_executable(proxy ${SOURCES})

target_link_libraries (proxy ${Boost_LIBRARIES})
target_link_libraries (proxy ${CMAKE_THREAD_LIBS_INIT})