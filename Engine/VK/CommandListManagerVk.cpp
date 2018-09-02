// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "CommandListManagerVk.h"

#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;

namespace Kodiak
{
CommandListManager g_commandManager;
} // namespace Kodiak


Fence::Fence(VkFence fence, CommandListType type)
	: m_fence(fence)
	, m_type(type)
{}


Fence::~Fence()
{
	g_commandManager.RetireFence(m_fence);
}


bool Fence::IsComplete() const
{
	return VK_SUCCESS == vkGetFenceStatus(*GetDevice(), m_fence);
}

CommandQueue::CommandQueue(CommandListType type)
	: m_type(type)
	, m_commandBufferPool(type)
{}


CommandQueue::~CommandQueue()
{
	Destroy();
}


void CommandQueue::SetWaitSemaphore(VkSemaphore waitSemaphore)
{
	m_waitSemaphore = waitSemaphore;
}


void CommandQueue::Create(uint32_t queueFamilyIndex, VkQueue queue)
{
	assert(!IsReady());

	m_queue = queue;

	m_commandBufferPool.Create(queueFamilyIndex);
}


void CommandQueue::Destroy()
{
	if (m_queue == VK_NULL_HANDLE)
	{
		return;
	}

	m_queue = VK_NULL_HANDLE;

	m_commandBufferPool.Destroy();

	m_waitSemaphore = VK_NULL_HANDLE;
}


bool CommandQueue::IsFenceComplete(shared_ptr<Fence> fence)
{
	return fence->IsComplete();
}


void CommandQueue::WaitForFence(shared_ptr<Fence> fence)
{
	VkDevice device = *GetDevice();
	VkResult res;
	do
	{
		res = vkWaitForFences(device, 1, &fence->m_fence, VK_TRUE, 100000);
	} while (res == VK_TIMEOUT);

	assert(res == VK_SUCCESS);
}


void CommandQueue::WaitForIdle()
{
	ThrowIfFailed(vkQueueWaitIdle(m_queue));
}


shared_ptr<Fence> CommandQueue::ExecuteCommandList(VkCommandBuffer cmdList, VkSemaphore signalSemaphore)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdList;
	if (m_waitSemaphore != VK_NULL_HANDLE)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_waitSemaphore;
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitStageMask;
	}
	if (signalSemaphore != VK_NULL_HANDLE)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
	}


	VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = 0;
	VkFence fence;
	ThrowIfFailed(vkCreateFence(*GetDevice(), &fenceInfo, nullptr, &fence));

	ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, fence));

	m_waitSemaphore = signalSemaphore;

	return make_shared<Fence>(fence, m_type);
}


VkCommandBuffer CommandQueue::RequestCommandBuffer()
{
	return m_commandBufferPool.RequestCommandBuffer();
}


void CommandQueue::DiscardCommandBuffer(shared_ptr<Fence> fence, VkCommandBuffer commandBuffer)
{
	m_commandBufferPool.DiscardCommandBuffer(fence, commandBuffer);
}


CommandListManager::CommandListManager()
	: m_graphicsQueue(CommandListType::Direct)
	, m_computeQueue(CommandListType::Compute)
	, m_copyQueue(CommandListType::Copy)
{}


CommandListManager::~CommandListManager()
{
	Destroy();
}


void CommandListManager::BeginFrame(VkSemaphore waitSemaphore)
{
	m_graphicsQueue.SetWaitSemaphore(waitSemaphore);
}


void CommandListManager::Create(uint32_t graphicsQueueIndex, VkQueue graphicsQueue, uint32_t computeQueueIndex, VkQueue computeQueue, uint32_t copyQueueIndex, VkQueue copyQueue)
{
	m_graphicsQueue.Create(graphicsQueueIndex, graphicsQueue);
	m_computeQueue.Create(computeQueueIndex, computeQueue);
	m_copyQueue.Create(copyQueueIndex, copyQueue);
}


void CommandListManager::Destroy()
{
	m_graphicsQueue.Destroy();
	m_computeQueue.Destroy();
	m_copyQueue.Destroy();

	lock_guard<mutex> CS(m_fenceMutex);

	VkDevice device = *GetDevice();

	auto it = begin(m_retiredFences);
	while (it != end(m_retiredFences))
	{
		auto fence = *it;
		vkDestroyFence(device, fence, nullptr);
		it = m_retiredFences.erase(it);
	}
}


void CommandListManager::WaitForFence(shared_ptr<Fence> fence)
{
	CommandQueue& producer = g_commandManager.GetQueue(fence->m_type);
	producer.WaitForFence(fence);
}


void CommandListManager::IdleGPU()
{
	m_graphicsQueue.WaitForIdle();
	m_computeQueue.WaitForIdle();
	m_copyQueue.WaitForIdle();
}


void CommandListManager::RetireFence(VkFence fence)
{
	lock_guard<mutex> CS(m_fenceMutex);

	m_retiredFences.push_back(fence);
}


void CommandListManager::DestroyRetiredFences()
{
	lock_guard<mutex> CS(m_fenceMutex);

	VkDevice device = *GetDevice();

	auto it = begin(m_retiredFences);
	while (it != end(m_retiredFences))
	{
		auto fence = *it;
		if (VK_SUCCESS == vkGetFenceStatus(device, fence))
		{
			vkDestroyFence(device, fence, nullptr);
			it = m_retiredFences.erase(it);
		}
		else
		{
			++it;
		}
	}
}