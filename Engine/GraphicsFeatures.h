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
	ImageCubeArray,
	IndependentBlend,
	GeometryShader,
	TessellationShader,
	SampleRateShading,
	DualSrcBlend,
	LogicOp,
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
};

class GraphicsFeatureProxy
{
public:
	GraphicsFeatureProxy(GraphicsFeatureSet* featureSet, const std::string& name, GraphicsFeature feature);

	operator bool() const { return m_enabled; }

	GraphicsFeatureProxy& operator=(bool enabled)
	{
		m_enabled = enabled;
		return *this;
	}

	GraphicsFeature GetFeature() const { return m_feature; }

	const std::string& GetName() const { return m_name; }

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
	GraphicsFeatureProxy robustBufferAccess{ this, "Robust Buffer Access", GraphicsFeature::TessellationShader };
	GraphicsFeatureProxy fullDrawIndexUint32{ this, "Full Draw Index Uint32", GraphicsFeature::FullDrawIndexUint32 };
	GraphicsFeatureProxy imageCubeArray{ this, "Image Cube Array", GraphicsFeature::ImageCubeArray };
	GraphicsFeatureProxy independentBlend{ this, "Independent Blend", GraphicsFeature::IndependentBlend };
	GraphicsFeatureProxy geometryShader{ this, "Geometry Shader", GraphicsFeature::GeometryShader };
	GraphicsFeatureProxy tessellationShader{ this, "Tessellation Shader", GraphicsFeature::TessellationShader };
	GraphicsFeatureProxy sampleRateShading{ this, "Sample Rate Shading", GraphicsFeature::SampleRateShading };
	GraphicsFeatureProxy dualSrcBlend{ this, "Dual Src Blend", GraphicsFeature::DualSrcBlend };
	GraphicsFeatureProxy logicOp{ this, "Logic Op", GraphicsFeature::LogicOp };
	GraphicsFeatureProxy depthClamp{ this, "Depth Clamp", GraphicsFeature::DepthClamp };
	GraphicsFeatureProxy depthBiasClamp{ this, "Depth Bias Clamp", GraphicsFeature::DepthBiasClamp };
	GraphicsFeatureProxy fillModeNonSolid{ this, "Fill Mode Non-Solid", GraphicsFeature::FillModeNonSolid };
	GraphicsFeatureProxy depthBounds{ this, "Depth Bounds", GraphicsFeature::DepthBounds };
	GraphicsFeatureProxy wideLines{ this, "Wide Lines", GraphicsFeature::WideLines };
	GraphicsFeatureProxy largePoints{ this, "Large Points", GraphicsFeature::LargePoints };
	GraphicsFeatureProxy alphaToOne{ this, "Alpha to One", GraphicsFeature::AlphaToOne };
	GraphicsFeatureProxy multiViewport{ this, "Multi Viewport", GraphicsFeature::MultiViewport };
	GraphicsFeatureProxy samplerAnisotropy{ this, "Sampler Anisotropy", GraphicsFeature::SamplerAnisotropy };
	GraphicsFeatureProxy textureCompressionETC2{ this, "Texture Compression ETC2", GraphicsFeature::TextureCompressionETC2 };
	GraphicsFeatureProxy textureCompressionASTC_LDR{ this, "Texture Compression ASTC LDR", GraphicsFeature::TextureCompressionASTC_LDR };
	GraphicsFeatureProxy textureCompressionBS{ this, "Texture Compression BC", GraphicsFeature::TextureCompressionBC };

	const GraphicsFeatureProxy& operator[](size_t index) const { return *m_features[index]; }
	GraphicsFeatureProxy& operator[](size_t index) { return *m_features[index]; }

	size_t GetNumFeatures() const { return m_features.size(); }

private:
	void RegisterFeature(GraphicsFeatureProxy* featureProxy);
};

} // namespace Kodiak