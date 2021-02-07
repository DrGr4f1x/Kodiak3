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

namespace Kodiak
{

class OcclusionQueryHeap
{
public:

	void Create(uint32_t queryCount);

	QueryHeapType GetType() const { return m_type; }
	uint32_t GetQueryCount() const { return m_queryCount; }

	VkQueryPool GetQueryPool() const { return m_pool->Get(); }

private:
	QueryHeapType m_type;
	uint32_t m_queryCount{ 0 };

	Microsoft::WRL::ComPtr<UVkQueryPool> m_pool;
};

} // namespace Kodiak