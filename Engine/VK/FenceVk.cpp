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

#include "FenceVk.h"

#include "LogicalDeviceVk.h"


using namespace Kodiak;
using namespace std;


Fence::Fence(const shared_ptr<LogicalDevice>& logicalDevice, bool signalled)
	: Reference(logicalDevice)
{
	Initialize(signalled);
}


Fence::~Fence()
{
	vkDestroyFence(*Get<LogicalDevice>(), m_fence, nullptr);
	m_fence = VK_NULL_HANDLE;
}


bool Fence::IsSignalled() const
{
	return vkGetFenceStatus(*Get<LogicalDevice>(), m_fence) == VK_SUCCESS;
}


void Fence::Reset()
{
	ThrowIfFailed(vkResetFences(*Get<LogicalDevice>(), 1, &m_fence));
}


void Fence::Wait(uint64_t timeout) const
{
	ThrowIfFailed(vkWaitForFences(*Get<LogicalDevice>(), 1, &m_fence, true, timeout));
}


void Fence::Initialize(bool signalled)
{
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = signalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	ThrowIfFailed(vkCreateFence(*Get<LogicalDevice>(), &createInfo, nullptr, &m_fence));
}


namespace Kodiak
{
void WaitForFences(const vector<shared_ptr<Fence>>& fences, bool all, uint64_t timeout)
{
	if (!fences.empty())
	{
		VkDevice device = *fences.front()->Get<LogicalDevice>();

		vector<VkFence> fencesArray;
		fencesArray.reserve(fences.size());
		for (const auto& fence : fences)
		{
			assert(device == *fence->Get<LogicalDevice>());
			fencesArray.push_back(*fence);
		}

		ThrowIfFailed(vkWaitForFences(device, (uint32_t)fencesArray.size(), fencesArray.data(), all, timeout));
	}
}


void ResetFences(const vector<shared_ptr<Fence>>& fences)
{
	if (!fences.empty())
	{
		VkDevice device = *fences.front()->Get<LogicalDevice>();

		vector<VkFence> fencesArray;
		fencesArray.reserve(fences.size());
		for (const auto& fence : fences)
		{
			assert(device == *fence->Get<LogicalDevice>());
			fencesArray.push_back(*fence);
		}

		ThrowIfFailed(vkResetFences(device, (uint32_t)fencesArray.size(), fencesArray.data()));
	}
}
} // namespace Kodiak