//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//
// Description:  This is a dynamic graphics memory allocator for DX12.  It's designed to work in concert
// with the CommandContext class and to do so in a thread-safe manner.  There may be many command contexts,
// each with its own linear allocators.  They act as windows into a global memory pool by reserving a
// context-local memory page.  Requesting a new page is done in a thread-safe manner by guarding accesses
// with a mutex lock.
//
// When a command context is finished, it will receive a fence ID that indicates when it's safe to reclaim
// used resources.  The CleanupUsedPages() method must be invoked at this time so that the used pages can be
// scheduled for reuse after the fence has cleared.

#pragma once

#include "GpuResource.h"

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256

namespace Kodiak
{

// Various types of allocations may contain NULL pointers.  Check before dereferencing if you are unsure.
struct DynAlloc
{
	DynAlloc(GpuResource& baseResource, size_t thisOffset, size_t thisSize)
		: buffer(baseResource)
		, offset(thisOffset)
		, size(thisSize) 
	{}

	GpuResource& buffer;	// The D3D buffer associated with this memory.
	size_t offset;			// Offset from start of buffer resource
	size_t size;			// Reserved size of this allocation
	void* dataPtr;			// The CPU-writeable address
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;	// The GPU-visible address
};


class LinearAllocationPage : public GpuResource
{
public:
	LinearAllocationPage(ID3D12Resource* resource, ResourceState usage) 
		: GpuResource()
	{
		m_resource.Attach(resource);
		m_usageState = usage;
		m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();
		m_resource->Map(0, nullptr, &m_cpuVirtualAddress);
	}


	~LinearAllocationPage()
	{
		Unmap();
	}


	void Map()
	{
		if (m_cpuVirtualAddress == nullptr)
		{
			m_resource->Map(0, nullptr, &m_cpuVirtualAddress);
		}
	}


	void Unmap()
	{
		if (m_cpuVirtualAddress != nullptr)
		{
			m_resource->Unmap(0, nullptr);
			m_cpuVirtualAddress = nullptr;
		}
	}


	void* m_cpuVirtualAddress{ nullptr };
	D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress;
};


enum LinearAllocatorType
{
	kInvalidAllocator = -1,

	kGpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
	kCpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

	kNumAllocatorTypes
};


enum
{
	kGpuAllocatorPageSize = 0x10000,	// 64K
	kCpuAllocatorPageSize = 0x200000	// 2MB
};


class LinearAllocatorPageManager
{
public:

	LinearAllocatorPageManager();
	LinearAllocationPage* RequestPage();
	LinearAllocationPage* CreateNewPage(size_t pageSize = 0);

	// Discarded pages will get recycled.  This is for fixed size pages.
	void DiscardPages(uint64_t fenceID, const std::vector<LinearAllocationPage*>& pages);

	// Freed pages will be destroyed once their fence has passed.  This is for single-use,
	// "large" pages.
	void FreeLargePages(uint64_t fenceID, const std::vector<LinearAllocationPage*>& pages);

	void Destroy() { m_pagePool.clear(); }

private:
	static LinearAllocatorType sm_autoType;

	LinearAllocatorType m_allocationType;
	std::vector<std::unique_ptr<LinearAllocationPage>> m_pagePool;
	std::queue<std::pair<uint64_t, LinearAllocationPage*>> m_retiredPages;
	std::queue<std::pair<uint64_t, LinearAllocationPage*>> m_deletionQueue;
	std::queue<LinearAllocationPage*> m_availablePages;
	std::mutex m_mutex;
};


class LinearAllocator
{
public:

	LinearAllocator(LinearAllocatorType type) 
		: m_allocationType(type)
		, m_pageSize(0)
		, m_curOffset(~(size_t)0)
		, m_curPage(nullptr)
	{
		assert(type > kInvalidAllocator && type < kNumAllocatorTypes);
		m_pageSize = (type == kGpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
	}


	DynAlloc Allocate(size_t sizeInBytes, size_t alignment = DEFAULT_ALIGN);

	void CleanupUsedPages(uint64_t fenceID);


	static void DestroyAll()
	{
		sm_pageManager[0].Destroy();
		sm_pageManager[1].Destroy();
	}

private:
	DynAlloc AllocateLargePage(size_t sizeInBytes);

	static LinearAllocatorPageManager sm_pageManager[2];

	LinearAllocatorType m_allocationType;
	size_t m_pageSize;
	size_t m_curOffset;
	LinearAllocationPage* m_curPage{ nullptr };
	std::vector<LinearAllocationPage*> m_retiredPages;
	std::vector<LinearAllocationPage*> m_largePageList;
};

} // namespace Kodiak