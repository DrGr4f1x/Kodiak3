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

// Forward declarations
class Allocator;
class LogicalDevice;


class Image : public Reference<LogicalDevice, Allocator>, public std::enable_shared_from_this<Image>, public NonCopyable
{
public:
	Image(const std::shared_ptr<LogicalDevice>& logicalDevice, const VkImage& image);
	virtual ~Image();

	operator VkImage();

private:
	VkImage			m_image{ VK_NULL_HANDLE };
	VmaAllocation	m_allocation{ VK_NULL_HANDLE };
	bool			m_managed{ false };
};


inline Image::operator VkImage()
{
	return m_image;
}

} // namespace Kodiak