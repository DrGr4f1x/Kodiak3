//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "CommandBufferPoolVk.h"

namespace Kodiak
{

class CommandQueue
{
	friend class CommandListManager;
	friend class CommandContext;

public:
	CommandQueue(CommandListType type);
	~CommandQueue();

	inline bool IsReady()
	{
		return m_queue != VK_NULL_HANDLE;
	}

	void Create();
	void Destroy();

	uint64_t IncrementFence();
	bool IsFenceComplete(uint64_t fenceValue);
	void StallForFence(uint64_t fenceValue);
	void StallForProducer(CommandQueue& producer);
	void WaitForFence(uint64_t fenceValue);
	void WaitForIdle()
	{
		WaitForFence(IncrementFence());
	}

	VkQueue GetCommandQueue() { return m_queue; }

	uint64_t GetNextFenceValue() const { return m_nextFenceValue; }

	VkSemaphore GetTimelineSemaphore() { return m_timelineSemaphore; }

private:
	uint64_t ExecuteCommandList(VkCommandBuffer cmdList);
	VkCommandBuffer RequestCommandBuffer();
	void DiscardCommandBuffer(uint64_t fenceValueForReset, VkCommandBuffer commandBuffer);

private:
	const CommandListType m_type;
	VkQueue m_queue{ VK_NULL_HANDLE };

	CommandBufferPool m_commandBufferPool;
	std::mutex m_fenceMutex;

	VkSemaphore m_timelineSemaphore{ VK_NULL_HANDLE };
	uint64_t m_nextFenceValue;
	uint64_t m_lastCompletedFenceValue;
};


class CommandListManager
{
public:
	CommandListManager();
	~CommandListManager();

	void Create();
	void Destroy();

	CommandQueue& GetGraphicsQueue() { return m_graphicsQueue; }
	CommandQueue& GetComputeQueue() { return m_computeQueue; }
	CommandQueue& GetCopyQueue() { return m_copyQueue; }

	VkQueue GetCommandQueue()
	{
		return m_graphicsQueue.GetCommandQueue();
	}

	CommandQueue& GetQueue(CommandListType type = CommandListType::Direct)
	{
		switch (type)
		{
		case CommandListType::Compute: return m_computeQueue;
		case CommandListType::Copy: return m_copyQueue;
		default: return m_graphicsQueue;
		}
	}

	// Test to see if a fence has already been reached
	bool IsFenceComplete(uint64_t fenceValue)
	{
		return GetQueue(static_cast<CommandListType>(fenceValue >> 56)).IsFenceComplete(fenceValue);
	}

	// The CPU will wait for a fence to reach a specified value
	void WaitForFence(uint64_t fenceValue);

	// The CPU will wait for all command queues to empty (so that the GPU is idle)
	void IdleGPU()
	{
		m_graphicsQueue.WaitForIdle();
		m_computeQueue.WaitForIdle();
		m_copyQueue.WaitForIdle();
	}

private:
	CommandQueue m_graphicsQueue;
	CommandQueue m_computeQueue;
	CommandQueue m_copyQueue;
};

extern CommandListManager g_commandManager;

} // namespace Kodiak