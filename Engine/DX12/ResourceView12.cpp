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

#include "Stdafx.h"

#include "ResourceView.h"

#include "Graphics\DescriptorHeap.h"
#include "GraphicsDevice.h"

#include "Util12.h"


using namespace Kodiak;


ShaderResourceView::ShaderResourceView()
{
	m_handle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


void ShaderResourceView::Create(const ResourceHandle& resource, ResourceType type, const TextureViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a texture SRV

	auto dxFormat = static_cast<DXGI_FORMAT>(desc.format);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = GetSRVDimension(type);
	srvDesc.Format = dxFormat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (desc.isDepth)
	{
		srvDesc.Format = GetDepthFormat(srvDesc.Format);
	}
	else if (desc.isStencil)
	{
		srvDesc.Format = GetStencilFormat(srvDesc.Format);
	}

	switch (type)
	{
	case ResourceType::Texture1D:
		srvDesc.Texture1D.MipLevels = desc.mipCount;
		break;
	case ResourceType::Texture1D_Array:
		srvDesc.Texture1DArray.MipLevels = desc.mipCount;
		srvDesc.Texture1DArray.ArraySize = desc.arraySize;
		break;
	case ResourceType::Texture2D:
	case ResourceType::Texture2DMS:
		srvDesc.Texture2D.MipLevels = desc.mipCount;
		if (desc.isStencil)
		{	
			srvDesc.Texture2D.PlaneSlice = (srvDesc.Format == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT) ? 1 : 0;
		}
		break;
	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS_Array:
		srvDesc.Texture2DArray.MipLevels = desc.mipCount;
		srvDesc.Texture2DArray.ArraySize = desc.arraySize;
		break;
	case ResourceType::TextureCube:
		srvDesc.TextureCube.MipLevels = desc.mipCount;
		break;
	case ResourceType::TextureCube_Array:
		srvDesc.TextureCubeArray.MipLevels = desc.mipCount;
		srvDesc.TextureCubeArray.NumCubes = desc.arraySize;
		break;
	case ResourceType::Texture3D:
		srvDesc.Texture3D.MipLevels = desc.mipCount;
		break;
	
	default:
		assert(false);
		return;
	}

	if (m_handle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(resource.Get(), &srvDesc, m_handle);
}


void ShaderResourceView::Create(const ResourceHandle& resource, ResourceType type, const BufferViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a buffer SRV

	auto dxFormat = static_cast<DXGI_FORMAT>(desc.format);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = GetSRVDimension(type);
	srvDesc.Format = dxFormat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (type)
	{
	case ResourceType::IndexBuffer:
	case ResourceType::ConstantBuffer:
	case ResourceType::ByteAddressBuffer:
	case ResourceType::IndirectArgsBuffer:
	case ResourceType::ReadbackBuffer:
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.NumElements = desc.bufferSize / sizeof(float);
		break;
	case ResourceType::VertexBuffer:
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.NumElements = desc.elementCount;
		srvDesc.Buffer.StructureByteStride = desc.elementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	case ResourceType::StructuredBuffer:
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.NumElements = desc.elementCount;
		srvDesc.Buffer.StructureByteStride = desc.elementSize;
		break;
	case ResourceType::TypedBuffer:
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.NumElements = desc.elementCount;

	default:
		assert(false);
		return;
	}

	if (m_handle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(resource.Get(), &srvDesc, m_handle);
}


UnorderedAccessView::UnorderedAccessView()
{
	m_handle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const TextureViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a texture UAV

	auto dxFormat = static_cast<DXGI_FORMAT>(desc.format);

	const bool isCube = type == ResourceType::TextureCube || type == ResourceType::TextureCube_Array;
	const uint32_t arrayMultiplier = isCube ? 6 : 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = GetUAVDimension(type);
	uavDesc.Format = GetUAVFormat(dxFormat);

	switch (type)
	{
	case ResourceType::Texture1D:
		uavDesc.Texture1D.MipSlice = desc.mipLevel;
		break;
	case ResourceType::Texture1D_Array:
		uavDesc.Texture1DArray.MipSlice = desc.mipLevel;
		uavDesc.Texture1DArray.ArraySize = desc.arraySize;
		uavDesc.Texture1DArray.FirstArraySlice = desc.firstArraySlice;
		break;
	case ResourceType::Texture2D:
		uavDesc.Texture2D.MipSlice = desc.mipLevel;
		break;
	case ResourceType::Texture2D_Array:
	case ResourceType::TextureCube:
	case ResourceType::TextureCube_Array:
		uavDesc.Texture2DArray.MipSlice = desc.mipLevel;
		uavDesc.Texture2DArray.ArraySize = desc.arraySize * arrayMultiplier;
		uavDesc.Texture2DArray.FirstArraySlice = desc.firstArraySlice * arrayMultiplier;
		break;
	case ResourceType::Texture3D:
		uavDesc.Texture3D.MipSlice = desc.mipLevel;
		break;
	
	default:
		assert(false);
		return;
	}

	if (m_handle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateUnorderedAccessView(resource.Get(), nullptr, &uavDesc, m_handle);
}


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const BufferViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a buffer UAV

	auto dxFormat = static_cast<DXGI_FORMAT>(desc.format);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = GetUAVDimension(type);
	uavDesc.Format = GetUAVFormat(dxFormat);

	switch (type)
	{
	case ResourceType::IndexBuffer:
	case ResourceType::ConstantBuffer:
	case ResourceType::ByteAddressBuffer:
	case ResourceType::IndirectArgsBuffer:
	case ResourceType::ReadbackBuffer:
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = desc.bufferSize / sizeof(float);
		break;
	case ResourceType::VertexBuffer:
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.NumElements = desc.elementCount;
		uavDesc.Buffer.StructureByteStride = desc.elementSize;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		break;
	case ResourceType::StructuredBuffer:
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.NumElements = desc.elementCount;
		uavDesc.Buffer.StructureByteStride = desc.elementSize;
		break;
	case ResourceType::TypedBuffer:
		uavDesc.Format = dxFormat;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.NumElements = desc.elementCount;

	default:
		assert(false);
		return;
	}

	if (m_handle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateUnorderedAccessView(resource.Get(), nullptr, &uavDesc, m_handle);
}


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const TypedBufferViewDesc& desc)
{
	BufferViewDesc bufferDesc{ desc.format, desc.bufferSize, desc.elementCount, desc.elementSize };
	Create(resource, type, bufferDesc);
}


IndexBufferView::IndexBufferView() = default;


void IndexBufferView::Create(const ResourceHandle& handle, const BufferViewDesc& desc)
{
	m_handle.BufferLocation = handle->GetGPUVirtualAddress();
	m_handle.Format = desc.elementSize == sizeof(uint32_t) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	m_handle.SizeInBytes = desc.bufferSize;
}


VertexBufferView::VertexBufferView() = default;


void VertexBufferView::Create(const ResourceHandle& handle, const BufferViewDesc& desc)
{
	m_handle.BufferLocation = handle->GetGPUVirtualAddress();
	m_handle.SizeInBytes = desc.bufferSize;
	m_handle.StrideInBytes = desc.elementSize;
}


ConstantBufferView::ConstantBufferView()
{
	m_handle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


void ConstantBufferView::Create(const ResourceHandle& resource, const BufferViewDesc& desc)
{
	UINT size = static_cast<UINT>(Math::AlignUp(desc.bufferSize, 16));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = size;

	m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GetDevice()->CreateConstantBufferView(&cbvDesc, m_handle);
}


DepthStencilView::DepthStencilView()
{
	m_handle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


void DepthStencilView::Create(const ResourceHandle& resource, const DepthStencilViewDesc& desc)
{
	auto dxFormat = static_cast<DXGI_FORMAT>(desc.format);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = GetDSVFormat(dxFormat);

	if (resource->GetDesc().SampleDesc.Count == 1)
	{
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
	}
	else
	{
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	}

	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	if (desc.readOnlyDepth && desc.readOnlyStencil)
	{
		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
	}
	else if (desc.readOnlyDepth)
	{
		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	}
	else if (desc.readOnlyStencil)
	{
		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
	}
	
	m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	GetDevice()->CreateDepthStencilView(resource.Get(), &dsvDesc, m_handle);
}


RenderTargetView::RenderTargetView()
{
	m_handle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


void RenderTargetView::Create(const ResourceHandle& resource, const RenderTargetViewDesc& desc)
{
	auto dxFormat = static_cast<DXGI_FORMAT>(desc.format);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = dxFormat;
	
	if (desc.arraySize > 1)
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = desc.arraySize;
	}
	else if (desc.fragmentCount > 1)
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
	}

	if (m_handle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create the render target view
	GetDevice()->CreateRenderTargetView(resource.Get(), desc.nullDesc ? nullptr : &rtvDesc, m_handle);
}