//===- llvm-build generated file --------------------------------*- C++ -*-===//
//
// Component Library Depenedency Table
//
// Automatically generated file, do not edit!
//
//===----------------------------------------------------------------------===//

struct AvailableComponent {
  /// The name of the component.
  const char *Name;

  /// The name of the library for this component (or NULL).
  const char *Library;

  /// Whether the component is installed.
  bool IsInstalled;

  /// The list of libraries required when linking this component.
  const char *RequiredLibraries[22];
} AvailableComponents[43] = {
  { "all", 0, 1, { "hlsl", "nativecodegen", "instrumentation", "all-targets", "option", "passes", "objcarcopts", "orcjit", "mcdisassembler", "native", "debuginfodwarf", "selectiondag", "irreader", "mirparser", "engine", "asmprinter", "lto", "mcjit", "mcparser", "tablegen", "debuginfopdb" } },
  { "all-targets", 0, 1, {  } },
  { "analysis", "libLLVMAnalysis.a", 1, { "core", "support" } },
  { "asmparser", "libLLVMAsmParser.a", 1, { "core", "support" } },
  { "asmprinter", "libLLVMAsmPrinter.a", 1, { "analysis", "codegen", "core", "support", "target", "transformutils" } },
  { "bitreader", "libLLVMBitReader.a", 1, { "core", "support" } },
  { "bitwriter", "libLLVMBitWriter.a", 1, { "core", "support" } },
  { "codegen", "libLLVMCodeGen.a", 1, { "analysis", "core", "scalaropts", "support", "target", "transformutils" } },
  { "core", "libLLVMCore.a", 1, { "support" } },
  { "debuginfodwarf", "libLLVMDebugInfoDWARF.a", 1, { "object", "support" } },
  { "debuginfopdb", "libLLVMDebugInfoPDB.a", 1, { "object", "support" } },
  { "engine", 0, 1, { "interpreter" } },
  { "executionengine", "libLLVMExecutionEngine.a", 1, { "core", "mc", "object", "runtimedyld", "support", "target" } },
  { "hlsl", "libLLVMHLSL.a", 1, { "core", "support" } },
  { "instcombine", "libLLVMInstCombine.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "instrumentation", "libLLVMInstrumentation.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "interpreter", "libLLVMInterpreter.a", 1, { "codegen", "core", "executionengine", "support" } },
  { "ipa", "libLLVMipa.a", 1, { "analysis", "core", "support" } },
  { "ipo", "libLLVMipo.a", 1, { "analysis", "core", "ipa", "instcombine", "scalaropts", "support", "transformutils", "vectorize" } },
  { "irreader", "libLLVMIRReader.a", 1, { "asmparser", "bitreader", "core", "support" } },
  { "linker", "libLLVMLinker.a", 1, { "core", "support", "transformutils" } },
  { "lto", "libLLVMLTO.a", 1, { "analysis", "bitreader", "bitwriter", "core", "ipa", "ipo", "instcombine", "linker", "scalaropts", "support", "target" } },
  { "mc", "libLLVMMC.a", 1, { "support" } },
  { "mcdisassembler", "libLLVMMCDisassembler.a", 1, { "mc", "support" } },
  { "mcjit", "libLLVMMCJIT.a", 1, { "core", "executionengine", "object", "runtimedyld", "support", "target" } },
  { "mcparser", "libLLVMMCParser.a", 1, { "mc", "support" } },
  { "mirparser", "libLLVMMIRParser.a", 1, { "core", "support", "target", "asmparser", "codegen" } },
  { "native", 0, 1, {  } },
  { "nativecodegen", 0, 1, {  } },
  { "objcarcopts", "libLLVMObjCARCOpts.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "object", "libLLVMObject.a", 1, { "bitreader", "core", "support" } },
  { "option", "libLLVMOption.a", 1, { "support" } },
  { "orcjit", "libLLVMOrcJIT.a", 1, { "core", "executionengine", "object", "runtimedyld", "support", "transformutils" } },
  { "passes", "libLLVMPasses.a", 1, { "analysis", "core", "ipa", "ipo", "instcombine", "scalaropts", "support", "transformutils", "vectorize" } },
  { "profiledata", "libLLVMProfileData.a", 1, { "core", "support" } },
  { "runtimedyld", "libLLVMRuntimeDyld.a", 1, { "mc", "object", "support" } },
  { "scalaropts", "libLLVMScalarOpts.a", 1, { "analysis", "core", "instcombine", "profiledata", "support", "transformutils" } },
  { "selectiondag", "libLLVMSelectionDAG.a", 1, { "analysis", "codegen", "core", "support", "target", "transformutils" } },
  { "support", "libLLVMSupport.a", 1, {  } },
  { "tablegen", "libLLVMTableGen.a", 1, { "support" } },
  { "target", "libLLVMTarget.a", 1, { "analysis", "core", "support" } },
  { "transformutils", "libLLVMTransformUtils.a", 1, { "analysis", "core", "ipa", "support" } },
  { "vectorize", "libLLVMVectorize.a", 1, { "analysis", "core", "support", "transformutils" } },
};
