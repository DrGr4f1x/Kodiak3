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

#include "SurfaceVk.h"

#include "InstanceVk.h"


using namespace Kodiak;
using namespace std;


Surface::Surface(const shared_ptr<Instance>& instance, const VkWin32SurfaceCreateInfoKHR& createInfo)
	: Reference(instance)
{
	Initialize(createInfo);
}


Surface::~Surface()
{
	vkDestroySurfaceKHR(*Get<Instance>(), m_surface, nullptr);
	m_surface = VK_NULL_HANDLE;
}


void Surface::Initialize(const VkWin32SurfaceCreateInfoKHR& createInfo)
{
	VkResult res = vkCreateWin32SurfaceKHR(*Get<Instance>(), &createInfo, nullptr, &m_surface);

	if (res != VK_SUCCESS)
	{
		Utility::ExitFatal("Could not create surface!", "Fatal error");
	}
}