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

#include "TextureApp.h"

#include "CommandContext.h"
#include "Filesystem.h"


using namespace Kodiak;
using namespace std;


void TextureApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Textures");
}


void TextureApp::Startup()
{

	LoadAssets();
}


void TextureApp::Shutdown()
{
	m_texture.reset();
}


void TextureApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	Color clearColor{ DirectX::Colors::Black };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.Finish();
}


void TextureApp::LoadAssets()
{
	m_texture = Texture::Load("metalplate01_rgba.ktx");
}