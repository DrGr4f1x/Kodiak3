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

#include "GpuBufferVk.h"

namespace Kodiak
{

class ContextManager
{
public:
	ContextManager() = default;

	CommandContext* AllocateContext();
	void FreeContext(CommandContext*);
	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<CommandContext> > sm_contextPool;
	std::queue<CommandContext*> sm_availableContexts;
	std::mutex sm_contextAllocationMutex;
};


class CommandContext : NonCopyable
{
	friend class ContextManager;

public:
	static CommandContext& Begin(const std::string ID = "");

	// Flush existing commands and release the current context
	void Finish(bool waitForCompletion = false);

	// Prepare to render by reserving a command list
	void Initialize();

	static void InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset = false, size_t offset = 0);

protected:
	VkCommandBuffer m_commandList{ VK_NULL_HANDLE };

private:
	CommandContext() = default;

	void Reset();

	void FinishInternal(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitForCompletion);
};

} // namespace Kodiak