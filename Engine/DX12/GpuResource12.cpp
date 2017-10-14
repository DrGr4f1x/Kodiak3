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

#include "GpuResource12.h"


using namespace Kodiak;


GpuResource::GpuResource()
	: m_resource(nullptr)
	, m_usageState((D3D12_RESOURCE_STATES)-1)
	, m_transitioningState((D3D12_RESOURCE_STATES)-1)
	, m_gpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
{}


GpuResource::GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState)
	: m_resource(resource)
	, m_usageState(initialState)
	, m_transitioningState((D3D12_RESOURCE_STATES)-1)
	, m_gpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
{}


void GpuResource::Destroy()
{
	m_resource = nullptr;
	m_gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
}