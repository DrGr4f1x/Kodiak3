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

#include "FluidEngine.h"

#include "Graphics\CommandContext.h"


using namespace Kodiak;
using namespace std;


FluidEngine::FluidEngine()
{
	SetFormat(RenderTarget::Velocity0, Format::R16G16B16A16_Float);
	SetFormat(RenderTarget::Velocity1, Format::R16G16B16A16_Float);
	SetFormat(RenderTarget::Pressure, Format::R16_Float);
	SetFormat(RenderTarget::Color0, Format::R16_Float);
	SetFormat(RenderTarget::Color1, Format::R16_Float);
	SetFormat(RenderTarget::Obstacles, Format::R8_UNorm);
	SetFormat(RenderTarget::ObstacleVelocity, Format::R16G16B16A16_Float);
	SetFormat(RenderTarget::TempScalar, Format::R16_Float);
	SetFormat(RenderTarget::TempVector, Format::R16G16B16A16_Float);

	SetName(RenderTarget::Velocity0, "Velocity 0");
	SetName(RenderTarget::Velocity1, "Velocity 1");
	SetName(RenderTarget::Pressure, "Pressure");
	SetName(RenderTarget::Color0, "Color 0");
	SetName(RenderTarget::Color1, "Color 1");
	SetName(RenderTarget::Obstacles, "Obstacles");
	SetName(RenderTarget::ObstacleVelocity, "Obstacle Velocity");
	SetName(RenderTarget::TempScalar, "Temp Scalar");
	SetName(RenderTarget::TempVector, "Temp Vector");
}


HRESULT FluidEngine::Initialize(uint32_t width, uint32_t height, uint32_t depth)
{
	HRESULT hr = S_OK;

	// Create 3D render targets
	for (uint32_t i = 0; i < 9; ++i)
	{
		ColorBufferPtr target = make_shared<ColorBuffer>();

		RenderTarget rt = RenderTarget(i);
		target->Create3D(GetName(rt), width, height, depth, GetFormat(rt));

		m_renderTargets[i] = target;
	}

	Reset();

	return hr;
}


void FluidEngine::Reset()
{
	GraphicsContext& context = GraphicsContext::Begin("Reset Fluid Textures");

	for(size_t i = 0; i < m_renderTargets.size(); ++i)
	{
		context.TransitionResource(*m_renderTargets[i], ResourceState::RenderTarget);
		context.ClearColor(*m_renderTargets[i]);
	}

	context.Finish(true);
}


void FluidEngine::SetFormat(RenderTarget target, Format format)
{
	m_renderTargetFormats[uint32_t(target)] = format;
}


Format FluidEngine::GetFormat(RenderTarget target) const
{
	return m_renderTargetFormats[uint32_t(target)];
}


void FluidEngine::SetName(RenderTarget target, const string& name)
{
	m_renderTargetNames[uint32_t(target)] = name;
}


const string& FluidEngine::GetName(RenderTarget target) const
{
	return m_renderTargetNames[uint32_t(target)];
}