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

#include "Graphics\PixelBuffer.h"


namespace Kodiak
{

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint8_t clearStencil = 0);
	~DepthBuffer();

	void Create(const std::string& name, uint32_t width, uint32_t height, Format format);
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numSamples, Format format);

	// Get pre-created CPU - visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return m_dsvHandle[0]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_DepthReadOnly() const { return m_dsvHandle[1]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_StencilReadOnly() const { return m_dsvHandle[2]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_ReadOnly() const { return m_dsvHandle[3]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const { return m_depthSRVHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const { return m_stencilSRVHandle; }

	float GetClearDepth() const { return m_clearDepth; }
	uint8_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews();

private:
	float m_clearDepth;
	uint8_t m_clearStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle[4];
	D3D12_CPU_DESCRIPTOR_HANDLE m_depthSRVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_stencilSRVHandle;
};

using DepthBufferPtr = std::shared_ptr<DepthBuffer>;

} // namespace Kodiak