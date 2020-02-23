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

#include "ImageVk.h"

#include "AllocatorVk.h"
#include "LogicalDeviceVk.h"


using namespace Kodiak;
using namespace std;


Image::Image(const shared_ptr<LogicalDevice>& logicalDevice, const VkImage& image)
	: Reference(logicalDevice, nullptr)
	, m_image(image)
	, m_managed(false)
{}


Image::~Image()
{
	if (m_managed && Get<Allocator>())
	{
		vmaDestroyImage(*Get<Allocator>(), m_image, m_allocation);
		m_allocation = VK_NULL_HANDLE;
	}
	m_image = VK_NULL_HANDLE;
}