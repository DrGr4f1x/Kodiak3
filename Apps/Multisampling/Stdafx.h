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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _ENABLE_EXTENDED_ALIGNED_STORAGE

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
#include "DX12\Platform12.h"
#elif defined(VK)
#include "VK\PlatformVk.h"
#else
#error No graphics API defined!
#endif


#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
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
#include "BitmaskEnum.h"
#include "GraphicsEnums.h"
#include "Math\CommonMath.h"
#include "NonCopyable.h"
#include "Utility.h"
#include "VectorMath.h"