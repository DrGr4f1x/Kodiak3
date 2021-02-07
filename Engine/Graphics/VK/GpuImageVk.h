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

namespace Kodiak
{

class GpuImage
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	GpuImage();
	GpuImage(UVkImage* uimage, ResourceState initialState);
	virtual ~GpuImage() = 0;

	const ResourceType GetType() const { return m_type; }

	UVkImage* GetImageHandle()  { return m_image.Get(); }
	const UVkImage* GetImageHandle() const { return m_image.Get(); }

protected:
	Microsoft::WRL::ComPtr<UVkImage> m_image;
	ResourceState	m_usageState;
	ResourceState	m_transitioningState;
	ResourceType	m_type;
};

} // namespace Kodiak