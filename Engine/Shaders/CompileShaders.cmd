setlocal
set error=0

set ShaderTarget=%1
set ShaderConfig=%2
set Optimize=%3
set IncludeDir=%~4
set OutputDir=%~5

echo CompileShaders %ShaderTarget% %ShaderConfig% %Optimize% %IncludeDir% %OutputDir%

if %ShaderTarget%.==dxil. goto continuedxil
if %ShaderTarget%.==spirv. goto continuespirv
echo usage: CompileShaders [dxil]
exit /b

:continuedxil
echo Compiling to DXIL
set DXCOPTS=-DHLSL -DDX12 -I %IncludeDir%
set PCDXC="%WindowsSdkVerBinPath%x86\dxc.exe"
if exist %PCDXC% goto continue
set PCDXC="%WindowsSdkBinPath%%WindowsSDKVersion%\x86\dxc.exe"
if exist %PCDXC% goto continue

set PCDXC=dxc.exe
goto continue

:continuespirv
echo Compiling to SPIR-V
set DXCOPTS=-DHLSL -DVK -spirv -fvk-use-dx-layout -I %IncludeDir%
set PCDXC="%VULKAN_SDK%\bin\dxc.exe"
if exist %PCDXC% goto continue

echo ERROR: Vulkan SDK not found
exit /b

:continue
@if not exist OutputDir mkdir OutputDir
call :CompileShader%1 UIVS vs main
call :CompileShader%1 UIPS ps main
call :CompileShader%1 GridVS vs main
call :CompileShader%1 GridPS ps main

echo.

if %error% == 0 (
    echo Shaders compiled ok
) else (
    echo There were shader compilation errors!
)

endlocal
exit /b

:CompileShaderdxil
set dxc=%PCDXC% %1.hlsl %DXCOPTS% /T%2_6_0 /E%3 /Fo %OutputDir%%1.cso
echo.
echo %dxc%
%dxc% || set error=1
exit /b

:CompileShaderspirv
set dxc=%PCDXC% %1.hlsl %DXCOPTS% /T%2_6_0 /E%3 /Fo %OutputDir%%1.spv
echo.
echo %dxc%
%dxc% || set error=1
exit /b