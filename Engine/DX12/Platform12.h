//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#include <d3d12.h>
#include <d3dcompiler.h>
#include <comdef.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <pix.h>
#include <wincodec.h>

#include "d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define DECLARE_COM_PTR(type) _COM_SMARTPTR_TYPEDEF(type, __uuidof(type))

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

const std::string s_apiPrefixString = "[DirectX 12]";

const std::string s_defaultShaderPath = "Shaders\\DXIL";

namespace Kodiak
{

DECLARE_COM_PTR(ID3D12Device);
DECLARE_COM_PTR(ID3D12Resource);
DECLARE_COM_PTR(IUnknown);

using PlatformHandle = IUnknownPtr;
using ResourceHandle = ID3D12ResourcePtr;

} // namespace Kodiak