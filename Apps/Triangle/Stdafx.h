#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Windows
#include <windows.h>
#include <wrl.h>
#include <Shlwapi.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <ppl.h>
#include <ppltasks.h>

#pragma comment(lib, "shlwapi.lib")

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}

// DirectX common
#if defined(DX12)
#include <D3Dcompiler.h>
#include <dxgi1_4.h>
#include <d2d1_3.h>
#include <dwrite_2.h>
#include <wincodec.h>
#define MY_IID_PPV_ARGS IID_PPV_ARGS
#endif

// Graphics APIs
#if defined(DX12)
#include <d3d12.h>
#include <d3d11on12.h>
#include <pix.h>
#include "d3dx12.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#elif defined(VK)

#define FORCE_VULKAN_VALIDATION 0
#define ENABLE_VULKAN_VALIDATION (_DEBUG || FORCE_VULKAN_VALIDATION)

#define FORCE_VULKAN_DEBUG_MARKUP 0
#define ENABLE_VULKAN_DEBUG_MARKUP (_DEBUG || _PROFILE || FORCE_VULKAN_DEBUG_MARKUP)

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>
#pragma comment(lib, "vulkan-1.lib")
inline void ThrowIfFailed(VkResult res)
{
	if (res != VK_SUCCESS)
	{
		throw;
	}
}
#else
#error No graphics API defined!
#endif


#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>


inline std::wstring MakeWStr(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}


inline std::string MakeStr(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}


// Engine headers
#include "Math\CommonMath.h"
#include "NonCopyable.h"
#include "Utility.h"
#include "VectorMath.h"