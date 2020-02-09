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
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <ppl.h>
#include <ppltasks.h>

#define USE_XINPUT
#include <XInput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#define USE_KEYBOARD_MOUSE
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}

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


// Engine headers
#include "BitmaskEnum.h"
#include "GraphicsEnums.h"
#include "Math\CommonMath.h"
#include "NonCopyable.h"
#include "Utility.h"
#include "VectorMath.h"