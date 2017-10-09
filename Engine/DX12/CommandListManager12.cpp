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

using namespace Kodiak;
using namespace std;


CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type)
	: m_type(type)
	, m_commandQueue(nullptr)
	, m_fence(nullptr)
	, m_nextFenceValue((uint64_t)type << 56 | 1)
	, m_lastCompletedFenceValue((uint64_t)type << 56)
	, m_allocatorPool(type)
{}


CommandQueue::~CommandQueue()
{
	Shutdown();
}


void CommandQueue::Shutdown()
{
	if (m_commandQueue == nullptr)
	{
		return;
	}

	m_allocatorPool.Shutdown();

	CloseHandle(m_fenceEventHandle);

	m_fence->Release();
	m_fence = nullptr;

	m_commandQueue->Release();
	m_commandQueue = nullptr;
}


CommandListManager::CommandListManager()
	: m_device(nullptr)
	, m_graphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)
	, m_computeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)
	, m_copyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{}


CommandListManager::~CommandListManager()
{
	Shutdown();
}


void CommandListManager::Shutdown()
{
	m_graphicsQueue.Shutdown();
	m_computeQueue.Shutdown();
	m_copyQueue.Shutdown();
}


void CommandQueue::Create(ID3D12Device* device)
{
	assert(device != nullptr);
	assert(!IsReady());
	assert(m_allocatorPool.Size() == 0);

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = m_type;
	queueDesc.NodeMask = 1;
	device->CreateCommandQueue(&queueDesc, MY_IID_PPV_ARGS(&m_commandQueue));
	m_commandQueue->SetName(L"CommandListManager::m_CommandQueue");

	assert_succeeded(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_fence)));
	m_fence->SetName(L"CommandListManager::m_pFence");
	m_fence->Signal((uint64_t)m_type << 56);

	m_fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
	assert(m_fenceEventHandle != INVALID_HANDLE_VALUE);

	m_allocatorPool.Create(device);

	assert(IsReady());
}


void CommandListManager::Create(ID3D12Device* device)
{
	assert(device != nullptr);

	m_device = device;

	m_graphicsQueue.Create(device);
	m_computeQueue.Create(device);
	m_copyQueue.Create(device);
}


void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** commandList, ID3D12CommandAllocator** allocator)
{
	assert_msg(type != D3D12_COMMAND_LIST_TYPE_BUNDLE, "Bundles are not yet supported");

	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT: *allocator = m_graphicsQueue.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: *allocator = m_computeQueue.RequestAllocator(); break;
	case D3D12_COMMAND_LIST_TYPE_COPY: *allocator = m_copyQueue.RequestAllocator(); break;
	}

	assert_succeeded(m_device->CreateCommandList(1, type, *allocator, nullptr, MY_IID_PPV_ARGS(commandList)));
	(*commandList)->SetName(L"CommandList");
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


namespace Kodiak
{
extern CommandListManager g_commandManager;
} // namespace Kodiak


void CommandQueue::StallForFence(uint64_t fenceValue)
{
	CommandQueue& producer = g_commandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(fenceValue >> 56));
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


void CommandListManager::WaitForFence(uint64_t fenceValue)
{
	CommandQueue& producer = g_commandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(fenceValue >> 56));
	producer.WaitForFence(fenceValue);
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