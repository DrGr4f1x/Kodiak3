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
class LogicalDevice;
class Surface;


class PhysicalDevice : public Reference<Instance>, public std::enable_shared_from_this<PhysicalDevice>
{
public:
	PhysicalDevice(const std::shared_ptr<Instance>& instance, VkPhysicalDevice physicalDevice);
	virtual ~PhysicalDevice();

	std::shared_ptr<LogicalDevice> CreateLogicalDevice(
		const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
		const std::vector<std::string>& enabledLayerNames,
		const std::vector<std::string>& enabledExtensionNames,
		const VkPhysicalDeviceFeatures2& enabledFeatures);

	const VkPhysicalDeviceProperties& GetDeviceProperties() const { return m_deviceProperties; }
	const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_memoryProperties; }

	const std::vector<VkExtensionProperties> GetExtensionProperties() const { return m_extensions; }

	const VkPhysicalDeviceFeatures2& GetDeviceFeatures2() const { return m_deviceFeatures2; }
	const VkPhysicalDeviceVulkan12Features& GetDeviceFeatures1_2() const { return m_deviceFeatures1_2; }

	bool GetSurfaceSupport(uint32_t index, const std::shared_ptr<Surface>& surface) const;
	const std::vector<VkQueueFamilyProperties>& GetQueueFamilyProperties() const { return m_queueFamilyProperties; }

	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound);

	bool IsExtensionSupported(const std::string& name) const;

	operator VkPhysicalDevice();

protected:
	void Initialize();

private:
	VkPhysicalDevice						m_physicalDevice{ VK_NULL_HANDLE };

	VkPhysicalDeviceProperties				m_deviceProperties;
	VkPhysicalDeviceMemoryProperties		m_memoryProperties;

	std::vector<VkExtensionProperties>		m_extensions;

	// Device features
	VkPhysicalDeviceFeatures2				m_deviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr };
	VkPhysicalDeviceVulkan12Features		m_deviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };

	// Queue family properties
	std::vector<VkQueueFamilyProperties>	m_queueFamilyProperties;
};


inline PhysicalDevice::operator VkPhysicalDevice()
{
	return m_physicalDevice;
}


std::vector<uint32_t> GetGraphicsPresentQueueFamilyIndices(const std::shared_ptr<PhysicalDevice>& physicalDevice, const std::shared_ptr<Surface>& surface);
std::vector<uint32_t> GetQueueFamilyIndices(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkQueueFlags queueFlags);
uint32_t GetQueueFamilyIndex(const std::shared_ptr<PhysicalDevice>& physicalDevice, VkQueueFlags queueFlags);

} // namespace Kodiak