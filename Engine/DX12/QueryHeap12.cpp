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

#include "QueryHeap.h"

#include "GraphicsDevice.h"


using namespace Kodiak;


void OcclusionQueryHeap::Create(uint32_t queryCount)
{
	m_type = QueryHeapType::Occlusion;
	m_queryCount = queryCount;

	D3D12_QUERY_HEAP_DESC desc = {};
	desc.Count = m_queryCount;
	desc.NodeMask = 0;
	desc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;

	ThrowIfFailed(GetDevice()->CreateQueryHeap(&desc, IID_PPV_ARGS(&m_handle)));
}