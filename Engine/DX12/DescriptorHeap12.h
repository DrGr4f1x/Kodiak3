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
	D3D12_CPU_DESCRIPTOR_HANDLE m_currentHandle{ D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN };
	uint32_t m_descriptorSize{ 0 };
	uint32_t m_remainingFreeHandles{ 0 };
};


class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		m_cpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_gpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		: m_cpuHandle(cpuHandle)
	{
		m_gpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
		: m_cpuHandle(cpuHandle)
		, m_gpuHandle(gpuHandle)
	{}

	DescriptorHandle operator+(int32_t offsetScaledByDescriptorSize) const
	{
		DescriptorHandle ret = *this;
		ret += offsetScaledByDescriptorSize;
		return ret;
	}

	void operator+=(int32_t offsetScaledByDescriptorSize)
	{
		if (m_cpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_cpuHandle.ptr += offsetScaledByDescriptorSize;
		}

		if (m_gpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_gpuHandle.ptr += offsetScaledByDescriptorSize;
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_cpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_gpuHandle; }

	bool IsNull() const { return m_cpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
	bool IsShaderVisible() const { return m_gpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
};


class UserDescriptorHeap
{
public:

	UserDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount)
	{
		m_heapDesc.Type = type;
		m_heapDesc.NumDescriptors = maxCount;
		m_heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_heapDesc.NodeMask = 1;
	}

	void Create(const std::string& debugHeapName);
	void Destroy();

	bool HasAvailableSpace(uint32_t count) const { return count <= m_numFreeDescriptors; }
	DescriptorHandle Alloc(uint32_t count = 1);

	DescriptorHandle GetHandleAtOffset(uint32_t offset) const { return m_firstHandle + offset * m_descriptorSize; }

	bool ValidateHandle(const DescriptorHandle& dhandle) const;

	ID3D12DescriptorHeap* GetHeapPointer() const { return m_heap.Get(); }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	D3D12_DESCRIPTOR_HEAP_DESC m_heapDesc;
	uint32_t m_descriptorSize;
	uint32_t m_numFreeDescriptors;
	DescriptorHandle m_firstHandle;
	DescriptorHandle m_nextFreeHandle;
};

extern DescriptorAllocator g_descriptorAllocator[];
inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1)
{
	return g_descriptorAllocator[type].Allocate(count);
}

extern UserDescriptorHeap g_userDescriptorHeap[];
inline DescriptorHandle AllocateUserDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1)
{
	return g_userDescriptorHeap[type].Alloc(count);
}

} // namespace Kodiak