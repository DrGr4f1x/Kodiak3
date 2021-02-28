using System;
using System.Collections;
using System.Collections.Generic;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace ShaderCompiler
{
    class Compiler
    {
        private enum ShaderStage
        {
            Vertex,
            Pixel,
            Geometry,
            Hull,
            Domain,
            Compute
        };

        private enum ShaderCompiler
        {
            DXC,
            FXC,
            GLSLC
        };


        private enum DXShaderProfile
        {
            SP_5_0,
            SP_5_1,
            SP_6_0,
            SP_6_1,
            SP_6_2,
            SP_6_3,
            SP_6_4,
            SP_6_5,
            SP_6_6,
        };

        private enum VKShaderProfile
        {
            SP_1_0,
            SP_1_1,
            SP_1_2,
            SP_1_3,
            SP_1_4,
            SP_1_5
        };

        private enum VKTarget
        {
            VK_1_0,
            VK_1_1,
            VK_1_2
        }

        private ShaderStage m_shaderStage = ShaderStage.Vertex;
        private ShaderCompiler m_compiler = ShaderCompiler.DXC;
        private DXShaderProfile m_dxProfile = DXShaderProfile.SP_6_0;
        private VKShaderProfile m_vkProfile = VKShaderProfile.SP_1_5;
        private VKTarget m_vkTarget = VKTarget.VK_1_2;
        private bool m_spirv = false;
        private string m_outputDir = "";
        private string m_outputPath = "";
        private string m_inputFile = "";
        private List<string> m_includePaths = new List<string>();
        private string m_cmdString = "";
        private string m_shaderCompError = "";
        private string m_shaderCompOutput = "";

        private bool IsDirectXCompile()
        {
            return m_compiler == ShaderCompiler.DXC || m_compiler == ShaderCompiler.FXC;
        }

        private bool IsVulkanCompile()
        {
            return m_compiler == ShaderCompiler.GLSLC;
        }

        private string GetShaderCompilerStr()
        {
            switch(m_compiler)
            {
                case ShaderCompiler.DXC:
                    if (m_spirv)
                    {
                        string vkSdkPath = System.Environment.GetEnvironmentVariable("VULKAN_SDK");
                        if (vkSdkPath != null)
                        {
                            return vkSdkPath + "\\bin\\dxc.exe";
                        }
                        return "..\\..\\..\\DXC\\dxc.exe";
                    }
                    else
                    {
                        return "dxc.exe";
                    }
                case ShaderCompiler.FXC: return "fxc.exe";
                case ShaderCompiler.GLSLC: return "glslc.exe";
            }
            return "";
        }

        private string GetDefines()
        {
            if (IsDirectXCompile())
            {
                string defStr = "-DHLSL ";
                if (m_spirv)
                    defStr += "-DVK";
                else
                    defStr += "-DDX12";
                return defStr;
            }
            else
            {
                return "-DGLSL -DVK";
            }
        }

        private string GetDxShaderProfile()
        {
            switch (m_dxProfile)
            {
                case DXShaderProfile.SP_5_0: return "5_0";
                case DXShaderProfile.SP_5_1: return "5_1";
                case DXShaderProfile.SP_6_0: return "6_0";
                case DXShaderProfile.SP_6_1: return "6_1";
                case DXShaderProfile.SP_6_2: return "6_2";
                case DXShaderProfile.SP_6_3: return "6_3";
                case DXShaderProfile.SP_6_4: return "6_4";
                case DXShaderProfile.SP_6_5: return "6_5";
                case DXShaderProfile.SP_6_6: return "6_6";

            }
            return "";
        }

        private string GetVkTarget()
        {
            switch (m_vkTarget)
            {
                case VKTarget.VK_1_0: return "--target-env=vulkan1.0";
                case VKTarget.VK_1_1: return "--target-env=vulkan1.1";
                case VKTarget.VK_1_2: return "--target-env=vulkan1.2";
            }
            return "";
        }

        private string GetVkShaderProfile()
        {
            switch (m_vkProfile)
            {
                case VKShaderProfile.SP_1_0: return "--target-spv=spv1.0 " + GetVkTarget();
                case VKShaderProfile.SP_1_1: return "--target-spv=spv1.1 " + GetVkTarget();
                case VKShaderProfile.SP_1_2: return "--target-spv=spv1.2 " + GetVkTarget();
                case VKShaderProfile.SP_1_3: return "--target-spv=spv1.3 " + GetVkTarget();
                case VKShaderProfile.SP_1_4: return "--target-spv=spv1.4 " + GetVkTarget();
                case VKShaderProfile.SP_1_5: return "--target-spv=spv1.5 " + GetVkTarget();
            }
            return "";
        }

        private string GetShaderStageStr()
        {
            if (IsDirectXCompile())
            {
                switch (m_shaderStage)
                {
                    case ShaderStage.Vertex:    return "-T vs_" + GetDxShaderProfile();
                    case ShaderStage.Pixel:     return "-T ps_" + GetDxShaderProfile();
                    case ShaderStage.Geometry:  return "-T gs_" + GetDxShaderProfile();
                    case ShaderStage.Hull:      return "-T hs_" + GetDxShaderProfile();
                    case ShaderStage.Domain:    return "-T ds_" + GetDxShaderProfile();
                    case ShaderStage.Compute:   return "-T cs_" + GetDxShaderProfile();
                }
            }
            else if (IsVulkanCompile())
            {
                switch (m_shaderStage)
                {
                    case ShaderStage.Vertex:    return "-fshader-stage=vert " + GetVkShaderProfile();
                    case ShaderStage.Pixel:     return "-fshader-stage=frag " + GetVkShaderProfile();
                    case ShaderStage.Geometry:  return "-fshader-stage=geom " + GetVkShaderProfile();
                    case ShaderStage.Hull:      return "-fshader-stage=tesc " + GetVkShaderProfile();
                    case ShaderStage.Domain:    return "-fshader-stage=tese " + GetVkShaderProfile();
                    case ShaderStage.Compute:   return "-fshader-stage=comp " + GetVkShaderProfile();
                }
            }
            return "";
        }

        private string GetSpirvStr()
        {
            if (m_compiler == ShaderCompiler.DXC && m_spirv)
                return "-spirv -fvk-use-dx-layout";
            return "";
        }

        private string GetIncludePaths()
        {
            string paths = "";

            foreach (var dir in m_includePaths)
            {
                paths += "-I ";
                paths += dir;
            }

            return paths;
        }

        public void SetShaderStage(string shaderStage)
        {
            if (shaderStage.Equals(ShaderStage.Vertex.ToString(), StringComparison.OrdinalIgnoreCase))
            {
                m_shaderStage = ShaderStage.Vertex;
            }
            else if (shaderStage.Equals(ShaderStage.Pixel.ToString(), StringComparison.OrdinalIgnoreCase))
            {
                m_shaderStage = ShaderStage.Pixel;
            }
            else if (shaderStage.Equals(ShaderStage.Geometry.ToString(), StringComparison.OrdinalIgnoreCase))
            {
                m_shaderStage = ShaderStage.Geometry;
            }
            else if (shaderStage.Equals(ShaderStage.Hull.ToString(), StringComparison.OrdinalIgnoreCase))
            {
                m_shaderStage = ShaderStage.Hull;
            }
            else if (shaderStage.Equals(ShaderStage.Domain.ToString(), StringComparison.OrdinalIgnoreCase))
            {
                m_shaderStage = ShaderStage.Domain;
            }
            else if (shaderStage.Equals(ShaderStage.Compute.ToString(), StringComparison.OrdinalIgnoreCase))
            {
                m_shaderStage = ShaderStage.Compute;
            }
            else
            {
                System.Console.WriteLine("Unknown shader stage {0}", shaderStage);
            }

            //System.Console.WriteLine("Shader stage: {0}", m_shaderStage.ToString());
        }

        public void SetShaderCompiler(string compilerStr)
        {
            if (compilerStr.Equals("dxc", StringComparison.OrdinalIgnoreCase))
            {
                m_compiler = ShaderCompiler.DXC;
            }
            else if (compilerStr.Equals("fxc", StringComparison.OrdinalIgnoreCase))
            {
                m_compiler = ShaderCompiler.FXC;
            }
            else if (compilerStr.Equals("glslc", StringComparison.OrdinalIgnoreCase))
            {
                m_compiler = ShaderCompiler.GLSLC;
            }
            else
            {
                System.Console.WriteLine("Unknown shader compiler {0}", compilerStr);
            }

            //System.Console.WriteLine("Shader compiler: {0}", m_compiler.ToString());
        }

        public void SetShaderProfile(string profileStr)
        {
            switch(m_compiler)
            {
                case ShaderCompiler.DXC:
                    if (profileStr.Equals("5.0"))
                    {
                        m_dxProfile = DXShaderProfile.SP_5_0;
                    }
                    else if (profileStr.Equals("5.1"))
                    {
                        m_dxProfile = DXShaderProfile.SP_5_1;
                    }
                    else if (profileStr.Equals("6.0"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_0;
                    }
                    else if (profileStr.Equals("6.1"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_1;
                    }
                    else if (profileStr.Equals("6.2"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_2;
                    }
                    else if (profileStr.Equals("6.3"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_3;
                    }
                    else if (profileStr.Equals("6.4"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_4;
                    }
                    else if (profileStr.Equals("6.5"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_5;
                    }
                    else if (profileStr.Equals("6.6"))
                    {
                        m_dxProfile = DXShaderProfile.SP_6_6;
                    }
                    else
                    {
                        System.Console.WriteLine("Unknown shader profile (DXC) {0}", profileStr);
                    }
                    break;

                case ShaderCompiler.FXC:
                    if (profileStr.Equals("5.0"))
                    {
                        m_dxProfile = DXShaderProfile.SP_5_0;
                    }
                    else if (profileStr.Equals("5.1"))
                    {
                        m_dxProfile = DXShaderProfile.SP_5_1;
                    }
                    else
                    {
                        System.Console.WriteLine("Unknown shader profile (FXC) {0}", profileStr);
                    }
                    break;

                case ShaderCompiler.GLSLC:
                    if (profileStr.Equals("1.0"))
                    {
                        m_vkProfile = VKShaderProfile.SP_1_0;
                    }
                    else if (profileStr.Equals("1.1"))
                    {
                        m_vkProfile = VKShaderProfile.SP_1_1;
                    }
                    else if (profileStr.Equals("1.2"))
                    {
                        m_vkProfile = VKShaderProfile.SP_1_2;
                    }
                    else if (profileStr.Equals("1.3"))
                    {
                        m_vkProfile = VKShaderProfile.SP_1_3;
                    }
                    else if (profileStr.Equals("1.4"))
                    {
                        m_vkProfile = VKShaderProfile.SP_1_4;
                    }
                    else if (profileStr.Equals("1.5"))
                    {
                        m_vkProfile = VKShaderProfile.SP_1_5;
                    }
                    else
                    {
                        System.Console.WriteLine("Unknown shader profile (GLSLC) {0}", profileStr);
                    }
                    break;
            }
            
            //if (m_compiler == ShaderCompiler.GLSLC)
            //{
            //    System.Console.WriteLine("Shader profile (GLSLC): {0}", m_vkProfile.ToString());
            //}
            //else
            //{
            //    System.Console.WriteLine("Shader profile (DXC/FXC): {0}", m_dxProfile.ToString());
            //}
        }

        public void SetSpirv(bool spirv)
        {
            m_spirv = spirv;

            //System.Console.WriteLine("SPIR-V codegen: {0}", m_spirv.ToString());
        }

        public void SetOutputDir(DirectoryInfo outputDir)
        {
            m_outputDir = outputDir.FullName;

            //System.Console.WriteLine("Output dir: {0}", m_outputDir);
        }

        public void SetIncludePaths(DirectoryInfo[] includePaths)
        {
            foreach(var dir in includePaths)
            {
                m_includePaths.Add(dir.FullName);
                //System.Console.WriteLine("Include path: {0}", dir.FullName);
            }
        }

        public void SetInputFile(FileInfo inputFile)
        {
            m_inputFile = inputFile.FullName;

            //System.Console.WriteLine("Input file: {0}", m_inputFile);
        }

        public void ProcessPaths()
        {
            m_outputDir = Path.GetFullPath(m_outputDir);
            for (int i = 0; i < m_includePaths.Count; ++i)
            {
                m_includePaths[i] = Path.GetFullPath(m_includePaths[i]);
            }
            m_inputFile = Path.GetFullPath(m_inputFile);

            string outputExtension = (IsDirectXCompile() && !m_spirv) ? ".cso" : ".spv";
            m_outputPath = IsDirectXCompile() ? "-Fo " : "-o ";
            m_outputPath += m_outputDir + Path.GetFileNameWithoutExtension(m_inputFile) + outputExtension;
        }

        public void BuildCommand()
        {
            m_cmdString += GetDefines();
            m_cmdString += " ";
            //m_cmdString += "-Zi -Fd " + m_outputDir + " -Od ";
            m_cmdString += GetShaderStageStr();
            m_cmdString += " ";
            m_cmdString += GetSpirvStr();
            m_cmdString += " ";
            m_cmdString += GetIncludePaths();
            m_cmdString += " ";
            m_cmdString += m_outputPath;
            m_cmdString += " ";
            m_cmdString += m_inputFile;

            //System.Console.WriteLine("Full command: {0} {1}", GetShaderCompilerStr(), m_cmdString);
        }

        public int Execute()
        {
            var location = new Uri(Assembly.GetEntryAssembly().GetName().CodeBase);
            var fileInfo = new FileInfo(location.AbsolutePath).Directory;
            Directory.SetCurrentDirectory(fileInfo.FullName);

            ProcessStartInfo processInfo = new ProcessStartInfo(GetShaderCompilerStr());
            processInfo.CreateNoWindow = true;
            processInfo.WorkingDirectory = fileInfo.FullName;
            processInfo.Arguments = m_cmdString;
            processInfo.RedirectStandardError = true;
            processInfo.RedirectStandardOutput = true;
            processInfo.UseShellExecute = false;

            var process = new System.Diagnostics.Process();
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.WorkingDirectory = fileInfo.FullName;
            process.StartInfo.Arguments = m_cmdString;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.FileName = GetShaderCompilerStr();

            process.ErrorDataReceived += this.StdErrorHandler;
            process.OutputDataReceived += this.StdOutputHandler;

            process.Start();

            process.BeginErrorReadLine();
            process.BeginOutputReadLine();

            process.WaitForExit();

            if(m_shaderCompError.Length > 0)
            {
                Console.WriteLine("Shader Compilation had errors");
                Console.WriteLine("Full command: {0} {1}", GetShaderCompilerStr(), m_cmdString);
                Console.WriteLine("{0}", m_shaderCompError);
            }

            return process.ExitCode;
        }

        private void StdErrorHandler(object sender, DataReceivedEventArgs args)
        {
            string message = args.Data;
            
            if (message == null)
                return;

            if (message.Length > 0 && !message.Contains("warning: DXIL.dll not found."))
            {
                m_shaderCompError += message + "\n";
            }
        }

        private void StdOutputHandler(object sender, DataReceivedEventArgs args)
        {
            string message = args.Data;

            if (message == null)
                return;

            if (message.Length > 0)
            {
                m_shaderCompOutput += message + "\n";
            }
        }

        static int Run(string compiler, string shader_stage, string profile, bool spirv, DirectoryInfo output_dir, DirectoryInfo[] include_paths, FileInfo input_file)
        {
            var compilerMain = new Compiler();

            compilerMain.SetShaderCompiler(compiler);
            compilerMain.SetShaderStage(shader_stage);
            compilerMain.SetShaderProfile(profile);
            compilerMain.SetSpirv(spirv);
            compilerMain.SetOutputDir(output_dir);
            compilerMain.SetIncludePaths(include_paths);
            compilerMain.SetInputFile(input_file);

            compilerMain.ProcessPaths();
            compilerMain.BuildCommand();
            int ret = compilerMain.Execute();

            return ret;
        }

        static int Main(string[] args)
        {
            //string commandline = "";
            //for (int i = 0; i < args.Length; ++i)
            //{
            //    commandline += args[i] + " ";
            //}
            //Console.WriteLine(commandline);

            var rootCommand = new RootCommand 
            { 
                new Option("--compiler", "Compiler name string (dxc, fxc, glslc)") { Argument = new Argument<string>() },
                new Option("--shader_stage", "Shader stage") { Argument = new Argument<string>() },
                new Option("--profile", "Shader profile") { Argument = new Argument<string>() },
                new Option("--spirv", "SPIR-V code generation") { Argument = new Argument<bool>() },
                new Option("--output_dir", "Compiled shader output directory") { Argument = new Argument<DirectoryInfo>().ExistingOnly() },
                new Option("--include_paths", "Any additional include paths") { Argument = new Argument<DirectoryInfo[]>().ExistingOnly() },
                new Option("--input_file", "Shader input file") { Argument = new Argument<FileInfo>().ExistingOnly() }
            };

            rootCommand.Handler = CommandHandler.Create<string, string, string, bool, DirectoryInfo, DirectoryInfo[], FileInfo>(Run);

            // Parse the incoming args and invoke the handler
            return rootCommand.InvokeAsync(args).Result;
        }
    }
}
