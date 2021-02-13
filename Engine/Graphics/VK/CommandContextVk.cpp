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

#include "Graphics\GraphicsDevice.h"
#include "Graphics\PipelineState.h"
#include "Graphics\QueryHeap.h"

#include "CommandListManagerVk.h"
#include "RootSignatureVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


// Extension methods
extern PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet;
extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel;
extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel;
extern PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel;


ContextManager g_contextManager;

static bool IsValidComputeResourceState(ResourceState state)
{
	// TODO: Also ResourceState::ShaderResource?
	switch (state)
	{
	case ResourceState::NonPixelShaderResource:
	case ResourceState::UnorderedAccess:
	case ResourceState::CopyDest:
	case ResourceState::CopySource:
		return true;

	default:
		return false;
	}
}


CommandContext* ContextManager::AllocateContext(CommandListType type)
{
	lock_guard<mutex> LockGuard(sm_contextAllocationMutex);

	auto& availableContexts = sm_availableContexts[static_cast<uint32_t>(type)];

	CommandContext* ret = nullptr;
	if (availableContexts.empty())
	{
		ret = new CommandContext(type);
		sm_contextPool[static_cast<uint32_t>(type)].emplace_back(ret);
		ret->Initialize();
	}
	else
	{
		ret = availableContexts.front();
		availableContexts.pop();
		ret->Reset();
	}
	assert(ret != nullptr);

	assert(ret->m_type == type);

	return ret;
}


void ContextManager::FreeContext(CommandContext* usedContext)
{
	assert(usedContext != nullptr);

	lock_guard<mutex> LockGuard(sm_contextAllocationMutex);
	sm_availableContexts[static_cast<uint32_t>(usedContext->m_type)].push(usedContext);
}


void ContextManager::DestroyAllContexts()
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		sm_contextPool[i].clear();
	}
}


CommandContext::CommandContext(CommandListType type)
	: m_type(type)
{}


void CommandContext::DestroyAllContexts()
{
	// TODO
#if 0
	LinearAllocator::DestroyAll();
#endif
	g_contextManager.DestroyAllContexts();
}


CommandContext& CommandContext::Begin(const string ID)
{
	CommandContext* newContext = g_contextManager.AllocateContext(CommandListType::Direct);
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

#if ENABLE_VULKAN_DEBUG_MARKUP
	if (!ID.empty())
	{
		newContext->BeginEvent(ID);
		newContext->m_hasPendingDebugEvent = true;
	}
#endif

	return *newContext;
}


void CommandContext::Finish(bool waitForCompletion)
{
	assert(m_type == CommandListType::Direct || m_type == CommandListType::Compute);

	FlushResourceBarriers();

	// TODO
#if 0
	if (m_ID.length() > 0)
	{
		EngineProfiling::EndBlock(this);
	}
#endif

#if ENABLE_VULKAN_DEBUG_MARKUP
	if (m_hasPendingDebugEvent)
	{
		EndEvent();
		m_hasPendingDebugEvent = false;
	}
#endif

	vkEndCommandBuffer(m_commandList);

	CommandQueue& Queue = g_commandManager.GetQueue(m_type);

	uint64_t fenceValue = Queue.ExecuteCommandList(m_commandList);
	Queue.DiscardCommandBuffer(fenceValue, m_commandList);
	m_commandList = VK_NULL_HANDLE;

	// Recycle dynamic allocations
	//m_vertexBufferAllocator.CleanupUsedBuffers(fence);
	//m_indexBufferAllocator.CleanupUsedBuffers(fence);
	//m_constantBufferAllocator.CleanupUsedBuffers(fence);

	if (waitForCompletion)
	{
		g_commandManager.WaitForFence(fenceValue);
	}

	g_contextManager.FreeContext(this);
}


void CommandContext::BeginEvent(const string& label)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	if (vkCmdBeginDebugUtilsLabel)
	{
		VkDebugUtilsLabelEXT labelInfo = {};
		labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		labelInfo.color[0] = 0.0f;
		labelInfo.color[1] = 0.0f;
		labelInfo.color[2] = 0.0f;
		labelInfo.color[3] = 0.0f;
		labelInfo.pLabelName = label.c_str();
		vkCmdBeginDebugUtilsLabel(m_commandList, &labelInfo);
	}
#endif
}


void CommandContext::EndEvent()
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	if (vkCmdEndDebugUtilsLabel)
	{
		vkCmdEndDebugUtilsLabel(m_commandList);
	}
#endif
}


void CommandContext::SetMarker(const string& label)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	if (vkCmdInsertDebugUtilsLabel)
	{
		VkDebugUtilsLabelEXT labelInfo = {};
		labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		labelInfo.color[0] = 0.0f;
		labelInfo.color[1] = 0.0f;
		labelInfo.color[2] = 0.0f;
		labelInfo.color[3] = 0.0f;
		labelInfo.pLabelName = label.c_str();
		vkCmdInsertDebugUtilsLabel(m_commandList, &labelInfo);
	}
#endif
}


void CommandContext::Initialize()
{
	assert(m_commandList == VK_NULL_HANDLE);

	m_commandList = g_commandManager.GetQueue(m_type).RequestCommandBuffer();
}


void CommandContext::InitializeTexture(Texture& dest, size_t numBytes, const void* initialData, uint32_t numBuffers, VkBufferImageCopy bufferCopies[])
{
	VkDevice device = GetDevice();
	VmaAllocator allocator = GetAllocator();

	VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stagingBufferInfo.size = numBytes;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo stagingAllocCreateInfo = {};
	stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	stagingAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
	VmaAllocationInfo stagingAllocInfo = {};
	ThrowIfFailed(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingAllocCreateInfo, &stagingBuffer, &stagingBufferAlloc, &stagingAllocInfo));

	memcpy(stagingAllocInfo.pMappedData, initialData, numBytes);

	// Upload to GPU
	CommandContext& context = CommandContext::Begin();

	context.TransitionResource(dest, ResourceState::CopyDest, true);

	vkCmdCopyBufferToImage(
		context.m_commandList,
		stagingBuffer,
		dest.m_image->Get(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		numBuffers,
		bufferCopies);

	context.TransitionResource(dest, ResourceState::ShaderResource);

	context.Finish(true);

	// Destroy staging buffer and memory
	vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAlloc);
}


void CommandContext::InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset, size_t offset)
{
	// TODO - handle device and allocator differently?  Maybe each CommandContext holds on to them directly?
	VkDevice device = GetDevice();
	VmaAllocator allocator = GetAllocator();

	VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stagingBufferInfo.size = numBytes;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo stagingAllocCreateInfo = {};
	stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	stagingAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
	VmaAllocationInfo stagingAllocInfo = {};
	ThrowIfFailed(vmaCreateBuffer(allocator, &stagingBufferInfo, &stagingAllocCreateInfo, &stagingBuffer, &stagingBufferAlloc, &stagingAllocInfo));

	memcpy(stagingAllocInfo.pMappedData, initialData, numBytes);

	// Upload to GPU
	CommandContext& context = CommandContext::Begin();

	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion = {};
	copyRegion.size = numBytes;
	vkCmdCopyBuffer(context.m_commandList, stagingBuffer, dest.m_buffer->Get(), 1, &copyRegion);

	context.Finish(true);

	// Destroy staging buffer and memory
	vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAlloc);
}


void CommandContext::TransitionResource(GpuBuffer& buffer, ResourceState newState, bool flushImmediate)
{
	assert_msg(newState != ResourceState::Undefined, "Can\'t transition to Undefined resource state");
	assert(!m_isRenderPassActive);

	ResourceState oldState = buffer.m_usageState;

	if (m_type == CommandListType::Compute)
	{
		assert(IsValidComputeResourceState(oldState));
		assert(IsValidComputeResourceState(newState));
	}

	if (oldState != newState)
	{
		assert(IsBufferResource(buffer.m_type));
		
		VkBufferMemoryBarrier barrierDesc = {};

		barrierDesc.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrierDesc.pNext = nullptr;
		barrierDesc.buffer = buffer.m_buffer->Get();
		barrierDesc.srcAccessMask = GetAccessMask(oldState);
		barrierDesc.dstAccessMask = GetAccessMask(newState);
		barrierDesc.offset = 0;
		barrierDesc.size = buffer.GetSize();

		auto srcStageMask = GetShaderStageMask(oldState, true);
		auto dstStageMask = GetShaderStageMask(newState, false);

		vkCmdPipelineBarrier(
			m_commandList,
			srcStageMask,
			dstStageMask,
			0,
			0,
			nullptr,
			1,
			&barrierDesc,
			0,
			nullptr);

		buffer.m_usageState = newState;
	}
}


void CommandContext::TransitionResource(GpuImage& image, ResourceState newState, bool flushImmediate)
{
	assert_msg(newState != ResourceState::Undefined, "Can\'t transition to Undefined resource state");
	assert(!m_isRenderPassActive);

	ResourceState oldState = image.m_usageState;

	if (m_type == CommandListType::Compute)
	{
		assert(IsValidComputeResourceState(oldState));
		assert(IsValidComputeResourceState(newState));
	}

	if (oldState != newState)
	{
		assert(IsTextureResource(image.m_type));

		PixelBuffer& texture = dynamic_cast<PixelBuffer&>(image);

		VkImageMemoryBarrier barrierDesc = {};

		barrierDesc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierDesc.pNext = nullptr;
		barrierDesc.image = image.m_image->Get();
		barrierDesc.oldLayout = GetImageLayout(oldState);
		barrierDesc.newLayout = GetImageLayout(newState);
		barrierDesc.subresourceRange.aspectMask = GetAspectFlagsFromFormat(texture.GetFormat());
		barrierDesc.subresourceRange.baseArrayLayer = 0;
		barrierDesc.subresourceRange.baseMipLevel = 0;
		barrierDesc.subresourceRange.layerCount = image.m_type == ResourceType::Texture3D ? 1 : texture.GetArraySize();
		barrierDesc.subresourceRange.levelCount = texture.GetNumMips();
		barrierDesc.srcAccessMask = GetAccessMask(oldState);
		barrierDesc.dstAccessMask = GetAccessMask(newState);

		auto srcStageMask = GetShaderStageMask(oldState, true);
		auto dstStageMask = GetShaderStageMask(newState, false);

		vkCmdPipelineBarrier(
			m_commandList,
			srcStageMask,
			dstStageMask,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrierDesc);

		image.m_usageState = newState;
	}
}


void CommandContext::InsertUAVBarrier(GpuBuffer& buffer, bool flushImmediate)
{
	//assert_msg(m_numBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");

	VkBufferMemoryBarrier barrierDesc = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
	barrierDesc.pNext = nullptr;
	barrierDesc.buffer = buffer.m_buffer->Get();
	barrierDesc.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrierDesc.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrierDesc.offset = 0;
	barrierDesc.size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(
		m_commandList,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0,
		nullptr,
		1,
		&barrierDesc,
		0,
		nullptr);
}


void CommandContext::Reset()
{
	assert(m_commandList == VK_NULL_HANDLE);

	m_commandList = g_commandManager.GetQueue(m_type).RequestCommandBuffer();

	m_curGraphicsPipelineLayout = VK_NULL_HANDLE;
	m_curComputePipelineLayout = VK_NULL_HANDLE;
}


void GraphicsContext::BeginRenderPass(FrameBuffer& framebuffer)
{
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = framebuffer.GetRenderPass();

	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = framebuffer.GetWidth();
	renderPassBeginInfo.renderArea.extent.height = framebuffer.GetHeight();

	renderPassBeginInfo.clearValueCount = 0;
	renderPassBeginInfo.pClearValues = nullptr;
	renderPassBeginInfo.framebuffer = framebuffer.GetFramebuffer();

	vkCmdBeginRenderPass(m_commandList, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_isRenderPassActive = true;
}


void GraphicsContext::BeginOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex)
{
	vkCmdBeginQuery(m_commandList, queryHeap.GetQueryPool(), heapIndex, VK_FLAGS_NONE);
}


void GraphicsContext::EndOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex)
{
	vkCmdEndQuery(m_commandList, queryHeap.GetQueryPool(), heapIndex);
}


void GraphicsContext::ResolveOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries, GpuBuffer& destBuffer, uint64_t destBufferOffset)
{
	assert(!m_isRenderPassActive);

	vkCmdCopyQueryPoolResults(
		m_commandList,
		queryHeap.GetQueryPool(),
		startIndex,
		numQueries,
		destBuffer.m_buffer->Get(),
		destBufferOffset,
		sizeof(uint64_t), // TODO - don't hardcode this
		VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
}


void GraphicsContext::ResetOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries)
{
	assert(!m_isRenderPassActive);

	vkCmdResetQueryPool(m_commandList, queryHeap.GetQueryPool(), startIndex, numQueries);
}


void GraphicsContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curComputePipelineLayout = VK_NULL_HANDLE;
	m_curGraphicsPipelineLayout = rootSig.GetLayout();

	uint32_t i = 0;
	for (i = 0; i < rootSig.GetNumParameters(); ++i)
	{
		m_shaderStages[i] = static_cast<VkShaderStageFlags>(rootSig[i].GetShaderVisibility());
	}
	for (; i < 8; ++i)
	{
		m_shaderStages[i] = VK_SHADER_STAGE_ALL;
	}
}


void GraphicsContext::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
	VkViewport viewport = {};
	viewport.x = x;
	viewport.y = h;
	viewport.height = -h;
	viewport.width = w;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	vkCmdSetViewport(m_commandList, 0, 1, &viewport);
}


void GraphicsContext::SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	VkRect2D scissor = {};
	scissor.extent.width = right - left;
	scissor.extent.height = bottom - top;
	scissor.offset.x = static_cast<int32_t>(left);
	scissor.offset.y = static_cast<int32_t>(top);
	vkCmdSetScissor(m_commandList, 0, 1, &scissor);
}


void GraphicsContext::SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	VkViewport vp{ (float)x, (float)h, (float)w, -1.0f * (float)h, 0.0f, 1.0f };
	VkRect2D rect;
	rect.extent.width = w;
	rect.extent.height = h;
	rect.offset.x = static_cast<int32_t>(x);
	rect.offset.y = static_cast<int32_t>(y);
	vkCmdSetViewport(m_commandList, 0, 1, &vp);
	vkCmdSetScissor(m_commandList, 0, 1, &rect);
}


void GraphicsContext::SetPipelineState(const GraphicsPSO& pso)
{
	vkCmdBindPipeline(m_commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetHandle());
}


void ComputeContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curGraphicsPipelineLayout = VK_NULL_HANDLE;
	m_curComputePipelineLayout = rootSig.GetLayout();

	for (uint32_t i = 0; i < 8; ++i)
	{
		m_shaderStages[i] = VK_SHADER_STAGE_COMPUTE_BIT;
	}
}


void ComputeContext::SetPipelineState(const ComputePSO& pso)
{
	vkCmdBindPipeline(m_commandList, VK_PIPELINE_BIND_POINT_COMPUTE, pso.GetHandle());
}