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

#include "ColorBuffer12.h"

namespace Kodiak
{

class SwapChain
{
public:
	void Create(IDXGIFactory4* dxgiFactory, HWND hWnd, uint32_t width, uint32_t height, DXGI_FORMAT format);
	void Destroy();

	void Present(UINT presentInterval);

	static uint32_t GetBufferCount();
	DXGI_FORMAT GetFormat() const { return m_format; }

	ColorBuffer& GetColorBuffer(uint32_t index) { return m_displayPlanes[index]; }
	const ColorBuffer& GetColorBuffer(uint32_t index) const { return m_displayPlanes[index]; }

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
	DXGI_FORMAT m_format{ DXGI_FORMAT_UNKNOWN };

	std::vector<ColorBuffer> m_displayPlanes;
};

} // namespace Kodiak