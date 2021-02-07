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

	ID3D12QueryHeap* GetQueryHeap() const { return m_heap.Get(); }

private:
	QueryHeapType m_type;
	uint32_t m_queryCount{ 0 };

	Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_heap;
};

} // namespace Kodiak