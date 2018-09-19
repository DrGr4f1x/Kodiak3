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

#include "GraphicsDevice.h"
#include "PipelineState.h"
#include "QueryHeap.h"

#include "CommandListManagerVk.h"
#include "RootSignatureVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


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
	, m_dynamicDescriptorPool(*this)
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	
	ThrowIfFailed(vkCreateSemaphore(GetDevice(), &semaphoreCreateInfo, nullptr, &m_signalSemaphore));
}


CommandContext::~CommandContext()
{
	vkDestroySemaphore(GetDevice(), m_signalSemaphore, nullptr);
}


void CommandContext::DestroyAllContexts()
{
	// TODO
#if 0
	LinearAllocator::DestroyAll();
#endif
	DynamicDescriptorPool::DestroyAll();
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

	return *newContext;
}

#pragma optimize("",off)
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

	vkEndCommandBuffer(m_commandList);

	CommandQueue& Queue = g_commandManager.GetQueue(m_type);

	auto fence = Queue.ExecuteCommandList(m_commandList, waitForCompletion ? VK_NULL_HANDLE : m_signalSemaphore);
	Queue.DiscardCommandBuffer(fence, m_commandList);
	m_commandList = VK_NULL_HANDLE;

	// Recycle dynamic allocations
	//m_vertexBufferAllocator.CleanupUsedBuffers(fence);
	//m_indexBufferAllocator.CleanupUsedBuffers(fence);
	//m_constantBufferAllocator.CleanupUsedBuffers(fence);
	m_dynamicDescriptorPool.CleanupUsedPools(fence);

	if (waitForCompletion)
	{
		g_commandManager.WaitForFence(fence);
	}

	g_contextManager.FreeContext(this);
}
#pragma optimize("",on)


void CommandContext::Initialize()
{
	assert(m_commandList == VK_NULL_HANDLE);

	m_commandList = g_commandManager.GetQueue(m_type).RequestCommandBuffer();
}


void CommandContext::InitializeTexture(Texture& dest, size_t numBytes, const void* initData, uint32_t numBuffers, VkBufferImageCopy bufferCopies[])
{
	VkDevice device = GetDevice();

	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = numBytes;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer stagingBuffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(device, &stagingBufferInfo, nullptr, &stagingBuffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data = nullptr;

	// Copy to staging buffer with Map/Unmap
	VkDeviceMemory stagingBufferMemory{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingBufferMemory));
	ThrowIfFailed(vkMapMemory(device, stagingBufferMemory, 0, numBytes, 0, &data));
	memcpy(data, initData, numBytes);
	vkUnmapMemory(device, stagingBufferMemory);
	ThrowIfFailed(vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0));

	// Upload to GPU
	CommandContext& context = CommandContext::Begin();

	context.TransitionResource(dest, ResourceState::CopyDest, true);

	vkCmdCopyBufferToImage(
		context.m_commandList,
		stagingBuffer,
		dest.m_resource,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		numBuffers,
		bufferCopies);

	context.TransitionResource(dest, ResourceState::GenericRead);

	context.Finish(true);

	// Destroy staging buffer and memory
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


void CommandContext::InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset, size_t offset)
{
	VkDevice device = GetDevice();

	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = numBytes;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer stagingBuffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(device, &stagingBufferInfo, nullptr, &stagingBuffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data = nullptr;

	// Copy to staging buffer with Map/Unmap
	VkDeviceMemory stagingBufferMemory{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingBufferMemory));
	ThrowIfFailed(vkMapMemory(device, stagingBufferMemory, 0, numBytes, 0, &data));
	memcpy(data, initialData, numBytes);
	vkUnmapMemory(device, stagingBufferMemory);
	ThrowIfFailed(vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0));

	// Upload to GPU
	CommandContext& context = CommandContext::Begin();

	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion = {};
	copyRegion.size = numBytes;
	vkCmdCopyBuffer(context.m_commandList, stagingBuffer, dest.m_resource, 1, &copyRegion);

	context.Finish(true);

	// Destroy staging buffer and memory
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


void CommandContext::TransitionResource(GpuResource& resource, ResourceState newState, bool flushImmediate)
{
	assert_msg(newState != ResourceState::Undefined, "Can\'t transition to Undefined resource state");
	assert(!m_isRenderPassActive);

	ResourceState oldState = resource.m_usageState;

	if (m_type == CommandListType::Compute)
	{
		assert(IsValidComputeResourceState(oldState));
		assert(IsValidComputeResourceState(newState));
	}

	if (oldState != newState)
	{
		if (IsTextureResource(resource.m_type))
		{
			PixelBuffer& texture = dynamic_cast<PixelBuffer&>(resource);

			VkImageMemoryBarrier barrierDesc = {};

			barrierDesc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrierDesc.pNext = nullptr;
			barrierDesc.image = resource.m_resource;
			barrierDesc.oldLayout = GetImageLayout(oldState);
			barrierDesc.newLayout = GetImageLayout(newState);
			barrierDesc.subresourceRange.aspectMask = GetAspectFlagsFromFormat(texture.GetFormat());
			barrierDesc.subresourceRange.baseArrayLayer = 0;
			barrierDesc.subresourceRange.baseMipLevel = 0;
			barrierDesc.subresourceRange.layerCount = resource.m_type == ResourceType::Texture3D ? 1 : texture.GetArraySize();
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

			resource.m_usageState = newState;
		}
		else if (IsBufferResource(resource.m_type))
		{
			GpuBuffer& buffer = dynamic_cast<GpuBuffer&>(resource);

			VkBufferMemoryBarrier barrierDesc = {};

			barrierDesc.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrierDesc.pNext = nullptr;
			barrierDesc.buffer = resource.m_resource;
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

			resource.m_usageState = newState;
		}
		else
		{
			assert(false);
		}
	}
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
	renderPassBeginInfo.renderPass = framebuffer.GetFboHandle();

	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = framebuffer.GetWidth();
	renderPassBeginInfo.renderArea.extent.height = framebuffer.GetHeight();

	renderPassBeginInfo.clearValueCount = 0;
	renderPassBeginInfo.pClearValues = nullptr;
	renderPassBeginInfo.framebuffer = framebuffer.GetFboHandle();

	vkCmdBeginRenderPass(m_commandList, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_isRenderPassActive = true;
}


void GraphicsContext::BeginOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex)
{
	vkCmdBeginQuery(m_commandList, queryHeap.GetHandle(), heapIndex, VK_FLAGS_NONE);
}


void GraphicsContext::EndOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex)
{
	vkCmdEndQuery(m_commandList, queryHeap.GetHandle(), heapIndex);
}


void GraphicsContext::ResolveOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries, GpuResource& destBuffer, uint64_t destBufferOffset)
{
	assert(!m_isRenderPassActive);

	vkCmdCopyQueryPoolResults(
		m_commandList,
		queryHeap.GetHandle(),
		startIndex,
		numQueries,
		destBuffer.GetHandle(),
		destBufferOffset,
		sizeof(uint64_t), // TODO - don't hardcode this
		VK_QUERY_RESULT_64_BIT);
}


void GraphicsContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curComputePipelineLayout = VK_NULL_HANDLE;
	m_curGraphicsPipelineLayout = rootSig.GetLayout();

	m_dynamicDescriptorPool.ParseGraphicsRootSignature(rootSig);
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


void GraphicsContext::SetRootConstantBuffer(uint32_t rootIndex, ConstantBuffer& constantBuffer, uint32_t dynamicOffset)
{
	VkDescriptorBufferInfo bufferInfo = constantBuffer.GetCBV().GetHandle();
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, 0, 1, &bufferInfo, dynamicOffset);
}


void ComputeContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curGraphicsPipelineLayout = VK_NULL_HANDLE;
	m_curComputePipelineLayout = rootSig.GetLayout();

	m_dynamicDescriptorPool.ParseComputeRootSignature(rootSig);
}


void ComputeContext::SetPipelineState(const ComputePSO& pso)
{
	vkCmdBindPipeline(m_commandList, VK_PIPELINE_BIND_POINT_COMPUTE, pso.GetHandle());
}


void ComputeContext::SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer, uint32_t dynamicOffset)
{
	VkDescriptorBufferInfo bufferInfo = constantBuffer.GetCBV().GetHandle();
	m_dynamicDescriptorPool.SetComputeDescriptorHandles(rootIndex, 0, 1, &bufferInfo, dynamicOffset);
}