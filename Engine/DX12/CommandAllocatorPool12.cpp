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

#include "CommandAllocatorPool12.h"


using namespace Kodiak;
using namespace std;


CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type)
	: m_commandListType(type)
{}


CommandAllocatorPool::~CommandAllocatorPool()
{
	Shutdown();
}


void CommandAllocatorPool::Create(ID3D12Device* device)
{
	m_device = device;
}


void CommandAllocatorPool::Shutdown()
{
	for (size_t i = 0; i < m_allocatorPool.size(); ++i)
	{
		m_allocatorPool[i]->Release();
	}

	m_allocatorPool.clear();
}


ID3D12CommandAllocator * CommandAllocatorPool::RequestAllocator(uint64_t completedFenceValue)
{
	lock_guard<mutex> lockGuard(m_allocatorMutex);

	ID3D12CommandAllocator* allocator = nullptr;

	if (!m_readyAllocators.empty())
	{
		pair<uint64_t, ID3D12CommandAllocator*>& allocatorPair = m_readyAllocators.front();

		if (allocatorPair.first <= completedFenceValue)
		{
			allocator = allocatorPair.second;
			assert_succeeded(allocator->Reset());
			m_readyAllocators.pop();
		}
	}

	// If no allocator's were ready to be reused, create a new one
	if (allocator == nullptr)
	{
		assert_succeeded(m_device->CreateCommandAllocator(m_commandListType, MY_IID_PPV_ARGS(&allocator)));
		wchar_t allocatorName[32];
		swprintf(allocatorName, 32, L"CommandAllocator %zu", m_allocatorPool.size());
		allocator->SetName(allocatorName);
		m_allocatorPool.push_back(allocator);
	}

	return allocator;
}


void CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator)
{
	lock_guard<mutex> lockGuard(m_allocatorMutex);

	// That fence value indicates we are free to reset the allocator
	m_readyAllocators.push(make_pair(fenceValue, allocator));
}