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

#include "SemaphoreVk.h"

#include "LogicalDeviceVk.h"


using namespace Kodiak;
using namespace std;


Semaphore::Semaphore(const shared_ptr<LogicalDevice>& device, SemaphoreType type)
	: Reference(device)
{
	Initialize(type);
}


Semaphore::~Semaphore()
{
	vkDestroySemaphore(*Get<LogicalDevice>(), m_semaphore, nullptr);
	m_semaphore = VK_NULL_HANDLE;
}


void Semaphore::Initialize(SemaphoreType type)
{
	VkSemaphoreTypeCreateInfo binaryCreateInfo;
	binaryCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	binaryCreateInfo.pNext = nullptr;
	binaryCreateInfo.semaphoreType = type == SemaphoreType::Binary ? VK_SEMAPHORE_TYPE_BINARY : VK_SEMAPHORE_TYPE_TIMELINE;
	binaryCreateInfo.initialValue = 0;

	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = &binaryCreateInfo;
	createInfo.flags = 0;

	ThrowIfFailed(vkCreateSemaphore(*Get<LogicalDevice>(), &createInfo, nullptr, &m_semaphore));
}