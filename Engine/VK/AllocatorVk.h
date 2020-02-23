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
class PhysicalDevice;


class Allocator : public Reference<Instance, PhysicalDevice, LogicalDevice>, public std::enable_shared_from_this<Allocator>, public NonCopyable
{
public:
	virtual ~Allocator();

	static std::shared_ptr<Allocator> Create(
		const std::shared_ptr<Instance>& instance,
		const std::shared_ptr<PhysicalDevice>& physicalDevice,
		const std::shared_ptr<LogicalDevice>& logicalDevice);

	operator VmaAllocator();

protected:
	Allocator(
		const std::shared_ptr<Instance>& instance,
		const std::shared_ptr<PhysicalDevice>& physicalDevice,
		const std::shared_ptr<LogicalDevice>& logicalDevice);

	void Initialize();

private:
	VmaAllocator m_allocator{ VK_NULL_HANDLE };
};


inline Allocator::operator VmaAllocator()
{
	return m_allocator;
}

} // namespace Kodiak