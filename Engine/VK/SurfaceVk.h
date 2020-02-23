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

class Instance;

class Surface : public Reference<Instance>, public NonCopyable
{
public:
	Surface(const std::shared_ptr<Instance>& instance, const VkWin32SurfaceCreateInfoKHR& createInfo);
	virtual ~Surface();

	operator VkSurfaceKHR();

protected:
	void Initialize(const VkWin32SurfaceCreateInfoKHR& createInfo);

private:
	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
};

inline Surface::operator VkSurfaceKHR()
{
	return m_surface;
}

} // namespace Kodiak