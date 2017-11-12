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

// Forward declarations
class Fence;


class CommandBufferPool
{
public:
	CommandBufferPool(CommandListType type);
	~CommandBufferPool();

	void Create(uint32_t queueFamilyIndex);
	void Destroy();

	VkCommandBuffer RequestCommandBuffer();
	void DiscardCommandBuffer(std::shared_ptr<Fence> fence, VkCommandBuffer commandBuffer);

	inline size_t Size() { return m_commandBufferPool.size(); }

private:
	const CommandListType m_commandListType;

	VkCommandPool m_commandPool{ VK_NULL_HANDLE };

	std::vector<VkCommandBuffer> m_commandBufferPool;
	std::queue<std::pair<std::shared_ptr<Fence>, VkCommandBuffer>> m_readyCommandBuffers;
	std::mutex m_commandBufferMutex;
};

} // namespace Kodiak