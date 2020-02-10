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

// Windows headers
#include <windows.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <ppl.h>
#include <ppltasks.h>

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}

// Graphics API headers
#if defined(DX12)
#include "DX12\Platform12.h"
#elif defined(VK)
#include "VK\PlatformVk.h"
#else
#error No graphics API defined!
#endif


// Standard library headers
#include <array>
#include <queue>
#include <string>


// Engine headers
#include "BitmaskEnum.h"
#include "GraphicsEnums.h"
#include "Math\CommonMath.h"
#include "NonCopyable.h"
#include "Utility.h"
#include "VectorMath.h"