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

class GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	GpuResource() = default;
	GpuResource(VkDeviceMemory memory);

	void Destroy();

	VkDeviceMemory operator->() { return m_deviceMemory; }
	const VkDeviceMemory operator->() const { return m_deviceMemory; }

	VkDeviceMemory GetResource() { return m_deviceMemory; }
	const VkDeviceMemory GetResource() const { return m_deviceMemory; }

protected:
	VkDeviceMemory m_deviceMemory{ VK_NULL_HANDLE };
};

} // namespace Kodiak