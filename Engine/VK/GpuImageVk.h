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

protected:
	std::shared_ptr<ImageRef>	m_image;
};

} // namespace Kodiak