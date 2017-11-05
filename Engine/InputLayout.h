//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

struct VertexStreamDesc
{
	uint32_t inputSlot;
	uint32_t stride;
	InputClassification inputClassification;
};

struct VertexElementDesc
{
	const char*				semanticName;
	uint32_t				semanticIndex;
	Format					format;
	uint32_t				inputSlot;		// Which input vertex stream we're coming from
	uint32_t				alignedByteOffset;
	InputClassification		inputClassification;
	uint32_t				instanceDataStepRate;
};

} // namespace Kodiak