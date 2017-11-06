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

#include "CommandContext12.h"

#include "CommandListManager12.h"
#include "Framebuffer12.h"
#include "RenderPass12.h"


using namespace Kodiak;
using namespace std;


namespace
{

ContextManager g_contextManager;

const ResourceState VALID_COMPUTE_QUEUE_RESOURCE_STATES =
	ResourceState::NonPixelShaderResource | ResourceState::UnorderedAccess | ResourceState::CopyDest | ResourceState::CopySource;

} // anonymous namespace


CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE type)
{
	lock_guard<mutex> lockGuard(sm_contextAllocationMutex);

	auto& availableContexts = sm_availableContexts[type];

	CommandContext* ret = nullptr;
	if (availableContexts.empty())
	{
		ret = new CommandContext(type);
		sm_contextPool[type].emplace_back(ret);
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
	lock_guard<mutex> CS(sm_contextAllocationMutex);

	sm_availableContexts[usedContext->m_type].push(usedContext);
}


void ContextManager::DestroyAllContexts()
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		sm_contextPool[i].clear();
	}
}


CommandContext::~CommandContext()
{
	if (m_commandList != nullptr)
	{
		m_commandList->Release();
		m_commandList = nullptr;
	}
}


void CommandContext::DestroyAllContexts()
{
	LinearAllocator::DestroyAll();
	// TODO
#if 0
	DynamicDescriptorHeap::DestroyAll();
#endif
	g_contextManager.DestroyAllContexts();
}


CommandContext& CommandContext::Begin(const string id)
{
	auto newContext = g_contextManager.AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
	newContext->SetID(id);
	// TODO
#if 0
	if (id.length() > 0)
		EngineProfiling::BeginBlock(id, newContext);
#endif
	return *newContext;
}


uint64_t CommandContext::Finish(bool waitForCompletion)
{
	assert(m_type == D3D12_COMMAND_LIST_TYPE_DIRECT || m_type == D3D12_COMMAND_LIST_TYPE_COMPUTE);

	FlushResourceBarriers();

	// TODO
#if 0
	if (m_ID.length() > 0)
	{
		EngineProfiling::EndBlock(this);
	}
#endif

	assert(m_currentAllocator != nullptr);


	CommandQueue& cmdQueue = g_commandManager.GetQueue(m_type);

	uint64_t fenceValue = cmdQueue.ExecuteCommandList(m_commandList);
	cmdQueue.DiscardAllocator(fenceValue, m_currentAllocator);
	m_currentAllocator = nullptr;

	m_cpuLinearAllocator.CleanupUsedPages(fenceValue);
	m_gpuLinearAllocator.CleanupUsedPages(fenceValue);

	// TODO
#if 0
	m_DynamicViewDescriptorHeap.CleanupUsedHeaps(FenceValue);
	m_DynamicSamplerDescriptorHeap.CleanupUsedHeaps(FenceValue);
#endif

	if (waitForCompletion)
	{
		g_commandManager.WaitForFence(fenceValue);
	}

	g_contextManager.FreeContext(this);

	return fenceValue;
}


void CommandContext::Initialize()
{
	g_commandManager.CreateNewCommandList(m_type, &m_commandList, &m_currentAllocator);
}


void CommandContext::InitializeBuffer(GpuResource& dest, const void* bufferData, size_t numBytes, size_t offset)
{
	CommandContext& initContext = CommandContext::Begin();

	DynAlloc mem = initContext.ReserveUploadMemory(numBytes);
	SIMDMemCopy(mem.dataPtr, bufferData, Math::DivideByMultiple(numBytes, 16));

	// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
	initContext.TransitionResource(dest, ResourceState::CopyDest, true);
	initContext.m_commandList->CopyBufferRegion(dest.GetResource(), offset, mem.buffer.GetResource(), 0, numBytes);
	initContext.TransitionResource(dest, ResourceState::GenericRead, true);

	// Execute the command list and wait for it to finish so we can release the upload buffer
	initContext.Finish(true);
}


void CommandContext::TransitionResource(GpuResource& resource, ResourceState newState, bool flushImmediate)
{
	ResourceState oldState = resource.m_usageState;

	if (m_type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		assert((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
		assert((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
	}

	if (oldState != newState)
	{
		assert_msg(m_numBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& barrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = resource.GetResource();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(oldState);
		barrierDesc.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(newState);

		// Check to see if we already started the transition
		if (newState == resource.m_transitioningState)
		{
			barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			resource.m_transitioningState = ResourceState::Undefined;
		}
		else
		{
			barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		}

		resource.m_usageState = newState;
	}
	else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		InsertUAVBarrier(resource, flushImmediate);
	}

	if (flushImmediate || m_numBarriersToFlush == 16)
	{
		FlushResourceBarriers();
	}
}


void CommandContext::BeginResourceTransition(GpuResource& resource, ResourceState newState, bool flushImmediate)
{
	// If it's already transitioning, finish that transition
	if (resource.m_transitioningState != (D3D12_RESOURCE_STATES)-1)
	{
		TransitionResource(resource, resource.m_transitioningState);
	}

	ResourceState oldState = resource.m_usageState;

	if (oldState != newState)
	{
		assert_msg(m_numBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& barrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = resource.GetResource();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(oldState);
		barrierDesc.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(newState);

		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

		resource.m_transitioningState = newState;
	}

	if (flushImmediate || m_numBarriersToFlush == 16)
	{
		FlushResourceBarriers();
	}
}


void CommandContext::InsertUAVBarrier(GpuResource& resource, bool flushImmediate)
{
	assert_msg(m_numBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
	D3D12_RESOURCE_BARRIER& barrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.UAV.pResource = resource.GetResource();

	if (flushImmediate)
	{
		FlushResourceBarriers();
	}
}


void CommandContext::InsertAliasBarrier(GpuResource& before, GpuResource& after, bool flushImmediate)
{
	assert_msg(m_numBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
	D3D12_RESOURCE_BARRIER& barrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Aliasing.pResourceBefore = before.GetResource();
	barrierDesc.Aliasing.pResourceAfter = after.GetResource();

	if (flushImmediate)
	{
		FlushResourceBarriers();
	}
}


CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
	: m_type(type)
	// TODO
	//, m_DynamicViewDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
	//, m_DynamicSamplerDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
	, m_cpuLinearAllocator(kCpuWritable)
	, m_gpuLinearAllocator(kGpuExclusive)
{
	m_owningManager = nullptr;
	m_commandList = nullptr;
	m_currentAllocator = nullptr;

	// TODO
#if 0
	ZeroMemory(m_CurrentDescriptorHeaps, sizeof(m_CurrentDescriptorHeaps));
#endif

	m_curGraphicsRootSignature = nullptr;
	m_curGraphicsPipelineState = nullptr;
	m_curComputeRootSignature = nullptr;
	m_curComputePipelineState = nullptr;
	m_numBarriersToFlush = 0;
}


void CommandContext::Reset()
{
	// We only call Reset() on previously freed contexts.  The command list persists, but we must
	// request a new allocator.
	assert(m_commandList != nullptr && m_currentAllocator == nullptr);
	m_currentAllocator = g_commandManager.GetQueue(m_type).RequestAllocator();
	m_commandList->Reset(m_currentAllocator, nullptr);

	m_curGraphicsRootSignature = nullptr;
	m_curGraphicsPipelineState = nullptr;
	m_curComputeRootSignature = nullptr;
	m_curComputePipelineState = nullptr;
	m_numBarriersToFlush = 0;

	m_curPrimitiveTopology = (D3D12_PRIMITIVE_TOPOLOGY)-1;

	// TODO
#if 0
	BindDescriptorHeaps();
#endif
}


void GraphicsContext::ClearColor(ColorBuffer& target)
{
	FlushResourceBarriers();
	m_commandList->ClearRenderTargetView(target.GetRTV(), target.GetClearColor().GetPtr(), 0, nullptr);
}


void GraphicsContext::ClearDepth(DepthBuffer& target)
{
	FlushResourceBarriers();
	m_commandList->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, target.GetClearDepth(), target.GetClearStencil(), 0, nullptr);
}


void GraphicsContext::ClearStencil(DepthBuffer& target)
{
	FlushResourceBarriers();
	m_commandList->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_STENCIL, target.GetClearDepth(), target.GetClearStencil(), 0, nullptr);
}


void GraphicsContext::ClearDepthAndStencil(DepthBuffer& target)
{
	FlushResourceBarriers();
	m_commandList->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, target.GetClearDepth(), target.GetClearStencil(), 0, nullptr);
}


void GraphicsContext::BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor)
{
	// Verify the renderpass and framebuffer are compatible
	assert(pass.GetNumColorAttachments() == framebuffer.GetNumColorBuffers());

	assert(!pass.HasDepthAttachment());
	assert(!framebuffer.HasDepthBuffer());

	array<D3D12_CPU_DESCRIPTOR_HANDLE, 8> RTVs;

	m_numRenderTargets = (uint32_t)pass.GetNumColorAttachments();

	// Gather render targets for EndRenderPass
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		m_renderTargets[i] = framebuffer.GetColorBuffer(i).get();
		m_renderTargetStates[i] = pass.GetFinalColorState(i);
	}
	m_depthTargetValid = false;

	// Transition resources to initial states
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		RTVs[i] = m_renderTargets[i]->GetRTV();
		TransitionResource(*m_renderTargets[i], ResourceState::RenderTarget);
	}

	FlushResourceBarriers();

	// Set the render targets
	m_commandList->OMSetRenderTargets(m_numRenderTargets, &RTVs[0], FALSE, nullptr);

	// Clear render targets
	float rgba[4];
	rgba[0] = clearColor.R();
	rgba[1] = clearColor.G();
	rgba[2] = clearColor.B();
	rgba[3] = clearColor.A();
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		m_commandList->ClearRenderTargetView(RTVs[i], rgba, 0, nullptr);
	}
}


void GraphicsContext::BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor, float clearDepth, uint32_t clearStencil)
{
	// Verify the renderpass and framebuffer are compatible
	assert(pass.GetNumColorAttachments() == framebuffer.GetNumColorBuffers());

	assert(pass.HasDepthAttachment());
	assert(framebuffer.HasDepthBuffer());

	array<D3D12_CPU_DESCRIPTOR_HANDLE, 8> RTVs;
	D3D12_CPU_DESCRIPTOR_HANDLE DSV;

	m_numRenderTargets = (uint32_t)pass.GetNumColorAttachments();

	// Gather render targets for EndRenderPass
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		m_renderTargets[i] = framebuffer.GetColorBuffer(i).get();
		m_renderTargetStates[i] = pass.GetFinalColorState(i);
	}
	m_depthTarget = framebuffer.GetDepthBuffer().get();
	m_depthTargetState = pass.GetFinalDepthState();
	m_depthTargetValid = true;

	// Transition resources to initial states
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		RTVs[i] = m_renderTargets[i]->GetRTV();
		TransitionResource(*m_renderTargets[i], ResourceState::RenderTarget);
	}
	DSV = m_depthTarget->GetDSV();
	TransitionResource(*m_depthTarget, ResourceState::DepthWrite);

	FlushResourceBarriers();

	// Set the render targets
	m_commandList->OMSetRenderTargets(m_numRenderTargets, &RTVs[0], FALSE, &DSV);

	// Clear render targets
	float rgba[4];
	rgba[0] = clearColor.R();
	rgba[1] = clearColor.G();
	rgba[2] = clearColor.B();
	rgba[3] = clearColor.A();
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		m_commandList->ClearRenderTargetView(RTVs[i], rgba, 0, nullptr);
	}
	m_commandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearDepth, (UINT8)clearStencil, 0, nullptr);
}


void GraphicsContext::EndRenderPass()
{
	// Transition render targets to final states
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		TransitionResource(*m_renderTargets[i], m_renderTargetStates[i]);
	}
	if (m_depthTargetValid)
	{
		TransitionResource(*m_depthTarget, m_depthTargetState);
	}

	FlushResourceBarriers();

	// Clear cached render targets
	for (uint32_t i = 0; i < m_numRenderTargets; ++i)
	{
		m_renderTargets[i] = nullptr;
	}
	if (m_depthTargetValid)
	{
		m_depthTarget = nullptr;
	}
	m_numRenderTargets = 0;
	m_depthTargetValid = false;
}


void GraphicsContext::SetRenderTarget(const ColorBuffer& colorBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = { colorBuffer.GetRTV() };
	m_commandList->OMSetRenderTargets(1, rtvs, FALSE, nullptr);
}


void GraphicsContext::SetRenderTarget(const ColorBuffer& colorBuffer, const DepthBuffer& depthBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = { colorBuffer.GetRTV() };
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = depthBuffer.GetDSV();
	m_commandList->OMSetRenderTargets(1, rtvs, FALSE, &dsv);
}


void GraphicsContext::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
	D3D12_VIEWPORT vp;
	vp.Width = w;
	vp.Height = h;
	vp.MinDepth = minDepth;
	vp.MaxDepth = maxDepth;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	m_commandList->RSSetViewports(1, &vp);
}


void GraphicsContext::SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	auto rect = CD3DX12_RECT(left, top, right, bottom);

	assert(rect.left < rect.right && rect.top < rect.bottom);

	m_commandList->RSSetScissorRects(1, &rect);
}


void GraphicsContext::SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	SetViewport((float)x, (float)y, (float)w, (float)h);
	SetScissor(x, y, x + w, y + h);
}