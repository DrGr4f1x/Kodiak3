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

#include "Graphics\QueryHeap.h"

#include "Graphics\GraphicsDevice.h"


using namespace Kodiak;


void OcclusionQueryHeap::Create(uint32_t queryCount)
{
	m_type = QueryHeapType::Occlusion;
	m_queryCount = queryCount;

	ThrowIfFailed(g_graphicsDevice->CreateQueryHeap(m_type, m_queryCount, &m_heap));
}