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

#include "CommandListManagerVk.h"
#include "GraphicsDeviceVk.h"
#include "PipelineStateVk.h"
#include "RenderPassVk.h"
#include "RootSignatureVk.h"


using namespace Kodiak;
using namespace std;


ContextManager g_contextManager;

static const ResourceState VALID_COMPUTE_QUEUE_RESOURCE_STATES =
ResourceState::NonPixelShaderResource | ResourceState::UnorderedAccess | ResourceState::CopyDest | ResourceState::CopySource;


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

	FlushImageBarriers();
	//FlushBufferBarriers();

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


void CommandContext::InitializeTexture(Texture& dest, const void* initialData, size_t numBytes)
{
	auto device = GetDevice();

	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.size = numBytes;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer stagingBuffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(GetDevice(), &stagingBufferInfo, nullptr, &stagingBuffer));

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

	context.TransitionResource(dest, ResourceState::CopyDest, true);

	// Copy single mip level from staging buffer to destination texture
	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = dest.GetWidth();
	bufferCopyRegion.imageExtent.height = dest.GetHeight();
	bufferCopyRegion.imageExtent.depth = 1;
	bufferCopyRegion.bufferOffset = 0;

	vkCmdCopyBufferToImage(
		context.m_commandList,
		stagingBuffer,
		dest.m_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferCopyRegion);

	context.TransitionResource(dest, ResourceState::GenericRead);

	context.Finish(true);

	// Destroy staging buffer and memory
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


void CommandContext::InitializeBuffer(GpuBuffer& dest, const void* initialData, size_t numBytes, bool useOffset, size_t offset)
{
	auto device = GetDevice();

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
	vkCmdCopyBuffer(context.m_commandList, stagingBuffer, dest.GetBuffer(), 1, &copyRegion);

	context.Finish(true);

	// Destroy staging buffer and memory
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


void CommandContext::TransitionResource(Texture& texture, ResourceState newState, bool flushImmediate)
{
	assert_msg(newState != ResourceState::Undefined, "Can\'t transition to Undefined resource state");

	ResourceState oldState = texture.m_usageState;

	if (m_type == CommandListType::Compute)
	{
		assert((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
		assert((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
	}

	if (oldState != newState)
	{
		assert_msg(m_numImageBarriersToFlush < 16, "Exceeded arbitrary limit on buffered image barriers");
		VkImageMemoryBarrier& barrierDesc = m_pendingImageBarriers[m_numImageBarriersToFlush++];

		barrierDesc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierDesc.pNext = nullptr;

		barrierDesc.image = texture.m_image;

		// Setup access flags and layout 
		barrierDesc.srcAccessMask = texture.m_accessFlags;
		barrierDesc.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		switch (barrierDesc.oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Only valid as initial layout, memory contents are not preserved
			// Can be accessed directly, no source dependency required
			barrierDesc.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes to the image have been finished
			barrierDesc.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Old layout is transfer destination
			// Make sure any writes to the image have been finished
			barrierDesc.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		}

		switch (newState)
		{
		case ResourceState::Clear:
			barrierDesc.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrierDesc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			break;
		case ResourceState::RenderTarget:
			barrierDesc.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrierDesc.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case ResourceState::PixelShaderResource:
		case ResourceState::NonPixelShaderResource:
		case ResourceState::GenericRead:
			// Shader read (sampler, input attachment)
			barrierDesc.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrierDesc.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case ResourceState::Present:
			barrierDesc.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrierDesc.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		case ResourceState::CopyDest:
			barrierDesc.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrierDesc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			break;
		default:
			assert(false);
			break;
		}

		barrierDesc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrierDesc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		// TODO this likely isn't correct
		barrierDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrierDesc.subresourceRange.baseMipLevel = 0;
		barrierDesc.subresourceRange.levelCount = 1;
		barrierDesc.subresourceRange.baseArrayLayer = 0;
		barrierDesc.subresourceRange.layerCount = 1;

		texture.m_layout = barrierDesc.newLayout;
		texture.m_accessFlags = barrierDesc.dstAccessMask;
		texture.m_usageState = newState;
	}
	else if (newState == ResourceState::UnorderedAccess)
	{
		// TODO
		assert(false);
#if 0
		InsertUAVBarrier(Resource, FlushImmediate);
#endif
	}

	if (flushImmediate || m_numImageBarriersToFlush == 16)
	{
		FlushImageBarriers();
	}
}


void CommandContext::Reset()
{
	assert(m_commandList == VK_NULL_HANDLE);

	m_commandList = g_commandManager.GetQueue(m_type).RequestCommandBuffer();

	m_curGraphicsPipelineLayout = VK_NULL_HANDLE;
	m_curComputePipelineLayout = VK_NULL_HANDLE;
}


void GraphicsContext::BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor)
{
	VkClearValue clearValue;
	clearValue.color = { clearColor.R(), clearColor.G(), clearColor.B(), clearColor.A() };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = pass.GetRenderPass();

	// TODO: Pass the renderArea rectangle in as a parameter?
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = framebuffer.GetWidth();
	renderPassBeginInfo.renderArea.extent.height = framebuffer.GetHeight();

	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;
	renderPassBeginInfo.framebuffer = framebuffer.GetFramebuffer();

	vkCmdBeginRenderPass(m_commandList, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void GraphicsContext::BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor, float clearDepth, uint32_t clearStencil)
{
	VkClearValue clearValues[2];
	clearValues[0].color = { clearColor.R(), clearColor.G(), clearColor.B(), clearColor.A() };
	clearValues[1].depthStencil = { clearDepth, clearStencil };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = pass.GetRenderPass();

	// TODO: Pass the renderArea rectangle in as a parameter?
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = framebuffer.GetWidth();
	renderPassBeginInfo.renderArea.extent.height = framebuffer.GetHeight();

	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = framebuffer.GetFramebuffer();

	vkCmdBeginRenderPass(m_commandList, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void GraphicsContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curGraphicsPipelineLayout = rootSig.GetLayout();

	m_dynamicDescriptorPool.ParseGraphicsRootSignature(rootSig);
}


void GraphicsContext::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
	VkViewport viewport = {};
	viewport.x = x;
	viewport.y = y;
	viewport.height = h;
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
	VkViewport vp{ (float)x, (float)y, (float)w, (float)h, 0.0f, 1.0f };
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
	vkCmdBindPipeline(m_commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetPipelineStateObject());
}


void GraphicsContext::SetRootConstantBuffer(uint32_t rootIndex, ConstantBuffer& constantBuffer)
{
	VkDescriptorBufferInfo bufferInfo = constantBuffer.GetDescriptorInfo();
	m_dynamicDescriptorPool.SetGraphicsDescriptorHandles(rootIndex, 0, 1, &bufferInfo);
}