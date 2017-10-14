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

#include "CommandContextVk.h"

#include "CommandBufferPoolVk.h"
#include "GraphicsDeviceVk.h"


using namespace Kodiak;
using namespace std;


ContextManager g_contextManager;


CommandContext* ContextManager::AllocateContext()
{
	lock_guard<mutex> LockGuard(sm_contextAllocationMutex);

	auto& availableContexts = sm_availableContexts;

	CommandContext* ret = nullptr;
	if (availableContexts.empty())
	{
		ret = new CommandContext();
		sm_contextPool.emplace_back(ret);
		ret->Initialize();
	}
	else
	{
		ret = availableContexts.front();
		availableContexts.pop();
		ret->Reset();
	}
	assert(ret != nullptr);

	return ret;
}


void ContextManager::FreeContext(CommandContext* usedContext)
{
	assert(usedContext != nullptr);

	lock_guard<mutex> LockGuard(sm_contextAllocationMutex);
	sm_availableContexts.push(usedContext);
}


void ContextManager::DestroyAllContexts()
{
	sm_contextPool.clear();
}


CommandContext& CommandContext::Begin(const string ID)
{
	CommandContext* newContext = g_contextManager.AllocateContext();
	// TODO
#if 0
	NewContext->SetID(ID);
	if (ID.length() > 0)
	{
		EngineProfiling::BeginBlock(ID, NewContext);
	}
#endif

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.pNext = nullptr;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(newContext->m_commandList, &beginInfo);

	return *newContext;
}


void CommandContext::Finish(bool waitForCompletion)
{
	FinishInternal(VK_NULL_HANDLE, VK_NULL_HANDLE, waitForCompletion);
}


void CommandContext::Initialize()
{
	assert(m_commandList == VK_NULL_HANDLE);
	m_commandList = g_commandBufferPool.RequestCommandBuffer();
}


void CommandContext::InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset, size_t offset)
{
	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = numBytes;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VkBuffer stagingBuffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(GetDevice(), &stagingBufferInfo, nullptr, &stagingBuffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(GetDevice(), stagingBuffer, &memReqs);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data = nullptr;

	// Copy to staging buffer with Map/Unmap
	VkDeviceMemory stagingBufferMemory{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(GetDevice(), &memAlloc, nullptr, &stagingBufferMemory));
	ThrowIfFailed(vkMapMemory(GetDevice(), stagingBufferMemory, 0, numBytes, 0, &data));
	memcpy(data, initialData, numBytes);
	vkUnmapMemory(GetDevice(), stagingBufferMemory);
	ThrowIfFailed(vkBindBufferMemory(GetDevice(), stagingBuffer, stagingBufferMemory, 0));

	// Upload to GPU
	CommandContext& context = CommandContext::Begin();

	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion = {};
	copyRegion.size = numBytes;
	vkCmdCopyBuffer(context.m_commandList, stagingBuffer, dest.GetBuffer(), 1, &copyRegion);

	context.Finish(true);

	// Destroy staging buffer and memory
	vkDestroyBuffer(GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(GetDevice(), stagingBufferMemory, nullptr);
}


void CommandContext::Reset()
{
	assert(m_commandList == VK_NULL_HANDLE);
	m_commandList = g_commandBufferPool.RequestCommandBuffer();
}


void CommandContext::FinishInternal(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitForCompletion)
{
	// TODO
	auto device = g_commandBufferPool.GetDevice();
	auto _queue = g_commandBufferPool.GetQueue();

	//FlushImageBarriers();
	//FlushBufferBarriers();

	// TODO
#if 0
	if (m_ID.length() > 0)
	{
		EngineProfiling::EndBlock(this);
	}
#endif

	vkEndCommandBuffer(m_commandList);

	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandList;
	if (waitSemaphore != VK_NULL_HANDLE)
	{
		submitInfo.pWaitDstStageMask = &waitStageMask;			// Pointer to the list of pipeline stages that the semaphore waits will occur at
		submitInfo.pWaitSemaphores = &waitSemaphore;			// Semaphore(s) to wait upon before the submitted command buffer starts executing
		submitInfo.waitSemaphoreCount = 1;						// One wait semaphore
	}
	if (signalSemaphore != VK_NULL_HANDLE)
	{
		submitInfo.pSignalSemaphores = &signalSemaphore;		// Semaphore(s) to be signaled when command buffers have completed
		submitInfo.signalSemaphoreCount = 1;					// One signal semaphore
	}

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	ThrowIfFailed(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

	// Submit to the queue
	ThrowIfFailed(vkQueueSubmit(_queue, 1, &submitInfo, fence));

	g_commandBufferPool.DiscardCommandBuffer(fence, m_commandList);
	m_commandList = VK_NULL_HANDLE;

	// Recycle dynamic allocations
	//m_vertexBufferAllocator.CleanupUsedBuffers(fence);
	//m_indexBufferAllocator.CleanupUsedBuffers(fence);
	//m_constantBufferAllocator.CleanupUsedBuffers(fence);
	//m_dynamicDescriptorPool.CleanupUsedPools(fence);

	if (waitForCompletion)
	{
		ThrowIfFailed(vkWaitForFences(device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
	}

	g_contextManager.FreeContext(this);
}