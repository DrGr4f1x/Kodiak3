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


CommandQueue::CommandQueue(CommandListType type)
	: m_type(type)
	, m_nextFenceValue((uint64_t)type << 56 | 1)
	, m_lastCompletedFenceValue((uint64_t)type << 56)
	, m_commandBufferPool(type)
{}


CommandQueue::~CommandQueue()
{
	Destroy();
}


void CommandQueue::Create()
{
	assert(!IsReady());

	uint32_t queueFamilyIndex = g_graphicsDevice->GetQueueFamilyIndex(m_type);
	
	vkGetDeviceQueue(GetDevice(), queueFamilyIndex, 0, &m_queue);

	m_commandBufferPool.Create(queueFamilyIndex);

	// Create the timeline semaphore object
	VkSemaphoreTypeCreateInfo timelineCreateInfo;
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	timelineCreateInfo.pNext = nullptr;
	timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	timelineCreateInfo.initialValue = m_lastCompletedFenceValue;

	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = &timelineCreateInfo;
	createInfo.flags = 0;

	vkCreateSemaphore(GetDevice(), &createInfo, nullptr, &m_timelineSemaphore);
}


void CommandQueue::Destroy()
{
	if (m_queue == VK_NULL_HANDLE)
	{
		return;
	}

	m_queue = VK_NULL_HANDLE;

	m_commandBufferPool.Destroy();

	// Destroy the timeline semaphore object
	vkDestroySemaphore(GetDevice(), m_timelineSemaphore, nullptr);
	m_timelineSemaphore = VK_NULL_HANDLE;
}


uint64_t CommandQueue::IncrementFence()
{
	lock_guard<mutex> lockGuard(m_fenceMutex);

	// Have the queue signal the timeline semaphore
	VkTimelineSemaphoreSubmitInfo timelineInfo;
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.pNext = nullptr;
	timelineInfo.waitSemaphoreValueCount = 0;
	timelineInfo.pWaitSemaphoreValues = nullptr;
	timelineInfo.signalSemaphoreValueCount = 1;
	timelineInfo.pSignalSemaphoreValues = &m_nextFenceValue;

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = &timelineInfo;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_timelineSemaphore;
	submitInfo.commandBufferCount = 0;
	submitInfo.pCommandBuffers = 0;

	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);

	return m_nextFenceValue++;
}


bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	// Avoid querying the fence value by testing against the last one seen.
	// The max() is to protect against an unlikely race condition that could cause the last
	// completed fence value to regress.
	if (fenceValue > m_lastCompletedFenceValue)
	{
		uint64_t semaphoreCounterValue;
		ThrowIfFailed(vkGetSemaphoreCounterValue(GetDevice(), m_timelineSemaphore, &semaphoreCounterValue));
		m_lastCompletedFenceValue = std::max(m_lastCompletedFenceValue, semaphoreCounterValue);
	}

	return fenceValue <= m_lastCompletedFenceValue;
}


void CommandQueue::StallForFence(uint64_t fenceValue)
{
	CommandQueue& producer = g_commandManager.GetQueue((CommandListType)(fenceValue >> 56));

	VkSemaphoreWaitInfo waitInfo;
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.pNext = nullptr;
	waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &producer.m_timelineSemaphore;
	waitInfo.pValues = &fenceValue;

	ThrowIfFailed(vkWaitSemaphores(GetDevice(), &waitInfo, UINT64_MAX));
}


void CommandQueue::StallForProducer(CommandQueue& producer)
{
	assert(producer.m_nextFenceValue > 0);
	
	uint64_t waitValue = producer.m_nextFenceValue - 1;

	VkSemaphoreWaitInfo waitInfo;
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.pNext = nullptr;
	waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &producer.m_timelineSemaphore;
	waitInfo.pValues = &waitValue;

	ThrowIfFailed(vkWaitSemaphores(GetDevice(), &waitInfo, UINT64_MAX));
}


void CommandQueue::WaitForFence(uint64_t fenceValue)
{
	if (IsFenceComplete(fenceValue))
	{
		return;
	}

	// TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
	// wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
	// the fence can only have one event set on completion, then thread B has to wait for 
	// 100 before it knows 99 is ready.  Maybe insert sequential events?
	{
		VkSemaphoreWaitInfo waitInfo;
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.pNext = nullptr;
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &m_timelineSemaphore;
		waitInfo.pValues = &fenceValue;

		ThrowIfFailed(vkWaitSemaphores(GetDevice(), &waitInfo, UINT64_MAX));

		m_lastCompletedFenceValue = fenceValue;
	}
}


uint64_t CommandQueue::ExecuteCommandList(VkCommandBuffer cmdList)
{
	lock_guard<mutex> lockGuard(m_fenceMutex);

	VkTimelineSemaphoreSubmitInfo timelineInfo;
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.pNext = nullptr;
	timelineInfo.waitSemaphoreValueCount = 0;
	timelineInfo.pWaitSemaphoreValues = nullptr;
	timelineInfo.signalSemaphoreValueCount = 1;
	timelineInfo.pSignalSemaphoreValues = &m_nextFenceValue;

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = &timelineInfo;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_timelineSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdList;

	ThrowIfFailed(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE));

	// Increment the fence value.  
	return m_nextFenceValue++;
}


VkCommandBuffer CommandQueue::RequestCommandBuffer()
{
	uint64_t completedFence;
	ThrowIfFailed(vkGetSemaphoreCounterValue(GetDevice(), m_timelineSemaphore, &completedFence));

	return m_commandBufferPool.RequestCommandBuffer(completedFence);
}


void CommandQueue::DiscardCommandBuffer(uint64_t fenceValueForReset, VkCommandBuffer commandBuffer)
{
	m_commandBufferPool.DiscardCommandBuffer(fenceValueForReset, commandBuffer);
}


CommandListManager::CommandListManager()
	: m_graphicsQueue(CommandListType::Direct)
	, m_computeQueue(CommandListType::Compute)
	, m_copyQueue(CommandListType::Copy)
{}


CommandListManager::~CommandListManager()
{
	// TODO
	//Destroy();
}


void CommandListManager::Create()
{
	m_graphicsQueue.Create();
	m_computeQueue.Create();
	m_copyQueue.Create();
}


void CommandListManager::Destroy()
{
	m_graphicsQueue.Destroy();
	m_computeQueue.Destroy();
	m_copyQueue.Destroy();
}


void CommandListManager::WaitForFence(uint64_t fenceValue)
{
	CommandQueue& producer = g_commandManager.GetQueue((CommandListType)(fenceValue >> 56));
	producer.WaitForFence(fenceValue);
}