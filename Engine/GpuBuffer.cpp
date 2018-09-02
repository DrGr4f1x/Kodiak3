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


using namespace Kodiak;


GpuBuffer::~GpuBuffer()
{}


void IndexBuffer::CreateDerivedViews()
{
	auto resDesc = DescribeIndexBuffer(m_elementSize, m_bufferSize);

	m_srv.Create(m_resource, ResourceType::IndexBuffer, resDesc);
	m_uav.Create(m_resource, ResourceType::IndexBuffer, resDesc);
	m_ibv.Create(m_resource, resDesc);

	m_indexSize16 = (m_elementSize == 2);
}


void VertexBuffer::CreateDerivedViews()
{
	auto resDesc = DescribeVertexBuffer(m_elementSize, m_elementCount, m_bufferSize);

	m_srv.Create(m_resource, ResourceType::VertexBuffer, resDesc);
	m_uav.Create(m_resource, ResourceType::VertexBuffer, resDesc);
	m_vbv.Create(m_resource, resDesc);
}


void ConstantBuffer::CreateDerivedViews()
{
	auto resDesc = DescribeConstantBuffer(m_bufferSize);

	m_cbv.Create(m_resource, resDesc);
}