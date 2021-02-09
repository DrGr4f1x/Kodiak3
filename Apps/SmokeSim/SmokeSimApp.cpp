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

#include "SmokeSimApp.h"

#include "Graphics\CommandContext.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void SmokeSimApp::Configure()
{
	Application::Configure();
}


void SmokeSimApp::Startup()
{}


void SmokeSimApp::Shutdown()
{}


bool SmokeSimApp::Update()
{
	return true;
}


void SmokeSimApp::Render()
{
	auto& context = GraphicsContext::Begin("Frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}