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

#include "Graphics\ColorBuffer.h"


class FluidEngine
{
public:
	enum class RenderTarget
	{
		Velocity0,
		Velocity1,
		Pressure,
		Color0,
		Color1,
		Obstacles,
		ObstacleVelocity,
		TempScalar,
		TempVector
	};

	FluidEngine();

	HRESULT Initialize(uint32_t width, uint32_t height, uint32_t depth);
	void Reset();

private:
	void SetFormat(RenderTarget target, Kodiak::Format format);
	Kodiak::Format GetFormat(RenderTarget target) const;
	void SetName(RenderTarget target, const std::string& name);
	const std::string& GetName(RenderTarget target) const;

private:
	// 3D color buffers
	std::array<Kodiak::Format, 9> m_renderTargetFormats;
	std::array<std::string, 9> m_renderTargetNames;
	std::array<Kodiak::ColorBufferPtr, 9> m_renderTargets;
};