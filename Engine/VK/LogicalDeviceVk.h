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
class Instance;
class PhysicalDevice;


class LogicalDevice : public Reference<PhysicalDevice>, public std::enable_shared_from_this<LogicalDevice>, public NonCopyable
{
public:
	virtual ~LogicalDevice();

	static std::shared_ptr<LogicalDevice> Create(
		const std::shared_ptr<PhysicalDevice>& physicalDevice,
		const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
		const std::vector<std::string>& enabledLayerNames,
		const std::vector<std::string>& enabledExtensionNames,
		const VkPhysicalDeviceFeatures2& enabledFeatures);

	operator VkDevice();

protected:
	LogicalDevice(const std::shared_ptr<PhysicalDevice>& physicalDevice);
	void Initialize(
		const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
		const std::vector<std::string>& enabledLayerNames,
		const std::vector<std::string>& enabledExtensionNames,
		const VkPhysicalDeviceFeatures2& enabledFeatures);

private:
	VkDevice m_device{ VK_NULL_HANDLE };
};


inline LogicalDevice::operator VkDevice()
{
	return m_device;
}

} // namespace Kodiak