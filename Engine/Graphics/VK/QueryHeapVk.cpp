//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "Graphics\QueryHeap.h"

#include "Graphics\GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void OcclusionQueryHeap::Create(uint32_t queryCount)
{
	m_type = QueryHeapType::Occlusion;
	m_queryCount = queryCount;

	VkQueryPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.queryCount = m_queryCount;
	info.queryType = VK_QUERY_TYPE_OCCLUSION;
	info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FLAG_BITS_MAX_ENUM;

	VkQueryPool pool{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateQueryPool(GetDevice(), &info, nullptr, &pool));
	m_handle = QueryHeapHandle::Create(pool);
}