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

#pragma once

namespace Kodiak
{

class CommandAllocatorPool
{
public:
	CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);
	~CommandAllocatorPool();

	void Create(ID3D12Device* device);
	void Shutdown();

	ID3D12CommandAllocator* RequestAllocator(uint64_t completedFenceValue);
	void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator);

	inline size_t Size() { return m_allocatorPool.size(); }

private:
	const D3D12_COMMAND_LIST_TYPE m_commandListType;

	ID3D12Device* m_device{ nullptr };
	std::vector<ID3D12CommandAllocator*> m_allocatorPool;
	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_readyAllocators;
	std::mutex m_allocatorMutex;
};

} // namespace Kodiak