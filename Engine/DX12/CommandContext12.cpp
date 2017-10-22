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


using namespace Kodiak;
using namespace std;


#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )


namespace
{

ContextManager g_contextManager;

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

	// TODO
#if 0
	m_CpuLinearAllocator.CleanupUsedPages(FenceValue);
	m_GpuLinearAllocator.CleanupUsedPages(FenceValue);
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
	initContext.TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	initContext.m_commandList->CopyBufferRegion(dest.GetResource(), offset, mem.buffer.GetResource(), 0, numBytes);
	initContext.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	// Execute the command list and wait for it to finish so we can release the upload buffer
	initContext.Finish(true);
}


void CommandContext::TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
{
	D3D12_RESOURCE_STATES oldState = resource.m_usageState;

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
		barrierDesc.Transition.StateBefore = oldState;
		barrierDesc.Transition.StateAfter = newState;

		// Check to see if we already started the transition
		if (newState == resource.m_transitioningState)
		{
			barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			resource.m_transitioningState = (D3D12_RESOURCE_STATES)-1;
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


void CommandContext::BeginResourceTransition(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
{
	// If it's already transitioning, finish that transition
	if (resource.m_transitioningState != (D3D12_RESOURCE_STATES)-1)
	{
		TransitionResource(resource, resource.m_transitioningState);
	}

	D3D12_RESOURCE_STATES oldState = resource.m_usageState;

	if (oldState != newState)
	{
		assert_msg(m_numBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& barrierDesc = m_resourceBarrierBuffer[m_numBarriersToFlush++];

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = resource.GetResource();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = oldState;
		barrierDesc.Transition.StateAfter = newState;

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

	m_CurGraphicsRootSignature = nullptr;
	m_CurGraphicsPipelineState = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurComputePipelineState = nullptr;
#endif
	m_numBarriersToFlush = 0;
}


void CommandContext::Reset()
{
	// We only call Reset() on previously freed contexts.  The command list persists, but we must
	// request a new allocator.
	assert(m_commandList != nullptr && m_currentAllocator == nullptr);
	m_currentAllocator = g_commandManager.GetQueue(m_type).RequestAllocator();
	m_commandList->Reset(m_currentAllocator, nullptr);

	m_numBarriersToFlush = 0;

	// TODO
#if 0
	m_CurGraphicsRootSignature = nullptr;
	m_CurGraphicsPipelineState = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurComputePipelineState = nullptr;
	

	BindDescriptorHeaps();
#endif
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