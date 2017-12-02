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
#include "Texture.h"

class TextureApp : public Kodiak::Application
{
public:
	TextureApp() : Kodiak::Application("Texture") {}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	void Render() final;

private:
	void LoadAssets();

private:
	
	// Assets
	Kodiak::TexturePtr m_texture;
};