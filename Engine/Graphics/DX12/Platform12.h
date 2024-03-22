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

#define FORCE_D3D12_VALIDATION 0
#define ENABLE_D3D12_VALIDATION (_DEBUG || FORCE_D3D12_VALIDATION)

#define FORCE_D3D12_DEBUG_MARKERS 1
#define ENABLE_D3D12_DEBUG_MARKERS (_DEBUG || _PROFILE || FORCE_D3D12_DEBUG_MARKERS || ENABLE_D3D12_VALIDATION)

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <wincodec.h>

#include "d3dx12.h"

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "Extern\D3D12MemoryAllocator\D3D12MemAlloc.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

const std::string s_apiName = "DirectX 12";
const std::string s_apiPrefixString = "[DirectX 12]";
const std::string s_defaultShaderPath = "Shaders\\DXIL";

namespace Kodiak
{

static const uint32_t NumSwapChainBuffers = 3;

void SetDebugName(ID3D12Object* object, const std::string& name);

} // namespace Kodiak