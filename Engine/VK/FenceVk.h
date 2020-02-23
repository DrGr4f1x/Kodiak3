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


class Fence : public Reference<LogicalDevice>, public NonCopyable
{
public:
	Fence(const std::shared_ptr<LogicalDevice>& logicalDevice, bool signalled);
	virtual ~Fence();

	bool IsSignalled() const;
	void Reset();
	void Wait(uint64_t timeout) const;

	operator VkFence();

protected:
	void Initialize(bool signalled);

private:
	VkFence m_fence{ VK_NULL_HANDLE };
};


void WaitForFences(const std::vector<std::shared_ptr<Fence>>& fences, bool all, uint64_t timeout);
void ResetFences(const std::vector<std::shared_ptr<Fence>>& fences);


inline Fence::operator VkFence()
{
	return m_fence;
}

} // namespace Kodiak