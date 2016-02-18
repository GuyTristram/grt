cmake_minimum_required(VERSION 2.8.11)

project(grt)



include_directories(
    ./include
)

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ")

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L\".\" ")

# Define the C sources
set ( C_SRCS
    src/opengl/gl3w.c
)

# Define the CXX sources
set ( CXX_SRCS
    src/common/charrange.cpp
    src/common/valuepack.cpp
    src/common/XML.cpp
    src/core/indexbuffer.cpp
    src/core/renderstate.cpp
    src/core/rendertarget.cpp
    src/core/shaderprogram.cpp
    src/core/texture.cpp
    src/core/texturetarget.cpp
    src/core/uniform.cpp
    src/core/vertexbuffer.cpp
    src/external/stb_image.cpp
    src/math/frustum.cpp
    src/math/perlin.cpp
    src/resource/animation.cpp
    src/resource/font.cpp
    src/resource/image.cpp
    src/resource/material.cpp
    src/resource/mesh.cpp
    src/resource/pprenderer.cpp
    src/resource/resourcepool.cpp
    src/resource/scenenode.cpp
    src/resource/textureatlas.cpp
    src/resource/voxelbox.cpp
    src/noplatform/device_nop.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -g -std=c++11")

set_source_files_properties(
    ${C_SRCS} PROPERTIES COMPILE_FLAGS 
    " -g")

add_library(grt ${CXX_SRCS} ${C_SRCS})

