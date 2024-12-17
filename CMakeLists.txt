cmake_minimum_required(VERSION 3.10)

project(Fission VERSION 1.0 LANGUAGES CXX)

find_package(Vulkan REQUIRED)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "src/*.cpp")

# Add platform-specific sources
if(WIN32)
    file(GLOB WINDOWS_SOURCES "src/windows/*.cpp")
    list(APPEND SOURCES ${WINDOWS_SOURCES})
elseif(APPLE)
    file(GLOB MACOS_SOURCES "src/macos/*.cpp")
    list(APPEND SOURCES ${MACOS_SOURCES})
    find_package(glfw3 REQUIRED)
endif()

add_library(Fission ${SOURCES})

# Specify include directories for the library
target_include_directories(Fission PUBLIC include/)

# Add include directories and link Vulkan library
target_include_directories(Fission PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Fission PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(Fission PRIVATE glfw)

target_compile_definitions(Fission PRIVATE __TITLE__="what?")
