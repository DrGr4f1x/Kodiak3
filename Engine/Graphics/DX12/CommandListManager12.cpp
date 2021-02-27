//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard
//

#include "Stdafx.h"

#include "CommandListManager12.h"

#include "Graphics\GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{
CommandListManager g_commandManager;
} // namespace Kodiak


CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type)
	: m_type(type)
	, m_commandQueue(nullptr)
	, m_fence(nullptr)
	, m_nextFenceValue((uint64_t)type << 56 | 1)
	, m_lastCompletedFenceValue((uint64_t)type << 56)
	, m_fenceEventHandle(nullptr)
	, m_allocatorPool(type)
{}


CommandQueue::~CommandQueue()
{
	Destroy();
}


void CommandQueue::Create()
{
	assert(!IsReady());
	assert(m_allocatorPool.Size() == 0);

	auto device = GetDevice();

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = m_type;
	queueDesc.NodeMask = 1;
	ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	SetDebugName(m_commandQueue, "CommandListManager::m_commandQueue");

	assert_succeeded(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

	SetDebugName(m_fence, "CommandListManager::m_fence");

	m_fence->Signal((uint64_t)m_type << 56);

	m_fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
	assert(m_fenceEventHandle != INVALID_HANDLE_VALUE);

	assert(IsReady());
}


void CommandQueue::Destroy()
{
	if (m_commandQueue == nullptr)
	{
		return;
	}

	m_allocatorPool.Shutdown();

	CloseHandle(m_fenceEventHandle);
	m_fenceEventHandle = nullptr;

	m_fence->Release();
	m_fence = nullptr;

	m_commandQueue->Release();
	m_commandQueue = nullptr;
}


uint64_t CommandQueue::IncrementFence()
{
	lock_guard<mutex> lockGuard(m_fenceMutex);

	m_commandQueue->Signal(m_fence, m_nextFenceValue);

	return m_nextFenceValue++;
}


bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	// Avoid querying the fence value by testing against the last one seen.
	// The max() is to protect against an unlikely race condition that could cause the last
	// completed fence value to regress.
	if (fenceValue > m_lastCompletedFenceValue)
	{
		m_lastCompletedFenceValue = std::max(m_lastCompletedFenceValue, m_fence->GetCompletedValue());
	}

	return fenceValue <= m_lastCompletedFenceValue;
}


void CommandQueue::StallForFence(uint64_t fenceValue)
{
	CommandQueue& producer = g_commandManager.GetQueue((CommandListType)(fenceValue >> 56));
	m_commandQueue->Wait(producer.m_fence, fenceValue);
}


void CommandQueue::StallForProducer(CommandQueue& producer)
{
	assert(producer.m_nextFenceValue > 0);
	m_commandQueue->Wait(producer.m_fence, producer.m_nextFenceValue - 1);
}


void CommandQueue::WaitForFence(uint64_t fenceValue)
{
	if (IsFenceComplete(fenceValue))
	{
		return;
	}

	// TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
	// wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
	// the fence can only have one event set on completion, then thread B has to wait for 
	// 100 before it knows 99 is ready.  Maybe insert sequential events?
	{
		lock_guard<mutex> LockGuard(m_eventMutex);

		m_fence->SetEventOnCompletion(fenceValue, m_fenceEventHandle);
		WaitForSingleObject(m_fenceEventHandle, INFINITE);
		m_lastCompletedFenceValue = fenceValue;
	}
}


uint64_t CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
	lock_guard<mutex> lockGuard(m_fenceMutex);

	assert_succeeded(((ID3D12GraphicsCommandList*)commandList)->Close());

	// Kickoff the command list
	m_commandQueue->ExecuteCommandLists(1, &commandList);

	// Signal the next fence value (with the GPU)
	m_commandQueue->Signal(m_fence, m_nextFenceValue);

	// And increment the fence value.  
	return m_nextFenceValue++;
}


ID3D12CommandAllocator* CommandQueue::RequestAllocator()
{
	uint64_t completedFence = m_fence->GetCompletedValue();

	return m_allocatorPool.RequestAllocator(completedFence);
}


void CommandQueue::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator)
{
	m_allocatorPool.DiscardAllocator(fenceValue, allocator);
}


CommandListManager::CommandListManager()
	: m_graphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)
	, m_computeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)
	, m_copyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{}


CommandListManager::~CommandListManager()
{
	Destroy();
}


void CommandListManager::Destroy()
{
	m_graphicsQueue.Destroy();
	m_computeQueue.Destroy();
	m_copyQueue.Destroy();
}


void CommandListManager::Create()
{
	m_graphicsQueue.Create();
	m_computeQueue.Create();
	m_copyQueue.Create();
}


void CommandListManager::CreateNewCommandList(CommandListType type, ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator)
{
	assert_msg(type != CommandListType::Bundle, "Bundles are not yet supported");

	switch (type)
	{
	case CommandListType::Direct: *allocator = m_graphicsQueue.RequestAllocator(); break;
	case CommandListType::Bundle: break;
	case CommandListType::Compute: *allocator = m_computeQueue.RequestAllocator(); break;
	case CommandListType::Copy: *allocator = m_copyQueue.RequestAllocator(); break;
	}

	assert_succeeded(GetDevice()->CreateCommandList(1, static_cast<D3D12_COMMAND_LIST_TYPE>(type), *allocator, nullptr, IID_PPV_ARGS(commandList)));

	SetDebugName(*commandList, "CommandList");
}


void CommandListManager::WaitForFence(uint64_t fenceValue)
{
	CommandQueue& producer = g_commandManager.GetQueue((CommandListType)(fenceValue >> 56));
	producer.WaitForFence(fenceValue);
}