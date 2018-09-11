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
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "Framebuffer.h"
#include "GpuBuffer.h"
#include "Texture.h"

#include "DynamicDescriptorPoolVk.h"
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

	void TransitionResource(GpuResource& resource, ResourceState newState, bool flushImmediate = false);
	inline void FlushResourceBarriers() { /* TODO - see if we can cache and flush multiple barriers at once */ }

protected:
	CommandListType m_type;
	VkCommandBuffer m_commandList{ VK_NULL_HANDLE };

	bool m_isRenderPassActive{ false };

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
	void SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer);
	

	void SetIndexBuffer(const IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
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
	void SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer);
	void SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	//void SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer);
	void SetUAV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	void SetUAV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer);

	void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
	void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
	void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
	void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
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


inline void GraphicsContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	VkDescriptorBufferInfo handle = constantBuffer.GetCBV().GetHandle();
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = texture.GetSRV().GetHandle();
	descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer)
{
	if (buffer.m_type == ResourceType::TypedBuffer)
	{
		VkBufferView bufferView = buffer.GetSRV().GetHandle();
		m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &bufferView);
	}
	else
	{
		VkDescriptorBufferInfo descriptorInfo = {};
		descriptorInfo.buffer = buffer.GetSRV().GetHandle();
		descriptorInfo.offset = 0;
		descriptorInfo.range = buffer.GetSize();
		m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
	}
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	VkDescriptorImageInfo descriptorInfo = colorBuffer.GetSRV().GetHandle();
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
	FlushResourceBarriers();
	m_dynamicDescriptorPool.CommitGraphicsRootDescriptorSets(m_commandList);
	vkCmdDraw(m_commandList, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}


inline void GraphicsContext::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
	int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	FlushResourceBarriers();
	m_dynamicDescriptorPool.CommitGraphicsRootDescriptorSets(m_commandList);
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


inline void ComputeContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	VkDescriptorBufferInfo handle = constantBuffer.GetCBV().GetHandle();
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	VkDescriptorImageInfo descriptorInfo = {};
	descriptorInfo.imageView = texture.GetSRV().GetHandle();
	descriptorInfo.imageLayout = GetImageLayout(texture.m_usageState);
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	VkDescriptorImageInfo descriptorInfo = colorBuffer.GetSRV().GetHandle();
	descriptorInfo.imageLayout = GetImageLayout(colorBuffer.m_usageState);
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


// TODO
//inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer)
//{
//	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &depthBuffer.GetDepthSRV());
//}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer)
{
	if (buffer.m_type == ResourceType::TypedBuffer)
	{
		VkBufferView bufferView = buffer.GetSRV().GetHandle();
		m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &bufferView);
	}
	else
	{
		VkDescriptorBufferInfo descriptorInfo = {};
		descriptorInfo.buffer = buffer.GetSRV().GetHandle();
		descriptorInfo.offset = 0;
		descriptorInfo.range = buffer.GetSize();
		m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
	}
}


inline void ComputeContext::SetUAV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	VkDescriptorImageInfo descriptorInfo = colorBuffer.GetSRV().GetHandle();
	descriptorInfo.imageLayout = GetImageLayout(colorBuffer.m_usageState);
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
}


inline void ComputeContext::SetUAV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer)
{
	if (buffer.m_type == ResourceType::TypedBuffer)
	{
		VkBufferView bufferView = buffer.GetUAV().GetHandle();
		m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &bufferView);
	}
	else
	{
		VkDescriptorBufferInfo descriptorInfo = {};
		descriptorInfo.buffer = buffer.GetUAV().GetHandle();
		descriptorInfo.offset = 0;
		descriptorInfo.range = buffer.GetSize();
		m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, offset, 1, &descriptorInfo);
	}
}


inline void ComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	FlushResourceBarriers();
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