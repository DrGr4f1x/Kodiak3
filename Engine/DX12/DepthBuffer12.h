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

#include "PixelBuffer12.h"

namespace Kodiak
{

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint8_t clearStencil = 0)
		: m_clearDepth(clearDepth)
		, m_clearStencil(clearStencil)
	{
		m_dsv[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_dsv[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_dsv[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_dsv[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_depthSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_stencilSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	void Create(const std::string& name, uint32_t width, uint32_t height, Format format);
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numSamples, Format format);

	// Get pre-created CPU-visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return m_dsv[0]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_DepthReadOnly() const { return m_dsv[1]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_StencilReadOnly() const { return m_dsv[2]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_ReadOnly() const { return m_dsv[3]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const { return m_depthSRV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const { return m_stencilSRV; }

	float GetClearDepth() const { return m_clearDepth; }
	uint8_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews(Format format);

private:
	float m_clearDepth;
	uint8_t m_clearStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsv[4];
	D3D12_CPU_DESCRIPTOR_HANDLE m_depthSRV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_stencilSRV;
};

} // namespace Kodiak