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
	GpuResource(const ResourceHandle& resource, ResourceState initialState);
	virtual ~GpuResource() = 0;

	const ResourceType GetType() const { return m_type; }
	const ResourceHandle& GetHandle() const { return m_resource; }

protected:
	ResourceHandle	m_resource;
	ResourceState	m_usageState;
	ResourceState	m_transitioningState;
	ResourceType	m_type;
};

} // namespace Kodiak