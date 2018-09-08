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

#include "CommandListManagerVk.h"
#include "GraphicsDevice.h"

#include <sstream>


using namespace Kodiak;
using namespace std;


CommandBufferPool::CommandBufferPool(CommandListType type)
	: m_commandListType(type)
{}


CommandBufferPool::~CommandBufferPool()
{
	Destroy();
}


void CommandBufferPool::Create(uint32_t queueFamilyIndex)
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	VkCommandPoolCreateInfo commandPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndex;

	ThrowIfFailed(vkCreateCommandPool(GetDevice(), &commandPoolInfo, nullptr, &m_commandPool));
	SetDebugName(m_commandPool, "CommandBufferPool::m_commandPool");
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
		m_readyCommandBuffers.pop();
	}

	auto device = GetDevice();

	if (!m_commandBufferPool.empty())
	{
		vkFreeCommandBuffers(device, m_commandPool, static_cast<uint32_t>(m_commandBufferPool.size()), m_commandBufferPool.data());
		m_commandBufferPool.clear();
	}

	vkDestroyCommandPool(device, m_commandPool, nullptr);
	m_commandPool = VK_NULL_HANDLE;
}


VkCommandBuffer CommandBufferPool::RequestCommandBuffer()
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

	if (!m_readyCommandBuffers.empty())
	{
		pair<shared_ptr<Fence>, VkCommandBuffer>& commandBufferPair = m_readyCommandBuffers.front();

		if (commandBufferPair.first->IsComplete())
		{
			commandBuffer = commandBufferPair.second;
			ThrowIfFailed(vkResetCommandBuffer(commandBuffer, 0));
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

		ThrowIfFailed(vkAllocateCommandBuffers(GetDevice(), &allocInfo, &commandBuffer));
		stringstream sstr;
		sstr << "CommandBuffer " << m_commandBufferPool.size();
	
		SetDebugName(commandBuffer, sstr.str());

		m_commandBufferPool.push_back(commandBuffer);
	}

	return commandBuffer;
}


void CommandBufferPool::DiscardCommandBuffer(shared_ptr<Fence> fence, VkCommandBuffer commandBuffer)
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	// Fence indicates we are free to re-use the command buffer
	m_readyCommandBuffers.push(make_pair(fence, commandBuffer));
}