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

#include "Color.h"
#include "FramebufferVk.h"
#include "GpuBufferVk.h"

namespace Kodiak
{

// Forward declarations
class ComputeContext;
class GraphicsContext;
class RenderPass;

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
	static CommandContext& Begin(const std::string id = "");

	// Flush existing commands and release the current context
	void Finish(bool waitForCompletion = false);

	void Finish(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitForCompletion = false);

	// Prepare to render by reserving a command list
	void Initialize();

	GraphicsContext& GetGraphicsContext()
	{
		return reinterpret_cast<GraphicsContext&>(*this);
	}

	ComputeContext& GetComputeContext()
	{
		return reinterpret_cast<ComputeContext&>(*this);
	}

	static void InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset = false, size_t offset = 0);

protected:
	VkCommandBuffer m_commandList{ VK_NULL_HANDLE };

private:
	CommandContext() = default;

	void Reset();

	void FinishInternal(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitForCompletion);
};


class GraphicsContext : public CommandContext
{
public:

	static GraphicsContext& Begin(const std::string& id = "")
	{
		return CommandContext::Begin(id).GetGraphicsContext();
	}

	void BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor);
	void BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor, float clearDepth, uint32_t clearStencil);
	void EndRenderPass();

	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

	// TODO remove me
	void BindDescriptorSet(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
	void BindPipeline(VkPipeline pipeline);

	void SetIndexBuffer(IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, VertexBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);

	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
};


inline void GraphicsContext::EndRenderPass()
{
	vkCmdEndRenderPass(m_commandList);
}


inline void GraphicsContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
	vkCmdBindIndexBuffer(m_commandList, indexBuffer.GetBuffer(), 0, indexBuffer.GetIndexType());
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, VertexBuffer& vertexBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	VkBuffer buffers[1] = { vertexBuffer.GetBuffer() };
	vkCmdBindVertexBuffers(m_commandList, 0, 1, buffers, offsets);
}


inline void GraphicsContext::SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[])
{
	std::vector<VkDeviceSize> offsets(count);
	std::vector<VkBuffer> buffers(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		offsets[i] = 0;
		buffers[i] = vertexBuffers[i].GetBuffer();
	}
	vkCmdBindVertexBuffers(m_commandList, startSlot, count, buffers.data(), offsets.data());
}


inline void GraphicsContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	vkCmdDrawIndexed(m_commandList, indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

} // namespace Kodiak