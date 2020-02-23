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
class LogicalDevice;


enum class SemaphoreType
{
	Binary,
	Timeline
};


class Semaphore : public Reference<LogicalDevice>, public NonCopyable
{
public:
	Semaphore(const std::shared_ptr<LogicalDevice>& device, SemaphoreType type);
	virtual ~Semaphore();

	operator VkSemaphore();

protected:
	void Initialize(SemaphoreType type);

private:
	VkSemaphore m_semaphore{ VK_NULL_HANDLE };
};


inline Semaphore::operator VkSemaphore()
{
	return m_semaphore;
}

} // namespace Kodiak