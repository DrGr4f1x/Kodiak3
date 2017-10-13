# Install script for directory: D:/Kodiak3/External/DirectXShaderCompiler/cmake/modules

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/llvm/cmake/LLVMExports.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/llvm/cmake/LLVMExports.cmake"
         "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/Export/share/llvm/cmake/LLVMExports.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/llvm/cmake/LLVMExports-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/llvm/cmake/LLVMExports.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE FILE FILES "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/Export/share/llvm/cmake/LLVMExports.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE FILE FILES "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/Export/share/llvm/cmake/LLVMExports-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE FILE FILES "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/Export/share/llvm/cmake/LLVMExports-minsizerel.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE FILE FILES "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/Export/share/llvm/cmake/LLVMExports-relwithdebinfo.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE FILE FILES "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/Export/share/llvm/cmake/LLVMExports-release.cmake")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE FILE FILES
    "D:/Kodiak3/External/hlsl.bin/cmake/modules/CMakeFiles/LLVMConfig.cmake"
    "D:/Kodiak3/External/hlsl.bin/share/llvm/cmake/LLVMConfigVersion.cmake"
    "D:/Kodiak3/External/DirectXShaderCompiler/cmake/modules/LLVM-Config.cmake"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/llvm/cmake" TYPE DIRECTORY FILES "D:/Kodiak3/External/DirectXShaderCompiler/cmake/modules/." FILES_MATCHING REGEX "/[^/]*\\.cmake$" REGEX "/\\.svn$" EXCLUDE REGEX "/llvmconfig\\.cmake$" EXCLUDE REGEX "/llvmconfigversion\\.cmake$" EXCLUDE REGEX "/llvm\\-config\\.cmake$" EXCLUDE REGEX "/gethosttriple\\.cmake$" EXCLUDE REGEX "/versionfromvcs\\.cmake$" EXCLUDE REGEX "/checkatomic\\.cmake$" EXCLUDE)
endif()

