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
#include "Graphics\ResourceView.h"


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
	const DepthStencilView& GetDSV() const { return m_dsv[0]; }
	const DepthStencilView& GetDSV_DepthReadOnly() const { return m_dsv[1]; }
	const DepthStencilView& GetDSV_StencilReadOnly() const { return m_dsv[2]; }
	const DepthStencilView& GetDSV_ReadOnly() const { return m_dsv[3]; }
	const ShaderResourceView& GetDepthSRV() const { return m_depthSRV; }
	const ShaderResourceView& GetStencilSRV() const { return m_stencilSRV; }

	float GetClearDepth() const { return m_clearDepth; }
	uint8_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews();

private:
	float m_clearDepth;
	uint8_t m_clearStencil;
	DepthStencilView m_dsv[4];
	ShaderResourceView m_depthSRV;
	ShaderResourceView m_stencilSRV;
};

using DepthBufferPtr = std::shared_ptr<DepthBuffer>;

} // namespace Kodiak