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

using PlatformHandle = Microsoft::WRL::ComPtr<IUnknown>;
using ResourceHandle = Microsoft::WRL::ComPtr<ID3D12Resource>;
using SwapChainHandle = Microsoft::WRL::ComPtr<IDXGISwapChain1>;
using PsoHandle = ID3D12PipelineState*;
using QueryHeapHandle = Microsoft::WRL::ComPtr<ID3D12QueryHeap>;

using SrvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using UavHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using VbvHandle = D3D12_VERTEX_BUFFER_VIEW;
using IbvHandle = D3D12_INDEX_BUFFER_VIEW;
using CbvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;

using DsvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using RtvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using FboHandle = void*;

static const uint32_t NumSwapChainBuffers = 3;

} // namespace Kodiak