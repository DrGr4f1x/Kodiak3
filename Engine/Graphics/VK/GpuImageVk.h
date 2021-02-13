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

#include "GpuResourceVk.h"


namespace Kodiak
{

class GpuImage : public GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	GpuImage() : m_image(nullptr) {}
	explicit GpuImage(UVkImage* uimage) : m_image(uimage) {}

	UVkImage* GetImageHandle()  { return m_image.Get(); }
	const UVkImage* GetImageHandle() const { return m_image.Get(); }

protected:
	Microsoft::WRL::ComPtr<UVkImage> m_image;
};

} // namespace Kodiak