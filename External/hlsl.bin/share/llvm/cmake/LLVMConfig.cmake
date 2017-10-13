# This file provides information and services to the final user.


# LLVM_BUILD_* values available only from LLVM build tree.
set(LLVM_BUILD_BINARY_DIR "D:/Kodiak3/External/hlsl.bin")
set(LLVM_BUILD_LIBRARY_DIR "D:/Kodiak3/External/hlsl.bin/$(Configuration)/lib")
set(LLVM_BUILD_MAIN_INCLUDE_DIR "D:/Kodiak3/External/DirectXShaderCompiler/include")
set(LLVM_BUILD_MAIN_SRC_DIR "D:/Kodiak3/External/DirectXShaderCompiler")


set(LLVM_VERSION_MAJOR 3)
set(LLVM_VERSION_MINOR 7)
set(LLVM_VERSION_PATCH 0)
set(LLVM_PACKAGE_VERSION 3.7-3a6f80e)

set(LLVM_COMMON_DEPENDS )

set(LLVM_AVAILABLE_LIBS LLVMSupport;LLVMMSSupport;LLVMTableGen;LLVMCore;LLVMIRReader;LLVMBitReader;LLVMBitWriter;LLVMTransformUtils;LLVMInstCombine;LLVMScalarOpts;LLVMipo;LLVMVectorize;LLVMLinker;LLVMAnalysis;LLVMipa;LLVMLTO;LLVMOption;LLVMTarget;LLVMAsmParser;LLVMProfileData;LLVMDxcSupport;LLVMHLSL)

set(LLVM_ALL_TARGETS AMDGPU;NVPTX)

set(LLVM_TARGETS_TO_BUILD None)

set(LLVM_TARGETS_WITH_JIT X86;PowerPC;AArch64;ARM;Mips;SystemZ)


set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMSupport )
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMMSSupport )
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMTableGen LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMCore LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMIRReader LLVMAsmParser;LLVMBitReader;LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMBitReader LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMBitWriter LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMTransformUtils LLVMAnalysis;LLVMCore;LLVMSupport;LLVMipa)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMInstCombine LLVMAnalysis;LLVMCore;LLVMSupport;LLVMTransformUtils)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMScalarOpts LLVMAnalysis;LLVMCore;LLVMInstCombine;LLVMProfileData;LLVMSupport;LLVMTransformUtils)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMipo LLVMAnalysis;LLVMCore;LLVMInstCombine;LLVMScalarOpts;LLVMSupport;LLVMTransformUtils;LLVMVectorize;LLVMipa)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMVectorize LLVMAnalysis;LLVMCore;LLVMSupport;LLVMTransformUtils)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMLinker LLVMCore;LLVMSupport;LLVMTransformUtils)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMAnalysis LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMipa LLVMAnalysis;LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMLTO LLVMAnalysis;LLVMBitReader;LLVMBitWriter;LLVMCore;LLVMInstCombine;LLVMLinker;LLVMScalarOpts;LLVMSupport;LLVMTarget;LLVMipa;LLVMipo)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMOption LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMTarget LLVMAnalysis;LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMAsmParser LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMProfileData LLVMCore;LLVMSupport)
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMDxcSupport )
set_property(GLOBAL PROPERTY LLVMBUILD_LIB_DEPS_LLVMHLSL LLVMCore;LLVMSupport)

set(TARGET_TRIPLE "dxil-ms-dx")

set(LLVM_ABI_BREAKING_CHECKS WITH_ASSERTS)

set(LLVM_ENABLE_ASSERTIONS ON)

set(LLVM_ENABLE_EH ON)

set(LLVM_ENABLE_RTTI ON)

set(LLVM_ENABLE_TERMINFO ON)

set(LLVM_ENABLE_THREADS ON)

set(LLVM_ENABLE_ZLIB 0)

set(LLVM_NATIVE_ARCH X86)

set(LLVM_ENABLE_PIC ON)

set(LLVM_ON_UNIX 0)
set(LLVM_ON_WIN32 1)

set(LLVM_LIBDIR_SUFFIX )

set(LLVM_INCLUDE_DIRS "D:/Kodiak3/External/DirectXShaderCompiler/include;D:/Kodiak3/External/hlsl.bin/include")
set(LLVM_LIBRARY_DIRS "D:/Kodiak3/External/hlsl.bin/$(Configuration)/lib")
set(LLVM_DEFINITIONS "-D__STDC_LIMIT_MACROS" "-D__STDC_CONSTANT_MACROS")
set(LLVM_CMAKE_DIR "D:/Kodiak3/External/DirectXShaderCompiler/cmake/modules")
set(LLVM_TOOLS_BINARY_DIR "D:/Kodiak3/External/hlsl.bin/$(Configuration)/bin")

if(NOT TARGET LLVMSupport)
  include("D:/Kodiak3/External/hlsl.bin/share/llvm/cmake/LLVMExports.cmake")
endif()

include(${LLVM_CMAKE_DIR}/LLVM-Config.cmake)
