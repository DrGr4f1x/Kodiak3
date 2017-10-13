# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
   message(FATAL_ERROR "CMake >= 2.6.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 2.6)
#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
set(_targetsDefined)
set(_targetsNotDefined)
set(_expectedTargets)
foreach(_expectedTarget LLVMSupport LLVMMSSupport LLVMTableGen llvm-tblgen LLVMCore LLVMIRReader LLVMBitReader LLVMBitWriter LLVMTransformUtils LLVMInstCombine LLVMScalarOpts LLVMipo LLVMVectorize LLVMLinker LLVMAnalysis LLVMipa LLVMLTO LLVMOption LLVMTarget LLVMAsmParser LLVMProfileData LLVMDxcSupport LLVMHLSL dxexp clang-tblgen)
  list(APPEND _expectedTargets ${_expectedTarget})
  if(NOT TARGET ${_expectedTarget})
    list(APPEND _targetsNotDefined ${_expectedTarget})
  endif()
  if(TARGET ${_expectedTarget})
    list(APPEND _targetsDefined ${_expectedTarget})
  endif()
endforeach()
if("${_targetsDefined}" STREQUAL "${_expectedTargets}")
  unset(_targetsDefined)
  unset(_targetsNotDefined)
  unset(_expectedTargets)
  set(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT "${_targetsDefined}" STREQUAL "")
  message(FATAL_ERROR "Some (but not all) targets in this export set were already defined.\nTargets Defined: ${_targetsDefined}\nTargets not yet defined: ${_targetsNotDefined}\n")
endif()
unset(_targetsDefined)
unset(_targetsNotDefined)
unset(_expectedTargets)


# Create imported target LLVMSupport
add_library(LLVMSupport STATIC IMPORTED)

# Create imported target LLVMMSSupport
add_library(LLVMMSSupport STATIC IMPORTED)

# Create imported target LLVMTableGen
add_library(LLVMTableGen STATIC IMPORTED)

set_target_properties(LLVMTableGen PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMSupport"
)

# Create imported target llvm-tblgen
add_executable(llvm-tblgen IMPORTED)

# Create imported target LLVMCore
add_library(LLVMCore STATIC IMPORTED)

set_target_properties(LLVMCore PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMSupport"
)

# Create imported target LLVMIRReader
add_library(LLVMIRReader STATIC IMPORTED)

set_target_properties(LLVMIRReader PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAsmParser;LLVMBitReader;LLVMCore;LLVMSupport"
)

# Create imported target LLVMBitReader
add_library(LLVMBitReader STATIC IMPORTED)

set_target_properties(LLVMBitReader PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport"
)

# Create imported target LLVMBitWriter
add_library(LLVMBitWriter STATIC IMPORTED)

set_target_properties(LLVMBitWriter PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport"
)

# Create imported target LLVMTransformUtils
add_library(LLVMTransformUtils STATIC IMPORTED)

set_target_properties(LLVMTransformUtils PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMSupport;LLVMipa"
)

# Create imported target LLVMInstCombine
add_library(LLVMInstCombine STATIC IMPORTED)

set_target_properties(LLVMInstCombine PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMSupport;LLVMTransformUtils"
)

# Create imported target LLVMScalarOpts
add_library(LLVMScalarOpts STATIC IMPORTED)

set_target_properties(LLVMScalarOpts PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMInstCombine;LLVMProfileData;LLVMSupport;LLVMTransformUtils"
)

# Create imported target LLVMipo
add_library(LLVMipo STATIC IMPORTED)

set_target_properties(LLVMipo PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMInstCombine;LLVMScalarOpts;LLVMSupport;LLVMTransformUtils;LLVMVectorize;LLVMipa"
)

# Create imported target LLVMVectorize
add_library(LLVMVectorize STATIC IMPORTED)

set_target_properties(LLVMVectorize PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMSupport;LLVMTransformUtils"
)

# Create imported target LLVMLinker
add_library(LLVMLinker STATIC IMPORTED)

set_target_properties(LLVMLinker PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport;LLVMTransformUtils"
)

# Create imported target LLVMAnalysis
add_library(LLVMAnalysis STATIC IMPORTED)

set_target_properties(LLVMAnalysis PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport"
)

# Create imported target LLVMipa
add_library(LLVMipa STATIC IMPORTED)

set_target_properties(LLVMipa PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMSupport"
)

# Create imported target LLVMLTO
add_library(LLVMLTO STATIC IMPORTED)

set_target_properties(LLVMLTO PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMBitReader;LLVMBitWriter;LLVMCore;LLVMInstCombine;LLVMLinker;LLVMScalarOpts;LLVMSupport;LLVMTarget;LLVMipa;LLVMipo"
)

# Create imported target LLVMOption
add_library(LLVMOption STATIC IMPORTED)

set_target_properties(LLVMOption PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMSupport"
)

# Create imported target LLVMTarget
add_library(LLVMTarget STATIC IMPORTED)

set_target_properties(LLVMTarget PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCore;LLVMSupport"
)

# Create imported target LLVMAsmParser
add_library(LLVMAsmParser STATIC IMPORTED)

set_target_properties(LLVMAsmParser PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport"
)

# Create imported target LLVMProfileData
add_library(LLVMProfileData STATIC IMPORTED)

set_target_properties(LLVMProfileData PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport"
)

# Create imported target LLVMDxcSupport
add_library(LLVMDxcSupport STATIC IMPORTED)

# Create imported target LLVMHLSL
add_library(LLVMHLSL STATIC IMPORTED)

set_target_properties(LLVMHLSL PROPERTIES
  INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMSupport"
)

# Create imported target dxexp
add_executable(dxexp IMPORTED)

# Create imported target clang-tblgen
add_executable(clang-tblgen IMPORTED)

# Import target "LLVMSupport" for configuration "Debug"
set_property(TARGET LLVMSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMSupport.lib"
  )

# Import target "LLVMMSSupport" for configuration "Debug"
set_property(TARGET LLVMMSSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMMSSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMMSSupport.lib"
  )

# Import target "LLVMTableGen" for configuration "Debug"
set_property(TARGET LLVMTableGen APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMTableGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMTableGen.lib"
  )

# Import target "llvm-tblgen" for configuration "Debug"
set_property(TARGET llvm-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(llvm-tblgen PROPERTIES
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/bin/llvm-tblgen.exe"
  )

# Import target "LLVMCore" for configuration "Debug"
set_property(TARGET LLVMCore APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMCore PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMCore.lib"
  )

# Import target "LLVMIRReader" for configuration "Debug"
set_property(TARGET LLVMIRReader APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMIRReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMIRReader.lib"
  )

# Import target "LLVMBitReader" for configuration "Debug"
set_property(TARGET LLVMBitReader APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMBitReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMBitReader.lib"
  )

# Import target "LLVMBitWriter" for configuration "Debug"
set_property(TARGET LLVMBitWriter APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMBitWriter PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMBitWriter.lib"
  )

# Import target "LLVMTransformUtils" for configuration "Debug"
set_property(TARGET LLVMTransformUtils APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMTransformUtils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMTransformUtils.lib"
  )

# Import target "LLVMInstCombine" for configuration "Debug"
set_property(TARGET LLVMInstCombine APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMInstCombine PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMInstCombine.lib"
  )

# Import target "LLVMScalarOpts" for configuration "Debug"
set_property(TARGET LLVMScalarOpts APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMScalarOpts PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMScalarOpts.lib"
  )

# Import target "LLVMipo" for configuration "Debug"
set_property(TARGET LLVMipo APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMipo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMipo.lib"
  )

# Import target "LLVMVectorize" for configuration "Debug"
set_property(TARGET LLVMVectorize APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMVectorize PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMVectorize.lib"
  )

# Import target "LLVMLinker" for configuration "Debug"
set_property(TARGET LLVMLinker APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMLinker PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMLinker.lib"
  )

# Import target "LLVMAnalysis" for configuration "Debug"
set_property(TARGET LLVMAnalysis APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMAnalysis PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMAnalysis.lib"
  )

# Import target "LLVMipa" for configuration "Debug"
set_property(TARGET LLVMipa APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMipa PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMipa.lib"
  )

# Import target "LLVMLTO" for configuration "Debug"
set_property(TARGET LLVMLTO APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMLTO PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMLTO.lib"
  )

# Import target "LLVMOption" for configuration "Debug"
set_property(TARGET LLVMOption APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMOption PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMOption.lib"
  )

# Import target "LLVMTarget" for configuration "Debug"
set_property(TARGET LLVMTarget APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMTarget PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMTarget.lib"
  )

# Import target "LLVMAsmParser" for configuration "Debug"
set_property(TARGET LLVMAsmParser APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMAsmParser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMAsmParser.lib"
  )

# Import target "LLVMProfileData" for configuration "Debug"
set_property(TARGET LLVMProfileData APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMProfileData PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMProfileData.lib"
  )

# Import target "LLVMDxcSupport" for configuration "Debug"
set_property(TARGET LLVMDxcSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMDxcSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMDxcSupport.lib"
  )

# Import target "LLVMHLSL" for configuration "Debug"
set_property(TARGET LLVMHLSL APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LLVMHLSL PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/lib/LLVMHLSL.lib"
  )

# Import target "dxexp" for configuration "Debug"
set_property(TARGET dxexp APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(dxexp PROPERTIES
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/bin/dxexp.exe"
  )

# Import target "clang-tblgen" for configuration "Debug"
set_property(TARGET clang-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(clang-tblgen PROPERTIES
  IMPORTED_LOCATION_DEBUG "D:/Kodiak3/External/hlsl.bin/Debug/bin/clang-tblgen.exe"
  )

# Import target "LLVMSupport" for configuration "Release"
set_property(TARGET LLVMSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMSupport.lib"
  )

# Import target "LLVMMSSupport" for configuration "Release"
set_property(TARGET LLVMMSSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMMSSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMMSSupport.lib"
  )

# Import target "LLVMTableGen" for configuration "Release"
set_property(TARGET LLVMTableGen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMTableGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMTableGen.lib"
  )

# Import target "llvm-tblgen" for configuration "Release"
set_property(TARGET llvm-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(llvm-tblgen PROPERTIES
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/bin/llvm-tblgen.exe"
  )

# Import target "LLVMCore" for configuration "Release"
set_property(TARGET LLVMCore APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMCore PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMCore.lib"
  )

# Import target "LLVMIRReader" for configuration "Release"
set_property(TARGET LLVMIRReader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMIRReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMIRReader.lib"
  )

# Import target "LLVMBitReader" for configuration "Release"
set_property(TARGET LLVMBitReader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMBitReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMBitReader.lib"
  )

# Import target "LLVMBitWriter" for configuration "Release"
set_property(TARGET LLVMBitWriter APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMBitWriter PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMBitWriter.lib"
  )

# Import target "LLVMTransformUtils" for configuration "Release"
set_property(TARGET LLVMTransformUtils APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMTransformUtils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMTransformUtils.lib"
  )

# Import target "LLVMInstCombine" for configuration "Release"
set_property(TARGET LLVMInstCombine APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMInstCombine PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMInstCombine.lib"
  )

# Import target "LLVMScalarOpts" for configuration "Release"
set_property(TARGET LLVMScalarOpts APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMScalarOpts PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMScalarOpts.lib"
  )

# Import target "LLVMipo" for configuration "Release"
set_property(TARGET LLVMipo APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMipo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMipo.lib"
  )

# Import target "LLVMVectorize" for configuration "Release"
set_property(TARGET LLVMVectorize APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMVectorize PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMVectorize.lib"
  )

# Import target "LLVMLinker" for configuration "Release"
set_property(TARGET LLVMLinker APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMLinker PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMLinker.lib"
  )

# Import target "LLVMAnalysis" for configuration "Release"
set_property(TARGET LLVMAnalysis APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMAnalysis PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMAnalysis.lib"
  )

# Import target "LLVMipa" for configuration "Release"
set_property(TARGET LLVMipa APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMipa PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMipa.lib"
  )

# Import target "LLVMLTO" for configuration "Release"
set_property(TARGET LLVMLTO APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMLTO PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMLTO.lib"
  )

# Import target "LLVMOption" for configuration "Release"
set_property(TARGET LLVMOption APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMOption PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMOption.lib"
  )

# Import target "LLVMTarget" for configuration "Release"
set_property(TARGET LLVMTarget APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMTarget PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMTarget.lib"
  )

# Import target "LLVMAsmParser" for configuration "Release"
set_property(TARGET LLVMAsmParser APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMAsmParser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMAsmParser.lib"
  )

# Import target "LLVMProfileData" for configuration "Release"
set_property(TARGET LLVMProfileData APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMProfileData PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMProfileData.lib"
  )

# Import target "LLVMDxcSupport" for configuration "Release"
set_property(TARGET LLVMDxcSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMDxcSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMDxcSupport.lib"
  )

# Import target "LLVMHLSL" for configuration "Release"
set_property(TARGET LLVMHLSL APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(LLVMHLSL PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/lib/LLVMHLSL.lib"
  )

# Import target "dxexp" for configuration "Release"
set_property(TARGET dxexp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(dxexp PROPERTIES
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/bin/dxexp.exe"
  )

# Import target "clang-tblgen" for configuration "Release"
set_property(TARGET clang-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(clang-tblgen PROPERTIES
  IMPORTED_LOCATION_RELEASE "D:/Kodiak3/External/hlsl.bin/Release/bin/clang-tblgen.exe"
  )

# Import target "LLVMSupport" for configuration "MinSizeRel"
set_property(TARGET LLVMSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "C;CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMSupport.lib"
  )

# Import target "LLVMMSSupport" for configuration "MinSizeRel"
set_property(TARGET LLVMMSSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMMSSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMMSSupport.lib"
  )

# Import target "LLVMTableGen" for configuration "MinSizeRel"
set_property(TARGET LLVMTableGen APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMTableGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMTableGen.lib"
  )

# Import target "llvm-tblgen" for configuration "MinSizeRel"
set_property(TARGET llvm-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(llvm-tblgen PROPERTIES
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/bin/llvm-tblgen.exe"
  )

# Import target "LLVMCore" for configuration "MinSizeRel"
set_property(TARGET LLVMCore APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMCore PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMCore.lib"
  )

# Import target "LLVMIRReader" for configuration "MinSizeRel"
set_property(TARGET LLVMIRReader APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMIRReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMIRReader.lib"
  )

# Import target "LLVMBitReader" for configuration "MinSizeRel"
set_property(TARGET LLVMBitReader APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMBitReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMBitReader.lib"
  )

# Import target "LLVMBitWriter" for configuration "MinSizeRel"
set_property(TARGET LLVMBitWriter APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMBitWriter PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMBitWriter.lib"
  )

# Import target "LLVMTransformUtils" for configuration "MinSizeRel"
set_property(TARGET LLVMTransformUtils APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMTransformUtils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMTransformUtils.lib"
  )

# Import target "LLVMInstCombine" for configuration "MinSizeRel"
set_property(TARGET LLVMInstCombine APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMInstCombine PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMInstCombine.lib"
  )

# Import target "LLVMScalarOpts" for configuration "MinSizeRel"
set_property(TARGET LLVMScalarOpts APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMScalarOpts PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMScalarOpts.lib"
  )

# Import target "LLVMipo" for configuration "MinSizeRel"
set_property(TARGET LLVMipo APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMipo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMipo.lib"
  )

# Import target "LLVMVectorize" for configuration "MinSizeRel"
set_property(TARGET LLVMVectorize APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMVectorize PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMVectorize.lib"
  )

# Import target "LLVMLinker" for configuration "MinSizeRel"
set_property(TARGET LLVMLinker APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMLinker PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMLinker.lib"
  )

# Import target "LLVMAnalysis" for configuration "MinSizeRel"
set_property(TARGET LLVMAnalysis APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMAnalysis PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMAnalysis.lib"
  )

# Import target "LLVMipa" for configuration "MinSizeRel"
set_property(TARGET LLVMipa APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMipa PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMipa.lib"
  )

# Import target "LLVMLTO" for configuration "MinSizeRel"
set_property(TARGET LLVMLTO APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMLTO PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMLTO.lib"
  )

# Import target "LLVMOption" for configuration "MinSizeRel"
set_property(TARGET LLVMOption APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMOption PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMOption.lib"
  )

# Import target "LLVMTarget" for configuration "MinSizeRel"
set_property(TARGET LLVMTarget APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMTarget PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMTarget.lib"
  )

# Import target "LLVMAsmParser" for configuration "MinSizeRel"
set_property(TARGET LLVMAsmParser APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMAsmParser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMAsmParser.lib"
  )

# Import target "LLVMProfileData" for configuration "MinSizeRel"
set_property(TARGET LLVMProfileData APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMProfileData PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMProfileData.lib"
  )

# Import target "LLVMDxcSupport" for configuration "MinSizeRel"
set_property(TARGET LLVMDxcSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMDxcSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMDxcSupport.lib"
  )

# Import target "LLVMHLSL" for configuration "MinSizeRel"
set_property(TARGET LLVMHLSL APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(LLVMHLSL PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/lib/LLVMHLSL.lib"
  )

# Import target "dxexp" for configuration "MinSizeRel"
set_property(TARGET dxexp APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(dxexp PROPERTIES
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/bin/dxexp.exe"
  )

# Import target "clang-tblgen" for configuration "MinSizeRel"
set_property(TARGET clang-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(clang-tblgen PROPERTIES
  IMPORTED_LOCATION_MINSIZEREL "D:/Kodiak3/External/hlsl.bin/MinSizeRel/bin/clang-tblgen.exe"
  )

# Import target "LLVMSupport" for configuration "RelWithDebInfo"
set_property(TARGET LLVMSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C;CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMSupport.lib"
  )

# Import target "LLVMMSSupport" for configuration "RelWithDebInfo"
set_property(TARGET LLVMMSSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMMSSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMMSSupport.lib"
  )

# Import target "LLVMTableGen" for configuration "RelWithDebInfo"
set_property(TARGET LLVMTableGen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMTableGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMTableGen.lib"
  )

# Import target "llvm-tblgen" for configuration "RelWithDebInfo"
set_property(TARGET llvm-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(llvm-tblgen PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/bin/llvm-tblgen.exe"
  )

# Import target "LLVMCore" for configuration "RelWithDebInfo"
set_property(TARGET LLVMCore APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMCore PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMCore.lib"
  )

# Import target "LLVMIRReader" for configuration "RelWithDebInfo"
set_property(TARGET LLVMIRReader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMIRReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMIRReader.lib"
  )

# Import target "LLVMBitReader" for configuration "RelWithDebInfo"
set_property(TARGET LLVMBitReader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMBitReader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMBitReader.lib"
  )

# Import target "LLVMBitWriter" for configuration "RelWithDebInfo"
set_property(TARGET LLVMBitWriter APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMBitWriter PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMBitWriter.lib"
  )

# Import target "LLVMTransformUtils" for configuration "RelWithDebInfo"
set_property(TARGET LLVMTransformUtils APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMTransformUtils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMTransformUtils.lib"
  )

# Import target "LLVMInstCombine" for configuration "RelWithDebInfo"
set_property(TARGET LLVMInstCombine APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMInstCombine PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMInstCombine.lib"
  )

# Import target "LLVMScalarOpts" for configuration "RelWithDebInfo"
set_property(TARGET LLVMScalarOpts APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMScalarOpts PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMScalarOpts.lib"
  )

# Import target "LLVMipo" for configuration "RelWithDebInfo"
set_property(TARGET LLVMipo APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMipo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMipo.lib"
  )

# Import target "LLVMVectorize" for configuration "RelWithDebInfo"
set_property(TARGET LLVMVectorize APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMVectorize PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMVectorize.lib"
  )

# Import target "LLVMLinker" for configuration "RelWithDebInfo"
set_property(TARGET LLVMLinker APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMLinker PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMLinker.lib"
  )

# Import target "LLVMAnalysis" for configuration "RelWithDebInfo"
set_property(TARGET LLVMAnalysis APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMAnalysis PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMAnalysis.lib"
  )

# Import target "LLVMipa" for configuration "RelWithDebInfo"
set_property(TARGET LLVMipa APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMipa PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMipa.lib"
  )

# Import target "LLVMLTO" for configuration "RelWithDebInfo"
set_property(TARGET LLVMLTO APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMLTO PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMLTO.lib"
  )

# Import target "LLVMOption" for configuration "RelWithDebInfo"
set_property(TARGET LLVMOption APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMOption PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMOption.lib"
  )

# Import target "LLVMTarget" for configuration "RelWithDebInfo"
set_property(TARGET LLVMTarget APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMTarget PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMTarget.lib"
  )

# Import target "LLVMAsmParser" for configuration "RelWithDebInfo"
set_property(TARGET LLVMAsmParser APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMAsmParser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMAsmParser.lib"
  )

# Import target "LLVMProfileData" for configuration "RelWithDebInfo"
set_property(TARGET LLVMProfileData APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMProfileData PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMProfileData.lib"
  )

# Import target "LLVMDxcSupport" for configuration "RelWithDebInfo"
set_property(TARGET LLVMDxcSupport APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMDxcSupport PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMDxcSupport.lib"
  )

# Import target "LLVMHLSL" for configuration "RelWithDebInfo"
set_property(TARGET LLVMHLSL APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LLVMHLSL PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/lib/LLVMHLSL.lib"
  )

# Import target "dxexp" for configuration "RelWithDebInfo"
set_property(TARGET dxexp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(dxexp PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/bin/dxexp.exe"
  )

# Import target "clang-tblgen" for configuration "RelWithDebInfo"
set_property(TARGET clang-tblgen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(clang-tblgen PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "D:/Kodiak3/External/hlsl.bin/RelWithDebInfo/bin/clang-tblgen.exe"
  )

# This file does not depend on other imported targets which have
# been exported from the same project but in a separate export set.

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
