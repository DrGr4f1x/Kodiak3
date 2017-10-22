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

#include "DescriptorHeap12.h"

#include "GraphicsDevice12.h"


using namespace Kodiak;
using namespace std;


mutex DescriptorAllocator::sm_allocationMutex;
vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sm_descriptorHeapPool;


DescriptorAllocator Kodiak::g_descriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
};


void DescriptorAllocator::DestroyAll()
{
	lock_guard<mutex> lockGuard(sm_allocationMutex);

	sm_descriptorHeapPool.clear();
}


ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	std::lock_guard<std::mutex> LockGuard(sm_allocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = sm_numDescriptorsPerHeap;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
	assert_succeeded(GetDevice()->CreateDescriptorHeap(&desc, MY_IID_PPV_ARGS(&pHeap)));
	sm_descriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}


D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t count)
{
	if (m_currentHeap == nullptr || m_remainingFreeHandles < count)
	{
		m_currentHeap = RequestNewHeap(m_type);
		m_currentHandle = m_currentHeap->GetCPUDescriptorHandleForHeapStart();
		m_remainingFreeHandles = sm_numDescriptorsPerHeap;

		if (m_descriptorSize == 0)
		{
			m_descriptorSize = GetDevice()->GetDescriptorHandleIncrementSize(m_type);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ret = m_currentHandle;
	m_currentHandle.ptr += count * m_descriptorSize;
	m_remainingFreeHandles -= count;
	return ret;
}