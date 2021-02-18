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

namespace Kodiak
{

class GraphicsFeatureSet;

enum class GraphicsFeature
{
	RobustBufferAccess,
	FullDrawIndexUint32,
	TextureCubeArray,
	IndependentBlend,
	GeometryShader,
	TessellationShader,
	SampleRateShading,
	DualSrcBlend,
	LogicOp,
	MultiDrawIndirect,
	DrawIndirectFirstInstance,
	DepthClamp,
	DepthBiasClamp,
	FillModeNonSolid,
	DepthBounds,
	WideLines,
	LargePoints,
	AlphaToOne,
	MultiViewport,
	SamplerAnisotropy,
	TextureCompressionETC2,
	TextureCompressionASTC_LDR,
	TextureCompressionBC,
	OcclusionQueryPrecise,
	PipelineStatisticsQuery,
	VertexPipelineStoresAndAtomics,
	PixelShaderStoresAndAtomics,
	ShaderTessellationAndGeometryPointSize,
	ShaderTextureGatherExtended,
	ShaderUAVExtendedFormats,
	ShaderUAVMultisample,

	ShaderClipDistance,
	ShaderCullDistance,
	ShaderFloat64,
	ShaderFloat16,
	ShaderInt64,
	ShaderInt16,
	ShaderInt8,

	VariableMultisampleRate,
	ImagelessFramebuffer,
};

class GraphicsFeatureProxy
{
public:
	GraphicsFeatureProxy(GraphicsFeatureSet* featureSet, GraphicsFeature feature);

	operator bool() const { return m_enabled; }

	GraphicsFeatureProxy& operator=(bool enabled)
	{
		m_enabled = enabled;
		return *this;
	}

	GraphicsFeature GetFeature() const { return m_feature; }

	const std::string GetName() const;

protected:
	const std::string m_name;
	const GraphicsFeature m_feature;
	bool m_enabled{ false };
};


class GraphicsFeatureSet : NonCopyable
{
	friend class GraphicsFeatureProxy;

	std::vector<GraphicsFeatureProxy*> m_features;

public:
	GraphicsFeatureProxy robustBufferAccess{ this, GraphicsFeature::TessellationShader };
	GraphicsFeatureProxy fullDrawIndexUint32{ this, GraphicsFeature::FullDrawIndexUint32 };
	GraphicsFeatureProxy textureCubeArray{ this, GraphicsFeature::TextureCubeArray };
	GraphicsFeatureProxy independentBlend{ this, GraphicsFeature::IndependentBlend };
	GraphicsFeatureProxy geometryShader{ this, GraphicsFeature::GeometryShader };
	GraphicsFeatureProxy tessellationShader{ this, GraphicsFeature::TessellationShader };
	GraphicsFeatureProxy sampleRateShading{ this, GraphicsFeature::SampleRateShading };
	GraphicsFeatureProxy dualSrcBlend{ this, GraphicsFeature::DualSrcBlend };
	GraphicsFeatureProxy logicOp{ this, GraphicsFeature::LogicOp };
	GraphicsFeatureProxy multiDrawIndirect{ this, GraphicsFeature::MultiDrawIndirect };
	GraphicsFeatureProxy drawIndirectFirstInstance{ this, GraphicsFeature::DrawIndirectFirstInstance };
	GraphicsFeatureProxy depthClamp{ this, GraphicsFeature::DepthClamp };
	GraphicsFeatureProxy depthBiasClamp{ this, GraphicsFeature::DepthBiasClamp };
	GraphicsFeatureProxy fillModeNonSolid{ this, GraphicsFeature::FillModeNonSolid };
	GraphicsFeatureProxy depthBounds{ this, GraphicsFeature::DepthBounds };
	GraphicsFeatureProxy wideLines{ this, GraphicsFeature::WideLines };
	GraphicsFeatureProxy largePoints{ this, GraphicsFeature::LargePoints };
	GraphicsFeatureProxy alphaToOne{ this, GraphicsFeature::AlphaToOne };
	GraphicsFeatureProxy multiViewport{ this, GraphicsFeature::MultiViewport };
	GraphicsFeatureProxy samplerAnisotropy{ this, GraphicsFeature::SamplerAnisotropy };
	GraphicsFeatureProxy textureCompressionETC2{ this, GraphicsFeature::TextureCompressionETC2 };
	GraphicsFeatureProxy textureCompressionASTC_LDR{ this, GraphicsFeature::TextureCompressionASTC_LDR };
	GraphicsFeatureProxy textureCompressionBS{ this, GraphicsFeature::TextureCompressionBC };
	GraphicsFeatureProxy occlusionQueryPrecise{ this, GraphicsFeature::OcclusionQueryPrecise };
	GraphicsFeatureProxy pipelineStatisticsQuery{ this, GraphicsFeature::PipelineStatisticsQuery };
	GraphicsFeatureProxy vertexPipelineStoresAndAtomics{ this, GraphicsFeature::VertexPipelineStoresAndAtomics };
	GraphicsFeatureProxy pixelShaderStoresAndAtomics{ this, GraphicsFeature::PixelShaderStoresAndAtomics };
	GraphicsFeatureProxy shaderTessellationAndGeometryPointSize{ this, GraphicsFeature::ShaderTessellationAndGeometryPointSize };
	GraphicsFeatureProxy shaderTextureGatherExtended{ this, GraphicsFeature::ShaderTextureGatherExtended };
	GraphicsFeatureProxy shaderUAVExtendedFormats{ this, GraphicsFeature::ShaderUAVExtendedFormats };
	GraphicsFeatureProxy shaderUAVMultisample{ this, GraphicsFeature::ShaderUAVMultisample };

	GraphicsFeatureProxy shaderClipDistance{ this, GraphicsFeature::ShaderClipDistance };
	GraphicsFeatureProxy shaderCullDistance{ this, GraphicsFeature::ShaderCullDistance };
	GraphicsFeatureProxy shaderFloat64{ this, GraphicsFeature::ShaderFloat64 };
	GraphicsFeatureProxy shaderFloat16{ this, GraphicsFeature::ShaderFloat16 };
	GraphicsFeatureProxy shaderInt64{ this, GraphicsFeature::ShaderInt64 };
	GraphicsFeatureProxy shaderInt16{ this, GraphicsFeature::ShaderInt16 };
	GraphicsFeatureProxy shaderInt8{ this, GraphicsFeature::ShaderInt8 };

	GraphicsFeatureProxy variableMultisampleRate{ this, GraphicsFeature::VariableMultisampleRate };
	GraphicsFeatureProxy imagelessFramebuffer{ this, GraphicsFeature::ImagelessFramebuffer };

	const GraphicsFeatureProxy& operator[](size_t index) const { return *m_features[index]; }
	GraphicsFeatureProxy& operator[](size_t index) { return *m_features[index]; }

	size_t GetNumFeatures() const { return m_features.size(); }

private:
	void RegisterFeature(GraphicsFeatureProxy* featureProxy);
};

extern GraphicsFeatureSet g_requiredFeatures;
extern GraphicsFeatureSet g_optionalFeatures;
extern const GraphicsFeatureSet g_enabledFeatures;


const std::string GraphicsFeatureToString(GraphicsFeature feature);

} // namespace Kodiak