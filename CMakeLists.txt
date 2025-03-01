cmake_minimum_required(VERSION 3.10)

project(Fission VERSION 1.0 LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(Freetype REQUIRED)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
#    file(GLOB PLATFORM_SOURCES "src/macos/*.cpp")
    find_package(glfw3 REQUIRED)
endif()

#file(GLOB SOURCES "src/*.cpp")
#add_library(Fission ${SOURCES} ${PLATFORM_SOURCES})

add_library(Fission "src/main.cpp")

# Specify include directories for the library
target_include_directories(Fission PUBLIC include/)

# ************************************* vendor *************************************
target_include_directories(Fission PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(Fission PRIVATE ${Vulkan_LIBRARIES})

target_include_directories(Fission PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(Fission PRIVATE ${FREETYPE_LIBRARIES})

if(APPLE)
    target_link_libraries(Fission PRIVATE glfw)
endif()
# ************************************* vendor *************************************

target_compile_definitions(Fission PRIVATE MACRO_NAME="this is a test")
