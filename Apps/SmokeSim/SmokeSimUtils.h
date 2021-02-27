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

struct GridVertex
{
	Math::Vector3 position;
	Math::Vector3 texcoord;
};

void ComputeFlattened3DTextureDims(uint32_t depth, uint32_t& outRows, uint32_t& outCols);