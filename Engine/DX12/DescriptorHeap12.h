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

// This is an unbounded resource descriptor allocator.  It is intended to provide space for CPU-visible resource descriptors
// as resources are created.  For those that need to be made shader-visible, they will need to be copied to a UserDescriptorHeap
// or a DynamicDescriptorHeap.
class DescriptorAllocator
{
public:
	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type) : m_type(type) {}

	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t count);

	static void DestroyAll();

protected:

	static const uint32_t sm_numDescriptorsPerHeap = 256;
	static std::mutex sm_allocationMutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_descriptorHeapPool;
	static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);

	D3D12_DESCRIPTOR_HEAP_TYPE m_type;
	ID3D12DescriptorHeap* m_currentHeap{ nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE m_currentHandle;
	uint32_t m_descriptorSize;
	uint32_t m_remainingFreeHandles;
};

} // namespace Kodiak