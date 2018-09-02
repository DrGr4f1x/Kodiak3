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
#include "DX12\GpuBuffer12.h"
#elif defined(VK)
#include "VK\GpuBufferVk.h"
#else
#error "No graphics API defined"
#endif

#if 0

#include "GpuResource.h"
#include "ResourceView.h"

namespace Kodiak
{

class GpuBuffer : public GpuBufferResource
{
public:
	~GpuBuffer();

	size_t GetSize() const { return m_bufferSize; }
	size_t GetElementCount() const { return m_elementCount; }
	size_t GetElementSize() const { return m_elementSize; }

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr);

protected:
	GpuBuffer()
	{
		m_usageState = ResourceState::Common;
	}
	virtual void PlatformCreate() = 0;

protected:
	size_t m_bufferSize{ 0 };
	size_t m_elementCount{ 0 };
	size_t m_elementSize{ 0 };
};

} // namespace Kodiak

#endif