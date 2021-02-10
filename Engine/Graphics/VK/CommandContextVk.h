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
#include "Graphics\Framebuffer.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\Texture.h"

#include "CommandListManagerVk.h"
#include "ResourceSetVk.h"
#include "UtilVk.h"


namespace Kodiak
{

// Forward declarations
class ComputeContext;
class ComputePSO;
class GraphicsContext;
class GraphicsPSO;
class OcclusionQueryHeap;
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
	~CommandContext() = default;

	static void DestroyAllContexts();

	static CommandContext& Begin(const std::string id = "");

	// Flush existing commands and release the current context
	void Finish(bool waitForCompletion = false);

	// Debug events and markers
	void BeginEvent(const std::string& label);
	void EndEvent();
	void SetMarker(const std::string& label);

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

	void TransitionResource(GpuResource& resource, ResourceState newState, bool flushImmediate = false);
	void InsertUAVBarrier(GpuResource& resource, bool flushImmediate = false);
	//void InsertAliasBarrier(GpuResource& before, GpuResource& after, bool flushImmediate = false);
	inline void FlushResourceBarriers() { /* TODO - see if we can cache and flush multiple barriers at once */ }

protected:
	CommandListType m_type;
	VkCommandBuffer m_commandList{ VK_NULL_HANDLE };

	bool m_isRenderPassActive{ false };

	// Current pipeline layouts
	VkPipelineLayout m_curGraphicsPipelineLayout{ VK_NULL_HANDLE };
	VkPipelineLayout m_curComputePipelineLayout{ VK_NULL_HANDLE };

	// Shader stages for the 8 descriptor set slots
	VkShaderStageFlags m_shaderStages[8];

	bool m_hasPendingDebugEvent{ false };

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

	void ClearColor(ColorBuffer& target);
	void ClearColor(ColorBuffer& target, Color clearColor);
	void ClearDepth(DepthBuffer& target);
	void ClearStencil(DepthBuffer& target);
	void ClearDepthAndStencil(DepthBuffer& target);

	void BeginRenderPass(FrameBuffer& framebuffer);
	void EndRenderPass();

	void BeginOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex);
	void EndOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex);
	void ResolveOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries, GpuResource& destBuffer, uint64_t destBufferOffset);
	void ResetOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries);

	void SetRootSignature(const RootSignature& rootSig);

	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetStencilRef(uint32_t stencilRef);

	void SetPipelineState(const GraphicsPSO& PSO);
	
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants);
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset);
	void SetResources(const ResourceSet& resources);	

	void SetIndexBuffer(const IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
	void SetVertexBuffer(uint32_t slot, const StructuredBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);

	void Resolve(ColorBuffer& src, ColorBuffer& dest, Format format);
};


class ComputeContext : public CommandContext
{
public:
	void SetRootSignature(const RootSignature& RootSig);

	void SetPipelineState(const ComputePSO& PSO);
	
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants);
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset);
	void SetResources(const ResourceSet& resources);

	void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
	void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
	void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
	void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
};


class ScopedDrawEvent
{
public:
	ScopedDrawEvent(CommandContext& context, const std::string& label)
		: m_context(context)
	{
		context.BeginEvent(label);
	}

	~ScopedDrawEvent()
	{
		m_context.EndEvent();
	}

private:
	CommandContext& m_context;
};


inline void GraphicsContext::ClearColor(ColorBuffer& target)
{
	ResourceState oldState = target.m_usageState;

	TransitionResource(target, ResourceState::CopyDest);

	VkClearColorValue colVal;
	Color clearColor = target.GetClearColor();
	colVal.float32[0] = clearColor.R();
	colVal.float32[1] = clearColor.G();
	colVal.float32[2] = clearColor.B();
	colVal.float32[3] = clearColor.A();

	VkImageSubresourceRange range;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = target.GetArraySize();
	range.levelCount = target.GetNumMips();
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	FlushResourceBarriers();
	vkCmdClearColorImage(m_commandList, target.m_resource, GetImageLayout(target.m_usageState), &colVal, 1, &range);

	TransitionResource(target, oldState);
}


inline void GraphicsContext::ClearColor(ColorBuffer& target, Color clearColor)
{
	ResourceState oldState = target.m_usageState;

	TransitionResource(target, ResourceState::CopyDest);

	VkClearColorValue colVal;
	colVal.float32[0] = clearColor.R();
	colVal.float32[1] = clearColor.G();
	colVal.float32[2] = clearColor.B();
	colVal.float32[3] = clearColor.A();

	VkImageSubresourceRange range;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = target.GetArraySize();
	range.levelCount = target.GetNumMips();
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	FlushResourceBarriers();
	vkCmdClearColorImage(m_commandList, target.m_resource, GetImageLayout(target.m_usageState), &colVal, 1, &range);

	TransitionResource(target, oldState);
}


inline void GraphicsContext::ClearDepth(DepthBuffer& target)
{
	ResourceState oldState = target.m_usageState;

	TransitionResource(target, ResourceState::CopyDest);

	VkClearDepthStencilValue depthVal;
	depthVal.depth = target.GetClearDepth();
	depthVal.stencil = target.GetClearStencil();

	VkImageSubresourceRange range;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = target.GetArraySize();
	range.levelCount = target.GetNumMips();
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	FlushResourceBarriers();
	vkCmdClearDepthStencilImage(m_commandList, target.m_resource, GetImageLayout(target.m_usageState), &depthVal, 1, &range);

	TransitionResource(target, oldState);
}


inline void GraphicsContext::ClearStencil(DepthBuffer& target)
{
	ResourceState oldState = target.m_usageState;

	TransitionResource(target, ResourceState::CopyDest);

	VkClearDepthStencilValue depthVal;
	depthVal.depth = target.GetClearDepth();
	depthVal.stencil = target.GetClearStencil();

	VkImageSubresourceRange range;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = target.GetArraySize();
	range.levelCount = target.GetNumMips();
	range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

	FlushResourceBarriers();
	vkCmdClearDepthStencilImage(m_commandList, target.m_resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &depthVal, 1, &range);

	TransitionResource(target, oldState);
}


inline void GraphicsContext::ClearDepthAndStencil(DepthBuffer& target)
{
	ResourceState oldState = target.m_usageState;

	TransitionResource(target, ResourceState::CopyDest);

	VkClearDepthStencilValue depthVal;
	depthVal.depth = target.GetClearDepth();
	depthVal.stencil = target.GetClearStencil();

	VkImageSubresourceRange range;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = target.GetArraySize();
	range.levelCount = target.GetNumMips();
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	FlushResourceBarriers();
	vkCmdClearDepthStencilImage(m_commandList, target.m_resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &depthVal, 1, &range);
	TransitionResource(target, oldState);
}


inline void GraphicsContext::EndRenderPass()
{
	vkCmdEndRenderPass(m_commandList);
	m_isRenderPassActive = false;
}


inline void GraphicsContext::SetStencilRef(uint32_t stencilRef)
{
	vkCmdSetStencilReference(m_commandList, VK_STENCIL_FRONT_AND_BACK, stencilRef);
}


inline void GraphicsContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants)
{
	vkCmdPushConstants(
		m_commandList, 
		m_curGraphicsPipelineLayout, 
		m_shaderStages[rootIndex],
		0, 
		numConstants * sizeof(DWORD), 
		constants);
}


inline void GraphicsContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset)
{
	vkCmdPushConstants(
		m_commandList,
		m_curGraphicsPipelineLayout,
		m_shaderStages[rootIndex],
		offset * sizeof(DWORD),
		numConstants * sizeof(DWORD),
		constants);
}


inline void GraphicsContext::SetResources(const ResourceSet& resources)
{
	uint32_t i = 0;
	for (i = 0; i < 8; ++i)
	{
		int rootIndex = resources.m_resourceTables[i].rootIndex;
		if (rootIndex == -1)
			break;

		if (resources.m_resourceTables[i].descriptorSet == VK_NULL_HANDLE)
			continue;

		vkCmdBindDescriptorSets(
			m_commandList, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, 
			m_curGraphicsPipelineLayout, 
			(uint32_t)rootIndex, 
			1, 
			&resources.m_resourceTables[i].descriptorSet, 
			resources.m_resourceTables[i].isDynamicCBV ? 1 : 0,
			resources.m_resourceTables[i].isDynamicCBV ? &resources.m_dynamicOffsets[i] : nullptr);
	}

	if (resources.m_staticSamplers != VK_NULL_HANDLE)
	{
		vkCmdBindDescriptorSets(
			m_commandList,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_curGraphicsPipelineLayout,
			(uint32_t)resources.m_staticSamplerIndex,
			1,
			&resources.m_staticSamplers,
			0,
			nullptr);
	}
}


inline void GraphicsContext::SetIndexBuffer(const IndexBuffer& indexBuffer)
{
	VkIndexType indexType = indexBuffer.IndexSize16Bit() ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
	vkCmdBindIndexBuffer(m_commandList, indexBuffer.m_resource, 0, indexType);
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	VkBuffer buffers[1] = { vertexBuffer.m_resource };
	vkCmdBindVertexBuffers(m_commandList, slot, 1, buffers, offsets);
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, const StructuredBuffer& vertexBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	VkBuffer buffers[1] = { vertexBuffer.m_resource };
	vkCmdBindVertexBuffers(m_commandList, slot, 1, buffers, offsets);
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
	FlushResourceBarriers();
	vkCmdDraw(m_commandList, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}


inline void GraphicsContext::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
	int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	FlushResourceBarriers();
	vkCmdDrawIndexed(m_commandList, indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}


inline void GraphicsContext::Resolve(ColorBuffer& src, ColorBuffer& dest, Format format)
{
	FlushResourceBarriers();

	VkImageResolve resolve = {};
	resolve.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	resolve.srcSubresource.baseArrayLayer = 0;
	resolve.srcSubresource.layerCount = src.GetArraySize();
	resolve.srcSubresource.mipLevel = 0;
	resolve.srcOffset.x = 0;
	resolve.srcOffset.y = 0;
	resolve.srcOffset.z = 0;
	resolve.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	resolve.dstSubresource.baseArrayLayer = 0;
	resolve.dstSubresource.layerCount = dest.GetArraySize();
	resolve.dstSubresource.mipLevel = 0;
	resolve.dstOffset.x = 0;
	resolve.dstOffset.y = 0;
	resolve.dstOffset.z = 0;
	resolve.extent.width = dest.GetWidth();
	resolve.extent.height = dest.GetHeight();;
	resolve.extent.depth = 1;

	vkCmdResolveImage(
		m_commandList, 
		src.m_resource, 
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
		dest.m_resource, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		1, 
		&resolve);
}


inline void ComputeContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants)
{
	vkCmdPushConstants(
		m_commandList,
		m_curGraphicsPipelineLayout,
		VK_SHADER_STAGE_COMPUTE_BIT,
		0,
		numConstants * sizeof(DWORD),
		constants);
}


inline void ComputeContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset)
{
	vkCmdPushConstants(
		m_commandList,
		m_curGraphicsPipelineLayout,
		VK_SHADER_STAGE_COMPUTE_BIT,
		offset * sizeof(DWORD),
		numConstants * sizeof(DWORD),
		constants);
}


inline void ComputeContext::SetResources(const ResourceSet& resources)
{
	uint32_t i = 0;
	for (i = 0; i < 8; ++i)
	{
		if (resources.m_resourceTables[i].descriptorSet == VK_NULL_HANDLE)
			break;

		vkCmdBindDescriptorSets(
			m_commandList,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			m_curComputePipelineLayout,
			i,
			1,
			&resources.m_resourceTables[i].descriptorSet,
			0,
			nullptr);
	}

	if (resources.m_staticSamplers != VK_NULL_HANDLE)
	{
		vkCmdBindDescriptorSets(
			m_commandList,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			m_curGraphicsPipelineLayout,
			i,
			1,
			&resources.m_staticSamplers,
			0,
			nullptr);
	}
}


inline void ComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	FlushResourceBarriers();
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