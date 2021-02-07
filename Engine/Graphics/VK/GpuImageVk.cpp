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

#include "GpuImageVk.h"


using namespace Kodiak;
using namespace std;


GpuImage::GpuImage()
	: m_image(nullptr)
	, m_usageState(ResourceState::Undefined)
	, m_transitioningState(ResourceState::Undefined)
	, m_type(ResourceType::Unknown)
{}


GpuImage::GpuImage(UVkImage* uimage, ResourceState initialState)
	: m_image(uimage)
	, m_usageState(initialState)
	, m_transitioningState(ResourceState::Undefined)
	, m_type(ResourceType::Unknown)
{}


GpuImage::~GpuImage() = default;