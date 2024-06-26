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

#include "GpuResourceVk.h"


using namespace Kodiak;


GpuResource::GpuResource()
	: m_usageState(ResourceState::Undefined)
	, m_transitioningState(ResourceState::Undefined)
	, m_type(ResourceType::Unknown)
{}


GpuResource::GpuResource(ResourceState initialState)
	: m_usageState(initialState)
	, m_transitioningState(ResourceState::Undefined)
	, m_type(ResourceType::Unknown)
{}


GpuResource::~GpuResource() = default;