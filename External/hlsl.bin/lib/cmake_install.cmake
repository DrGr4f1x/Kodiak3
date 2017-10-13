# Install script for directory: D:/Kodiak3/External/DirectXShaderCompiler/lib

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
  include("D:/Kodiak3/External/hlsl.bin/lib/IR/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/IRReader/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/Bitcode/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/Transforms/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/Linker/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/Analysis/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/LTO/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/Option/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/Target/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/AsmParser/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/ProfileData/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/DxcSupport/cmake_install.cmake")
  include("D:/Kodiak3/External/hlsl.bin/lib/HLSL/cmake_install.cmake")

endif()

