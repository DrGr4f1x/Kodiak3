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

#include "CommandBufferPoolVk.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{

CommandBufferPool g_commandBufferPool;

} // namespace Kodiak


void CommandBufferPool::Create(VkQueue _queue, VkDevice device, uint32_t queueFamilyIndex)
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	m_queue = _queue;
	m_device = device;

	VkCommandPoolCreateInfo commandPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndex;

	ThrowIfFailed(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool));
}


void CommandBufferPool::Destroy()
{
	if (m_commandPool == VK_NULL_HANDLE)
	{
		return;
	}

	lock_guard<mutex> CS(m_commandBufferMutex);

	while (!m_readyCommandBuffers.empty())
	{
		pair<VkFence, VkCommandBuffer>& commandBufferPair = m_readyCommandBuffers.front();
		vkDestroyFence(m_device, commandBufferPair.first, nullptr);
		m_readyCommandBuffers.pop();
	}

	if (!m_commandBufferPool.empty())
	{
		vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBufferPool.size()), m_commandBufferPool.data());
		m_commandBufferPool.clear();
	}

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	m_commandPool = VK_NULL_HANDLE;
	m_device = VK_NULL_HANDLE;
}


VkCommandBuffer CommandBufferPool::RequestCommandBuffer()
{
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

	if (!m_readyCommandBuffers.empty())
	{
		pair<VkFence, VkCommandBuffer>& commandBufferPair = m_readyCommandBuffers.front();

		if (VK_SUCCESS == vkGetFenceStatus(m_device, commandBufferPair.first))
		{
			commandBuffer = commandBufferPair.second;
			ThrowIfFailed(vkResetCommandBuffer(commandBuffer, 0));
			vkDestroyFence(m_device, commandBufferPair.first, nullptr);
			m_readyCommandBuffers.pop();
		}
	}

	// If no command buffers were ready to be reused, create a new one
	if (commandBuffer == VK_NULL_HANDLE)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		ThrowIfFailed(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer));

		m_commandBufferPool.push_back(commandBuffer);
	}

	return commandBuffer;
}


void CommandBufferPool::DiscardCommandBuffer(VkFence fence, VkCommandBuffer commandBuffer)
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	// Fence indicates we are free to re-use the command buffer
	m_readyCommandBuffers.push(make_pair(fence, commandBuffer));
}