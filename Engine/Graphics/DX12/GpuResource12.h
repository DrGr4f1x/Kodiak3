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
	GpuResource(ID3D12Resource* resource, ResourceState initialState);
	virtual ~GpuResource() = 0;

	const ResourceType GetType() const { return m_type; }
	ID3D12Resource* GetResource() { return m_resource.Get(); }
	const ID3D12Resource* GetResource() const { return m_resource.Get(); }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	ResourceState	m_usageState;
	ResourceState	m_transitioningState;
	ResourceType	m_type;
};

} // namespace Kodiak