/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Option Parsing Definitions                                                 *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

/////////
// Prefixes

#ifdef PREFIX
#define COMMA ,
PREFIX(prefix_0, {0})
PREFIX(prefix_3, {"-" COMMA 0})
PREFIX(prefix_4, {"-" COMMA "--" COMMA "/" COMMA 0})
PREFIX(prefix_1, {"-" COMMA "/" COMMA 0})
PREFIX(prefix_2, {"--" COMMA 0})
#undef COMMA
#endif

/////////
// Groups

#ifdef OPTION
OPTION(0, "<action group>", Action_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<CompileOnly group>", CompileOnly_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<I group>", I_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<M group>", M_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<O group>", O_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<R group>", R_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<R (with value) group>", R_value_Group, Group, R_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<T group>", T_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<W group>", W_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<W (with value) group>", W_value_Group, Group, W_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang i group>", clang_i_Group, Group, i_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<d group>", d_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<f group>", f_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<f (clang-only) group>", f_clang_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<g group>", g_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<g flags group>", g_flags_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "HLSL Compilation", hlslcomp_Group, Group, INVALID, INVALID, 0, 0, 0,
       "Compilation Options", 0)
OPTION(0, "HLSL Core", hlslcore_Group, Group, INVALID, INVALID, 0, 0, 0,
       "Common Options", 0)
OPTION(0, "HLSL Optimization", hlsloptz_Group, Group, INVALID, INVALID, 0, 0, 0,
       "Optimization Options", 0)
OPTION(0, "HLSL Utility", hlslutil_Group, Group, INVALID, INVALID, 0, 0, 0,
       "Utility Options", 0)
OPTION(0, "<i group>", i_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang internal options>", internal_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "<clang debug/development internal options>", internal_debug_Group, Group, internal_Group, INVALID, 0, 0, 0,
       "DEBUG/DEVELOPMENT OPTIONS", 0)
OPTION(0, "<clang driver internal options>", internal_driver_Group, Group, internal_Group, INVALID, 0, 0, 0,
       "DRIVER OPTIONS", 0)
OPTION(0, "<m group>", m_Group, Group, CompileOnly_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(0, "SPIR-V CodeGen", spirv_Group, Group, INVALID, INVALID, 0, 0, 0,
       "SPIR-V CodeGen Options", 0)
OPTION(0, "<u group>", u_Group, Group, INVALID, INVALID, 0, 0, 0, 0, 0)

//////////
// Options

OPTION(prefix_0, "<input>", INPUT, Input, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_0, "<unknown>", UNKNOWN, Unknown, INVALID, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "?", _help_question, Flag, INVALID, help, 0, DriverOption, 0, 0, 0)
OPTION(prefix_2, "all-warnings", _all_warnings, Flag, INVALID, Wall, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "all_resources_bound", all_resources_bound, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Enables agressive flattening", 0)
OPTION(prefix_1, "ast-dump", ast_dump, Flag, INVALID, INVALID, 0, CoreOption | DriverOption | HelpHidden, 0,
       "Dumps the parsed Abstract Syntax Tree.", 0)
OPTION(prefix_1, "Cc", Cc, Flag, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output color coded assembly listings", 0)
OPTION(prefix_1, "denorm", denorm, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "select denormal value options (any, preserve, ftz). any is the default.", 0)
OPTION(prefix_1, "dumpbin", dumpbin, Flag, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Load a binary file rather than compiling", 0)
OPTION(prefix_1, "D", D, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Define macro", 0)
OPTION(prefix_3, "d", d_Flag, Flag, d_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "d", d_Joined, Joined, d_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "external-fn", external_fn, Separate, hlslcore_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "External function name to load for compiler support", 0)
OPTION(prefix_1, "external", external_lib, Separate, hlslcore_Group, INVALID, 0, DriverOption | HelpHidden, 0,
       "External DLL name to load for compiler support", 0)
OPTION(prefix_1, "extractrootsignature", extractrootsignature, Flag, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Extract root signature from shader bytecode (must be used with /Fo <file>)", 0)
OPTION(prefix_1, "E", entrypoint, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Entry point name", 0)
OPTION(prefix_3, "fassociative-math", fassociative_math, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "fcgl", fcgl, Flag, hlslcore_Group, INVALID, 0, CoreOption | HelpHidden, 0,
       "Generate high-level code only", 0)
OPTION(prefix_3, "fconstexpr-backtrace-limit=", fconstexpr_backtrace_limit_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fconstexpr-depth=", fconstexpr_depth_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fconstexpr-steps=", fconstexpr_steps_EQ, Joined, f_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Fc", Fc, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output assembly code listing file", "<file>")
OPTION(prefix_3, "fdiagnostics-color=", fdiagnostics_color_EQ, Joined, hlslcomp_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fdiagnostics-color", fdiagnostics_color, Flag, hlslcomp_Group, INVALID, 0, CoreOption | DriverOption, 0, 0, 0)
OPTION(prefix_3, "fdiagnostics-show-category=", fdiagnostics_show_category_EQ, Joined, hlslcomp_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Fd", Fd, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Write debug information to the given file or directory; trail \\ to auto-generate and imply Qstrip_priv", "<file>")
OPTION(prefix_3, "ferror-limit=", ferror_limit_EQ, Joined, hlslcomp_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "Fe", Fe, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output warnings and errors to the given file", "<file>")
OPTION(prefix_3, "ffinite-math-only", ffinite_math_only, Flag, hlsloptz_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_3, "fhonor-infinities", fhonor_infinities, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fhonor-nans", fhonor_nans, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "Fh", Fh, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output header file containing object code", "<file>")
OPTION(prefix_3, "flimited-precision=", flimited_precision_EQ, Joined, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fno-associative-math", fno_associative_math, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fno-finite-math-only", fno_finite_math_only, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fno-honor-infinities", fno_honor_infinities, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fno-honor-nans", fno_honor_nans, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fno-reciprocal-math", fno_reciprocal_math, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fno-unsafe-math-optimizations", fno_unsafe_math_optimizations, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_1, "force_rootsig_ver", force_rootsig_ver, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "force root signature version (rootsig_1_1 if omitted)", "<profile>")
OPTION(prefix_1, "Fo", Fo, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output object file", "<file>")
OPTION(prefix_3, "fsigned-zeros", fsigned_zeros, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "funsafe-math-optimizations", funsafe_math_optimizations, Flag, hlsloptz_Group, INVALID, 0, 0, 0, 0, 0)
OPTION(prefix_3, "fvk-stage-io-order=", fvk_stage_io_order_EQ, Joined, spirv_Group, INVALID, 0, CoreOption | DriverOption | HelpHidden, 0,
       "Specify Vulkan stage I/O location assignment order", 0)
OPTION(prefix_1, "Ges", Ges, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Enable strict mode", 0)
OPTION(prefix_1, "getprivate", getprivate, JoinedOrSeparate, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Save private data from shader blob", "<file>")
OPTION(prefix_1, "Gfa", Gfa, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Avoid flow control constructs", 0)
OPTION(prefix_1, "Gfp", Gfp, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Prefer flow control constructs", 0)
OPTION(prefix_1, "Gis", Gis, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Force IEEE strictness", 0)
OPTION(prefix_2, "help-hidden", _help_hidden, Flag, INVALID, INVALID, 0, DriverOption, 0, 0, 0)
OPTION(prefix_4, "help", help, Flag, hlslcore_Group, INVALID, 0, DriverOption, 0,
       "Display available options", 0)
OPTION(prefix_1, "HV", hlsl_version, Separate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "HLSL version (2016, 2017)", 0)
OPTION(prefix_3, "H", H, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Show header includes and nesting depth", 0)
OPTION(prefix_1, "ignore-line-directives", ignore_line_directives, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Ignore line directives", 0)
OPTION(prefix_1, "I", I, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Add directory to include search path", 0)
OPTION(prefix_1, "Lx", Lx, Flag, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output hexadecimal literals", 0)
OPTION(prefix_1, "Ni", Ni, Flag, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output instruction numbers in assembly listings", 0)
OPTION(prefix_1, "no-min-precision", no_min_precision, Flag, INVALID, INVALID, 0, CoreOption | DriverOption | HelpHidden, 0,
       "Do not use min precision but use strict precision types.", 0)
OPTION(prefix_1, "no-warnings", no_warnings, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Suppress warnings", 0)
OPTION(prefix_1, "nologo", nologo, Flag, hlslcore_Group, INVALID, 0, DriverOption, 0,
       "Suppress copyright message", 0)
OPTION(prefix_1, "not_use_legacy_cbuf_load", not_use_legacy_cbuf_load, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Do not use legacy cbuffer load", 0)
OPTION(prefix_1, "No", No, Flag, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Output instruction byte offsets in assembly listings", 0)
OPTION(prefix_1, "O0", O0, Flag, hlsloptz_Group, INVALID, 0, CoreOption, 0,
       "Optimization Level 0", 0)
OPTION(prefix_1, "O1", O1, Flag, hlsloptz_Group, INVALID, 0, CoreOption, 0,
       "Optimization Level 1", 0)
OPTION(prefix_1, "O2", O2, Flag, hlsloptz_Group, INVALID, 0, CoreOption, 0,
       "Optimization Level 2", 0)
OPTION(prefix_1, "O3", O3, Flag, hlsloptz_Group, INVALID, 0, CoreOption, 0,
       "Optimization Level 3 (Default)", 0)
OPTION(prefix_1, "O4", O4, Flag, hlsloptz_Group, INVALID, 0, CoreOption, 0,
       "Optimization Level 4", 0)
OPTION(prefix_1, "Odump", Odump, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Print the optimizer commands.", 0)
OPTION(prefix_1, "Od", Od, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Disable optimizations", 0)
OPTION(prefix_1, "pack_optimized", pack_optimized, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Optimize signature packing assuming identical signature provided for each connecting stage", 0)
OPTION(prefix_1, "pack_prefix_stable", pack_prefix_stable, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "(default) Pack signatures preserving prefix-stable property - appended elements will not disturb placement of prior elements", 0)
OPTION(prefix_1, "P", P, Separate, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Preprocess to file (must be used alone)", 0)
OPTION(prefix_1, "Qstrip_debug", Qstrip_debug, Flag, hlslutil_Group, INVALID, 0, CoreOption, 0,
       "Strip debug information from 4_0+ shader bytecode  (must be used with /Fo <file>)", 0)
OPTION(prefix_1, "Qstrip_priv", Qstrip_priv, Flag, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Strip private data from shader bytecode  (must be used with /Fo <file>)", 0)
OPTION(prefix_1, "Qstrip_reflect", Qstrip_reflect, Flag, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Strip reflection data from shader bytecode  (must be used with /Fo <file>)", 0)
OPTION(prefix_1, "Qstrip_rootsignature", Qstrip_rootsignature, Flag, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Strip root signature data from shader bytecode  (must be used with /Fo <file>)", 0)
OPTION(prefix_3, "Qunused-arguments", Qunused_arguments, Flag, hlslcore_Group, INVALID, 0, CoreOption, 0,
       "Don't emit warning for unused driver arguments", 0)
OPTION(prefix_1, "recompile", recompile, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "recompile from DXIL container with Debug Info or Debug Info bitcode file", 0)
OPTION(prefix_1, "rootsig-define", rootsig_define, Separate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Read root signature from a #define", 0)
OPTION(prefix_1, "setprivate", setprivate, JoinedOrSeparate, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Private data to add to compiled shader blob", "<file>")
OPTION(prefix_1, "setrootsignature", setrootsignature, JoinedOrSeparate, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Attach root signature to shader bytecode", "<file>")
OPTION(prefix_3, "spirv", spirv, Flag, spirv_Group, INVALID, 0, CoreOption | DriverOption | HelpHidden, 0,
       "Generate SPIR-V binary code", 0)
OPTION(prefix_1, "T", target_profile, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Target profile", 0)
OPTION(prefix_1, "Vd", VD, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Disable validation", 0)
OPTION(prefix_1, "verifyrootsignature", verifyrootsignature, JoinedOrSeparate, hlslutil_Group, INVALID, 0, DriverOption, 0,
       "Verify shader bytecode with root signature", "<file>")
OPTION(prefix_1, "Vi", _vi, Flag, hlslcomp_Group, H, 0, CoreOption, 0,
       "Display details about the include process.", 0)
OPTION(prefix_1, "Vn", Vn, JoinedOrSeparate, hlslcomp_Group, INVALID, 0, DriverOption, 0,
       "Use <name> as variable name in header file", "<name>")
OPTION(prefix_3, "Wall", Wall, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_3, "Wdeprecated", Wdeprecated, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0, 0, 0)
OPTION(prefix_1, "WX", _SLASH_WX, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Treat warnings as errors", 0)
OPTION(prefix_1, "Zi", _SLASH_Zi, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Enable debug information", 0)
OPTION(prefix_1, "Zpc", Zpc, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Pack matrices in column-major order", 0)
OPTION(prefix_1, "Zpr", Zpr, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Pack matrices in row-major order", 0)
OPTION(prefix_1, "Zsb", Zsb, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Build debug name considering only output binary", 0)
OPTION(prefix_1, "Zss", Zss, Flag, hlslcomp_Group, INVALID, 0, CoreOption, 0,
       "Build debug name considering source information", 0)
#endif
