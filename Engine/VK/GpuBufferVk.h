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


#include "GpuResourceVk.h"


namespace Kodiak
{

class GpuBuffer : public GpuResource
{
public:
	void Destroy();

	VkBuffer GetBuffer() { return m_buffer; }
	const VkBuffer GetBuffer() const { return m_buffer; }

	size_t GetSize() const { return m_bufferSize; }
	uint32_t GetElementCount() const { return m_elementCount; }
	uint32_t GetElementSize() const { return m_elementSize; }

protected:
	void CreateBuffer(const std::string& name, uint32_t numElements, uint32_t elementSize, VkBufferUsageFlagBits flags, bool bHostMappable, const void* initialData);

protected:
	size_t		m_bufferSize{ 0 };
	uint32_t	m_elementCount{ 0 };
	uint32_t	m_elementSize{ 0 };

	VkBuffer		m_buffer{ VK_NULL_HANDLE };
	VkDescriptorBufferInfo m_descriptorInfo;
};

} // namespace Kodiak