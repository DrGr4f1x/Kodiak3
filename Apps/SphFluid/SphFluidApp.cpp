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

#include "SphFluidApp.h"

#include "Graphics\CommandContext.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void SphFluidApp::Configure()
{
	Application::Configure();
}


void SphFluidApp::Startup()
{}


void SphFluidApp::Shutdown()
{}


bool SphFluidApp::Update()
{
	return true;
}


void SphFluidApp::Render()
{
	auto& context = GraphicsContext::Begin("Frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}