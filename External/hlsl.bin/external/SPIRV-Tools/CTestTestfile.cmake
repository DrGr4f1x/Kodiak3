# CMake generated Testfile for 
# Source directory: D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools
# Build directory: D:/Kodiak3/External/hlsl.bin/external/SPIRV-Tools
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(spirv-tools-copyrights "C:/Python/Python3.6/python.exe" "utils/check_copyright.py")
  set_tests_properties(spirv-tools-copyrights PROPERTIES  WORKING_DIRECTORY "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(spirv-tools-copyrights "C:/Python/Python3.6/python.exe" "utils/check_copyright.py")
  set_tests_properties(spirv-tools-copyrights PROPERTIES  WORKING_DIRECTORY "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(spirv-tools-copyrights "C:/Python/Python3.6/python.exe" "utils/check_copyright.py")
  set_tests_properties(spirv-tools-copyrights PROPERTIES  WORKING_DIRECTORY "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(spirv-tools-copyrights "C:/Python/Python3.6/python.exe" "utils/check_copyright.py")
  set_tests_properties(spirv-tools-copyrights PROPERTIES  WORKING_DIRECTORY "D:/Kodiak3/External/DirectXShaderCompiler/external/SPIRV-Tools")
else()
  add_test(spirv-tools-copyrights NOT_AVAILABLE)
endif()
subdirs("external")
subdirs("source")
subdirs("tools")
subdirs("test")
subdirs("examples")
