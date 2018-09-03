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

// COM types
DECLARE_COM_PTR(IUnknown);

// DXGI types
DECLARE_COM_PTR(IDXGIFactory4);
DECLARE_COM_PTR(IDXGISwapChain1);

// DirectX 12 types
DECLARE_COM_PTR(ID3D12Device);
DECLARE_COM_PTR(ID3D12Resource);

#if _DEBUG
DECLARE_COM_PTR(ID3D12Debug);
#endif

using PlatformHandle = IUnknownPtr;
using ResourceHandle = ID3D12ResourcePtr;
using DeviceHandle = ID3D12DevicePtr;
using SwapChainHandle = IDXGISwapChain1Ptr;

using SrvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using UavHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using VbvHandle = D3D12_VERTEX_BUFFER_VIEW;
using IbvHandle = D3D12_INDEX_BUFFER_VIEW;
using CbvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;

using DsvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using RtvHandle = D3D12_CPU_DESCRIPTOR_HANDLE;

static const uint32_t NumSwapChainBuffers = 3;

} // namespace Kodiak