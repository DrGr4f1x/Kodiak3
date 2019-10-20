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

#include "Application.h"
#include "ColorBuffer.h"
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"

class ComputeClothApp : public Kodiak::Application
{
public:
	ComputeClothApp() : Kodiak::Application("Compute Cloth") {}

};