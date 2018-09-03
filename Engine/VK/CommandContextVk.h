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
#include "DepthBuffer.h"
#include "GpuBuffer.h"

#include "ColorBufferVk.h"
#include "DynamicDescriptorPoolVk.h"
#include "FramebufferVk.h"
#include "TextureVk.h"


namespace Kodiak
{

// Forward declarations
class ComputeContext;
class ComputePSO;
class GraphicsContext;
class GraphicsPSO;
class RenderPass;
class RootSignature;


class ContextManager
{
public:
	ContextManager() = default;

	CommandContext* AllocateContext(CommandListType type);
	void FreeContext(CommandContext*);
	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<CommandContext> > sm_contextPool[4];
	std::queue<CommandContext*> sm_availableContexts[4];
	std::mutex sm_contextAllocationMutex;
};


class CommandContext : NonCopyable
{
	friend class ContextManager;
	friend class DynamicDescriptorPool;

public:
	~CommandContext();

	static void DestroyAllContexts();

	static CommandContext& Begin(const std::string id = "");

	// Flush existing commands and release the current context
	void Finish(bool waitForCompletion = false);

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

	static void InitializeTexture(Texture& dest, size_t numBytes, const void* initData, uint32_t numBuffers, VkBufferImageCopy bufferCopies[]);
	static void InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset = false, size_t offset = 0);

	void TransitionResource(Texture& texture, ResourceState newState, bool flushImmediate = false);
	void TransitionResource(ColorBuffer& colorBuffer, ResourceState newState, bool flushImmediate = false);
	inline void FlushImageBarriers();

protected:
	CommandListType m_type;
	VkCommandBuffer m_commandList{ VK_NULL_HANDLE };

	// Image barriers
	VkImageMemoryBarrier m_pendingImageBarriers[16];
	uint8_t m_numImageBarriersToFlush{ 0 };

	// Current pipeline layouts
	VkPipelineLayout m_curGraphicsPipelineLayout{ VK_NULL_HANDLE };
	VkPipelineLayout m_curComputePipelineLayout{ VK_NULL_HANDLE };

	// Descriptor pool
	DynamicDescriptorPool m_dynamicDescriptorPool;

	VkSemaphore m_signalSemaphore{ VK_NULL_HANDLE };

private:
	CommandContext(CommandListType type);

	void Reset();
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

	void SetRootSignature(const RootSignature& rootSig);

	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetStencilRef(uint32_t stencilRef);

	void SetPipelineState(const GraphicsPSO& PSO);
	void SetRootConstantBuffer(uint32_t rootIndex, ConstantBuffer& constantBuffer);
	void SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	//void SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer);

	void SetIndexBuffer(const IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);
};


class ComputeContext : public CommandContext
{
public:
	void SetRootSignature(const RootSignature& RootSig);

	void SetPipelineState(const ComputePSO& PSO);
	void SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer);
	void SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	//void SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer);
	void SetUAV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);

	void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
	void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
	void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
	void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
};


inline void CommandContext::FlushImageBarriers()
{
	if (m_numImageBarriersToFlush > 0)
	{
		vkCmdPipelineBarrier(
			m_commandList,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			m_numImageBarriersToFlush, m_pendingImageBarriers);

		m_numImageBarriersToFlush = 0;
	}
}


inline void GraphicsContext::EndRenderPass()
{
	vkCmdEndRenderPass(m_commandList);
}


inline void GraphicsContext::SetStencilRef(uint32_t stencilRef)
{
	vkCmdSetStencilReference(m_commandList, VK_STENCIL_FRONT_AND_BACK, stencilRef);
}


inline void GraphicsContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	VkDescriptorBufferInfo handle = constantBuffer.GetCBV().GetHandle();
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = texture.GetSRV();
	descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = colorBuffer.GetSRV();
	descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


//inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer)
//{
//	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &depthBuffer.GetDepthSRV());
//}


inline void GraphicsContext::SetIndexBuffer(const IndexBuffer& indexBuffer)
{
	VkIndexType indexType = indexBuffer.IndexSize16Bit() ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
	vkCmdBindIndexBuffer(m_commandList, indexBuffer.m_resource, 0, indexType);
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	VkBuffer buffers[1] = { vertexBuffer.m_resource };
	vkCmdBindVertexBuffers(m_commandList, 0, 1, buffers, offsets);
}


inline void GraphicsContext::SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[])
{
	std::vector<VkDeviceSize> offsets(count);
	std::vector<VkBuffer> buffers(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		offsets[i] = 0;
		buffers[i] = vertexBuffers[i].m_resource;
	}
	vkCmdBindVertexBuffers(m_commandList, startSlot, count, buffers.data(), offsets.data());
}


inline void GraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset)
{
	DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
}


inline void GraphicsContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}


inline void GraphicsContext::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
	uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	FlushImageBarriers();
	m_dynamicDescriptorPool.CommitGraphicsRootDescriptorSets(m_commandList);
	vkCmdDraw(m_commandList, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}


inline void GraphicsContext::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
	int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	FlushImageBarriers();
	m_dynamicDescriptorPool.CommitGraphicsRootDescriptorSets(m_commandList);
	vkCmdDrawIndexed(m_commandList, indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}


inline void ComputeContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	VkDescriptorBufferInfo handle = constantBuffer.GetCBV().GetHandle();
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = texture.GetSRV();
	descriptorInfo.imageLayout = texture.m_layout;
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = colorBuffer.GetSRV();
	descriptorInfo.imageLayout = colorBuffer.GetLayout();
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


// TODO
//inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer)
//{
//	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &depthBuffer.GetDepthSRV());
//}


inline void ComputeContext::SetUAV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = colorBuffer.GetSRV();
	descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


inline void ComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	FlushImageBarriers();
	m_dynamicDescriptorPool.CommitComputeRootDescriptorSets(m_commandList);
	vkCmdDispatch(m_commandList, groupCountX, groupCountY, groupCountZ);
}


inline void ComputeContext::Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX)
{
	Dispatch(Math::DivideByMultiple(threadCountX, groupSizeX), 1, 1);
}


inline void ComputeContext::Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
{
	Dispatch(
		Math::DivideByMultiple(threadCountX, groupSizeX),
		Math::DivideByMultiple(threadCountY, groupSizeY), 1);
}


inline void ComputeContext::Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ)
{
	Dispatch(
		Math::DivideByMultiple(threadCountX, groupSizeX),
		Math::DivideByMultiple(threadCountY, groupSizeY),
		Math::DivideByMultiple(threadCountZ, groupSizeZ));
}

} // namespace Kodiak