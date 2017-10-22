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

namespace Kodiak
{

class GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	GpuResource();
	GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState);

	virtual void Destroy();

	ID3D12Resource* operator->() { return m_resource.Get(); }
	const ID3D12Resource* operator->() const { return m_resource.Get(); }

	ID3D12Resource* GetResource() { return m_resource.Get(); }
	const ID3D12Resource* GetResource() const { return m_resource.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_gpuVirtualAddress; }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_RESOURCE_STATES m_usageState;
	D3D12_RESOURCE_STATES m_transitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress;
};

} // namespace Kodiak