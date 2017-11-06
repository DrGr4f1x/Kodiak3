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

class CommandBufferPool
{
public:
	CommandBufferPool() = default;
	~CommandBufferPool() = default;

	void Create(VkQueue queue, VkDevice device, uint32_t queueFamilyIndex);
	void Destroy();

	VkQueue GetQueue() { return m_queue; }
	VkDevice GetDevice() { return m_device; }

	VkCommandBuffer RequestCommandBuffer();
	void DiscardCommandBuffer(VkFence fence, VkCommandBuffer commandBuffer);

	inline size_t Size() { return m_commandBufferPool.size(); }

	// TODO - Probably not thread-safe.  Think about how to handle submissions from multiple threads.
	VkSemaphore GetCurWaitSemaphore() { return m_curWaitSemaphore; }
	void SetCurWaitSemaphore(VkSemaphore waitSemaphore) { m_curWaitSemaphore = waitSemaphore; }
	
private:
	VkCommandPool	m_commandPool{ VK_NULL_HANDLE };
	VkQueue			m_queue{ VK_NULL_HANDLE };
	VkDevice		m_device{ VK_NULL_HANDLE };

	std::vector<VkCommandBuffer> m_commandBufferPool;
	std::queue<std::pair<VkFence, VkCommandBuffer>> m_readyCommandBuffers;
	std::mutex m_commandBufferMutex;

	VkSemaphore m_curWaitSemaphore{ VK_NULL_HANDLE };
};

extern CommandBufferPool g_commandBufferPool;

} // namespace Kodiak