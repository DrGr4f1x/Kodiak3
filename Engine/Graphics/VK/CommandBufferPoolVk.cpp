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
#include "Graphics\GraphicsDevice.h"

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

	ThrowIfFailed(g_graphicsDevice->CreateCommandPool(queueFamilyIndex, &m_commandPool));
	SetDebugName(m_commandPool->Get(), "CommandBufferPool::m_commandPool");
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
		vkFreeCommandBuffers(device, m_commandPool->Get(), static_cast<uint32_t>(m_commandBufferPool.size()), m_commandBufferPool.data());
		m_commandBufferPool.clear();
	}
}


VkCommandBuffer CommandBufferPool::RequestCommandBuffer(uint64_t completedFenceValue)
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

	if (!m_readyCommandBuffers.empty())
	{
		pair<uint64_t, VkCommandBuffer>& commandBufferPair = m_readyCommandBuffers.front();

		if (commandBufferPair.first <= completedFenceValue)
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
		allocInfo.commandPool = m_commandPool->Get();
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


void CommandBufferPool::DiscardCommandBuffer(uint64_t fenceValue, VkCommandBuffer commandBuffer)
{
	lock_guard<mutex> CS(m_commandBufferMutex);

	// Fence indicates we are free to re-use the command buffer
	m_readyCommandBuffers.push(make_pair(fenceValue, commandBuffer));
}