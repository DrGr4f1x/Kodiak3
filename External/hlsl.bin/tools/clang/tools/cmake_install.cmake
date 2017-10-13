# Install script for directory: D:/Kodiak3/External/DirectXShaderCompiler/tools/clang/tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/LLVM")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/driver/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/libclang/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/d3dcomp/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxcompiler/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxa/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxc/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxopt/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxl/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxr/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxv/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dotnetc/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/tools/clang/tools/dxlib-sample/cmake_install.cmake")

endif()

