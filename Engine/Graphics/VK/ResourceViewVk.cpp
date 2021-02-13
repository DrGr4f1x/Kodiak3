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

#include "Graphics\ResourceView.h"

#include "Graphics\GraphicsDevice.h"

#include "UtilVk.h"

using namespace Kodiak;


ShaderResourceView::ShaderResourceView() = default;


void ShaderResourceView::Create(UVkImage* uimage, ResourceType type, const TextureViewDesc& desc)
{
	ThrowIfFailed(g_graphicsDevice->CreateImageView(uimage, type, desc.format, false, 0, desc.mipCount, 0, desc.arraySize, &m_imageView));
	m_imageInfo = { VK_NULL_HANDLE, m_imageView->Get(), GetImageLayout(desc.usage) };
}


void ShaderResourceView::Create(UVkBuffer* ubuffer, ResourceType type, const BufferViewDesc& desc)
{
	m_bufferInfo = { ubuffer->Get(), 0, VK_WHOLE_SIZE };
}


UnorderedAccessView::UnorderedAccessView() = default;


void UnorderedAccessView::Create(UVkImage* uimage, ResourceType type, const TextureViewDesc& desc)
{
	ThrowIfFailed(g_graphicsDevice->CreateImageView(uimage, type, desc.format, true, 0, desc.mipCount, 0, desc.arraySize, &m_imageView));
	m_imageInfo = { VK_NULL_HANDLE, m_imageView->Get(), GetImageLayout(desc.usage) };
}


void UnorderedAccessView::Create(UVkBuffer* ubuffer, ResourceType type, const BufferViewDesc& desc)
{
	m_bufferInfo = { ubuffer->Get(), 0, VK_WHOLE_SIZE };
}


void UnorderedAccessView::Create(UVkBuffer* ubuffer, ResourceType type, const TypedBufferViewDesc& desc)
{
	m_bufferInfo = { ubuffer->Get(), 0, VK_WHOLE_SIZE };
}


ConstantBufferView::ConstantBufferView() = default;


void ConstantBufferView::Create(UVkBuffer* ubuffer, const BufferViewDesc& desc)
{
	m_bufferInfo = { ubuffer->Get(), 0, VK_WHOLE_SIZE };
}


DepthStencilView::DepthStencilView() = default;


void DepthStencilView::Create(UVkImage* uimage, const DepthStencilViewDesc& desc)
{
	ThrowIfFailed(g_graphicsDevice->CreateImageView(uimage, ResourceType::Texture2D, desc.format, false, 0, 1, 0, 1, &m_imageView));
}


RenderTargetView::RenderTargetView() = default;


void RenderTargetView::Create(UVkImage* uimage, const RenderTargetViewDesc& desc)
{
	ThrowIfFailed(g_graphicsDevice->CreateImageView(uimage, ResourceType::Texture2D, desc.format, false, 0, desc.numMips, 0, desc.arraySize, &m_imageView));
}