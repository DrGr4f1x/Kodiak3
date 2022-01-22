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

#if defined(_MSVC_LANG)
#define KODIAK_CPLUSPLUS _MSVC_LANG
#else
#define KODIAK_CPLUSPLUS __cplusplus
#endif

#if 201703L < KODIAK_CPLUSPLUS
#define KODIAK_CPP_VERSION 20
#else
#error "Kodiak needs at least C++ standard version 20"
#endif

#if (17 <= KODIAK_CPP_VERSION)
#define KODIAK_NODISCARD [[nodiscard]]
#else
#define KODIAK_NODISCARD
#endif

// Windows headers
#include <windows.h>
#include <wrl.h>
#include <Shlwapi.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <ppl.h>
#include <ppltasks.h>
#include <comdef.h>

#pragma comment(lib, "shlwapi.lib")

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


// Standard library headers
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <shared_mutex>
#include <span>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

// Additional headers
#define FMT_HEADER_ONLY 1
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/format-inl.h>

// Engine headers
#include "BitmaskEnum.h"
#include "Log.h"
#include "NonCopyable.h"
#include "Utility.h"
#include "VectorMath.h"


// Graphics API headers
#if defined(DX12)
#include "Graphics\DX12\Platform12.h"
#elif defined(VK)
#include "Graphics\VK\PlatformVk.h"
#else
#error No graphics API defined!
#endif


// Engine\Graphics headers
#include "Graphics\GraphicsEnums.h"


// Engine info
#define KODIAK_MAKE_VERSION(major, minor, patch) \
    (((major) << 22) | ((minor) << 12) | (patch))
#define KODIAK_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define KODIAK_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define KODIAK_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

constexpr uint32_t s_engineMajorVersion = 3;
constexpr uint32_t s_engineMinorVersion = 0;
constexpr uint32_t s_enginePatchVersion = 0;

constexpr uint32_t s_engineVersion = KODIAK_MAKE_VERSION(s_engineMajorVersion, s_engineMinorVersion, s_enginePatchVersion);
static const std::string s_engineVersionStr
{
	std::to_string(s_engineMajorVersion) + "." + std::to_string(s_engineMinorVersion) + "." + std::to_string(s_enginePatchVersion)
};

static const std::string s_engineName{ "Kodiak" };