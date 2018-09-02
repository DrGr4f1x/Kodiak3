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

#include "ResourceView.h"


using namespace Kodiak;


ShaderResourceView::ShaderResourceView() = default;


void ShaderResourceView::Create(const ResourceHandle& resource, ResourceType type, const ResourceViewDesc& desc)
{
	m_handle.buffer = resource;
	m_handle.offset = 0;
	m_handle.range = desc.bufferSize;
}


UnorderedAccessView::UnorderedAccessView() = default;


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const ResourceViewDesc& desc)
{
	m_handle.buffer = resource;
	m_handle.offset = 0;
	m_handle.range = desc.bufferSize;
}


IndexBufferView::IndexBufferView() = default;


void IndexBufferView::Create(const ResourceHandle& handle, const ResourceViewDesc& desc)
{
	// Nothing to do for Vulkan
}


VertexBufferView::VertexBufferView() = default;


void VertexBufferView::Create(const ResourceHandle& handle, const ResourceViewDesc& desc)
{
	// Nothing to do for Vulkan
}


ConstantBufferView::ConstantBufferView() = default;


void ConstantBufferView::Create(const ResourceHandle& resource, const ResourceViewDesc& desc)
{
	m_handle.buffer = resource;
	m_handle.offset = 0;
	m_handle.range = desc.bufferSize;
}