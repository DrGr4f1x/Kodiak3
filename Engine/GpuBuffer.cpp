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

#include "GpuBuffer.h"

#include "CommandContext.h"
#include "GraphicsDevice.h"


using namespace Kodiak;


static inline bool HasFlag(ResourceType type, ResourceType flag)
{
	return (type & flag) != 0;
}


GpuBuffer::~GpuBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource);
}


BufferViewDesc GpuBuffer::GetDesc() const
{
	BufferViewDesc resDesc = {};
	resDesc.format = Format::Unknown;
	resDesc.bufferSize = (uint32_t)m_bufferSize;
	resDesc.elementCount = (uint32_t)m_elementCount;
	resDesc.elementSize = (uint32_t)m_elementSize;
	return resDesc;
}


void IndexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();
	
	m_srv.Create(m_resource, m_type, resDesc);
	m_uav.Create(m_resource, m_type, resDesc);
	m_ibv.Create(m_resource, resDesc);

	m_indexSize16 = (m_elementSize == 2);
}


void VertexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_resource, m_type, resDesc);
	m_uav.Create(m_resource, m_type, resDesc);
	m_vbv.Create(m_resource, resDesc);
}


void ConstantBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_cbv.Create(m_resource, resDesc);
}


void ByteAddressBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_resource, m_type, resDesc);
	m_uav.Create(m_resource, m_type, resDesc);
}


void StructuredBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_resource, ResourceType::StructuredBuffer, resDesc);
	m_uav.Create(m_resource, ResourceType::StructuredBuffer, resDesc);
	if (HasFlag(m_type, ResourceType::VertexBuffer))
	{
		m_vbv.Create(m_resource, resDesc);
	}

	m_counterBuffer.Create("Counter Buffer", 1, 4);
}


const ShaderResourceView& StructuredBuffer::GetCounterSRV(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::GenericRead);
	return m_counterBuffer.GetSRV();
}


const UnorderedAccessView& StructuredBuffer::GetCounterUAV(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::UnorderedAccess);
	return m_counterBuffer.GetUAV();
}


void TypedBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();
	resDesc.format = m_dataFormat;

	m_srv.Create(m_resource, m_type, resDesc);
	m_uav.Create(m_resource, m_type, resDesc);
}