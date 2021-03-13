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

#include "Graphics\ResourceSet.h"

#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\RootSignature.h"
#include "Graphics\Texture.h"

#include "DescriptorHeap12.h"


using namespace std;
using namespace Kodiak;


void ResourceSet::Init(const RootSignature* rootSig)
{
	for (uint32_t i = 0; i < rootSig->GetNumParameters(); ++i)
	{
		m_descriptorSets[i].Init(*rootSig, i);
	}
}


void ResourceSet::Finalize()
{
	for (uint32_t i = 0; i < 8; ++i)
	{
		if (m_descriptorSets[i].IsInitialized())
		{
			m_descriptorSets[i].Update();
		}
	}
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const ColorBuffer& buffer)
{
	m_descriptorSets[rootIndex].SetSRV(paramIndex, buffer);
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const DepthBuffer& buffer, bool depthSrv)
{
	m_descriptorSets[rootIndex].SetSRV(paramIndex, buffer, depthSrv);
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const StructuredBuffer& buffer)
{
	m_descriptorSets[rootIndex].SetSRV(paramIndex, buffer);
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const Texture& texture)
{
	m_descriptorSets[rootIndex].SetSRV(paramIndex, texture);
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const ColorBuffer& buffer)
{
	m_descriptorSets[rootIndex].SetUAV(paramIndex, buffer);
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const DepthBuffer& buffer)
{
	m_descriptorSets[rootIndex].SetUAV(paramIndex, buffer);
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const StructuredBuffer& buffer)
{
	m_descriptorSets[rootIndex].SetUAV(paramIndex, buffer);
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const Texture& texture)
{
	m_descriptorSets[rootIndex].SetUAV(paramIndex, texture);
}


void ResourceSet::SetCBV(int rootIndex, int paramIndex, const ConstantBuffer& buffer)
{
	m_descriptorSets[rootIndex].SetCBV(paramIndex, buffer);
}


void ResourceSet::SetDynamicOffset(int rootIndex, uint32_t offset)
{
	m_descriptorSets[rootIndex].SetDynamicOffset(offset);
}