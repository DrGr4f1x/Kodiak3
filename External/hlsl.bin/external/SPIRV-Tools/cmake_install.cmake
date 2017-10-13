# Install script for directory: D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv-tools" TYPE FILE FILES
    "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools/include/spirv-tools/libspirv.h"
    "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools/include/spirv-tools/libspirv.hpp"
    "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools/include/spirv-tools/optimizer.hpp"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Kodiak3/External/hlsl.bin/external/SPIRV-Tools/external/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/external/SPIRV-Tools/source/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/external/SPIRV-Tools/tools/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/external/SPIRV-Tools/test/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/external/SPIRV-Tools/examples/cmake_install.cmake")

endif()

