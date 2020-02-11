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

#include <list>

namespace Kodiak
{

class Fence
{
	friend class CommandQueue;
	friend class CommandListManager;
public:
	Fence() = delete;
	explicit Fence(VkFence fence, CommandListType type);
	Fence(const Fence& other) = delete;
	Fence(Fence&& other) = delete;
	~Fence();

	bool IsComplete() const;

	Fence& operator=(const Fence& other) = delete;
	Fence& operator=(Fence&& other) = delete;

private:
	VkFence m_fence;
	CommandListType m_type;
};


class CommandQueue
{
	friend class CommandListManager;
	friend class CommandContext;

public:
	CommandQueue(CommandListType type);
	~CommandQueue();

	void SetWaitSemaphore(VkSemaphore waitSemaphore);
	VkSemaphore GetWaitSemaphore() { return m_waitSemaphore; }

	void Create();
	void Destroy();

	bool IsFenceComplete(std::shared_ptr<Fence> fence);
	void WaitForFence(std::shared_ptr<Fence> fence);
	void WaitForIdle();

	inline bool IsReady()
	{
		return m_queue != VK_NULL_HANDLE;
	}

	VkQueue GetCommandQueue() { return m_queue; }

private:
	std::shared_ptr<Fence> ExecuteCommandList(VkCommandBuffer cmdList, VkSemaphore signalSemaphore);
	VkCommandBuffer RequestCommandBuffer();
	void DiscardCommandBuffer(std::shared_ptr<Fence> fence, VkCommandBuffer commandBuffer);

private:
	const CommandListType m_type;
	VkQueue m_queue{ VK_NULL_HANDLE };

	CommandBufferPool m_commandBufferPool;

	VkSemaphore m_waitSemaphore{ VK_NULL_HANDLE };
};


class CommandListManager
{
public:
	CommandListManager();
	~CommandListManager();

	void BeginFrame(VkSemaphore waitSemaphore);

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

	void WaitForFence(std::shared_ptr<Fence> fence);

	// The CPU will wait for all command queues to empty (so that the GPU is idle)
	void IdleGPU();

	void RetireFence(VkFence fence);
	void DestroyRetiredFences();

private:
	CommandQueue m_graphicsQueue;
	CommandQueue m_computeQueue;
	CommandQueue m_copyQueue;

	std::mutex m_fenceMutex;
	std::list<VkFence> m_retiredFences;
};

extern CommandListManager g_commandManager;

} // namespace Kodiak