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

class PhysicalDevice;

class Instance : public std::enable_shared_from_this<Instance>, public NonCopyable
{
public:
	Instance(const VkInstanceCreateInfo& createInfo);
	virtual ~Instance();

	static std::shared_ptr<Instance> Create(const std::string& appName);

	size_t GetPhysicalDeviceCount() const { return m_physicalDevices.size(); }
	std::shared_ptr<PhysicalDevice> GetPhysicalDevice(size_t index);

	operator VkInstance();

protected:
	void Initialize(const VkInstanceCreateInfo& createInfo);
	void EnumeratePhysicalDevices();

private:
	VkInstance									m_instance{ VK_NULL_HANDLE };
	std::vector<VkPhysicalDevice>				m_physicalDevices;
	std::vector<std::weak_ptr<PhysicalDevice>>	m_cachedPhysicalDevices;
};

inline Instance::operator VkInstance()
{
	return m_instance;
}

} // namespace Kodiak