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

#if defined(DX12)
#include "DX12\GraphicsEnums12.h"
#elif defined(VK)
#include "VK\GraphicsEnumsVk.h"
#else
#error "No graphics API defined"
#endif

namespace Kodiak
{

struct Defaults
{
	static const uint32_t DepthBias;
	static const float DepthBiasClamp;
	static const float SlopeScaledDepthBias;
	static const byte StencilReadMask;
	static const byte StencilWriteMask;
	static const float Float32Max;
};

} // namespace Kodiak