#include "rde/RenderEngine.hpp"

#include "Logging.hpp"
#include "rde/Mesh.hpp"

#include <VkMana/ShaderCompiler.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>

constexpr auto GeometryShaderHLSL = R"(
struct VSInput
{
	[[vk::location(0)]] float3 Position : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PushConsts
{
	float4x4 viewProjMatrix;
	float4x4 modelMatrix;
};
[[vk::push_constant]] PushConsts consts;

struct VSOutput
{
	float4 FragPos : SV_POSITION;
	[[vk::location(0)]] float3 WorldPos : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;

	output.FragPos = mul(consts.viewProjMatrix, mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)));

	output.WorldPos = mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)).xyz;
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(input.Normal);
	output.Tangent = normalize(input.Tangent);
	return output;
}

struct PSInput
{
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PSOutput
{
	float4 FragColor : SV_TARGET0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput output;

	// output.Albedo = textureColor.Sample(samplerColor, input.UV);
	output.FragColor = float4(1, 1, 1, 1);

	return output;
}
)";

namespace rde
{
	constexpr auto GenSortKey() -> uint64_t
	{
		return 0;
	}

	bool RenderEngine::Init(VkMana::WSI& window)
	{
		m_window = &window;

		if (!m_ctx.Init(m_window))
		{
			RDE_ERROR("Failed to initialise VkMana context");
			return false;
		}

		{
			const VkMana::PipelineLayoutCreateInfo layoutInfo{
				.PushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, 64 * 2 },
				.SetLayouts = {},
			};
			auto pipelineLayout = m_ctx.CreatePipelineLayout(layoutInfo);

			VkMana::ShaderCompileInfo compileInfo{
				.SrcLanguage = VkMana::SourceLanguage::HLSL,
				.SrcString = GeometryShaderHLSL,
				.Stage = vk::ShaderStageFlagBits::eVertex,
				.EntryPoint = "VSMain",
				.Debug = false,
			};
			auto vertexSpirvOpt = CompileShader(compileInfo);
			if (!vertexSpirvOpt)
			{
				VM_ERR("Failed to read/compile Vertex shader.");
				return false;
			}

			compileInfo.Stage = vk::ShaderStageFlagBits::eFragment;
			compileInfo.EntryPoint = "PSMain";
			auto fragmentSpirvOpt = CompileShader(compileInfo);
			if (!fragmentSpirvOpt)
			{
				VM_ERR("Failed to read/compile Fragment shader.");
				return false;
			}

			const VkMana::GraphicsPipelineCreateInfo pipelineInfo{
				.Vertex = { vertexSpirvOpt.value(), "VSMain" },
				.Fragment = { fragmentSpirvOpt.value(), "PSMain" },
				.VertexAttributes = {
					vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, 0),
					vk::VertexInputAttributeDescription(1, 1, vk::Format::eR32G32Sfloat, 0),
					vk::VertexInputAttributeDescription(2, 2, vk::Format::eR32G32B32Sfloat, 0),
					vk::VertexInputAttributeDescription(3, 3, vk::Format::eR32G32B32Sfloat, 0),
				},
				.VertexBindings = {
					vk::VertexInputBindingDescription(0, sizeof(float) * 3, vk::VertexInputRate::eVertex),
					vk::VertexInputBindingDescription(1, sizeof(float) * 2, vk::VertexInputRate::eVertex),
					vk::VertexInputBindingDescription(2, sizeof(float) * 3, vk::VertexInputRate::eVertex),
					vk::VertexInputBindingDescription(3, sizeof(float) * 3, vk::VertexInputRate::eVertex),
				},
				.Topology =  vk::PrimitiveTopology::eTriangleList,
				.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
				.Layout = pipelineLayout,
			};
			m_pipeline = m_ctx.CreateGraphicsPipeline(pipelineInfo);
		}

		return true;
	}

	void RenderEngine::SetMeshLoadFunc(const MeshLoadFunc& func)
	{
		m_meshLoadFunc = func;
	}

	void RenderEngine::RegisterMesh(uint64_t id, const MeshInfo& meshInfo) {}

	void RenderEngine::UnregisterMesh(uint64_t id) {}

	void RenderEngine::RegisterMaterial(uint64_t id) {}

	void RenderEngine::UnregisterMaterial(uint64_t materialId) {}

#if 0
	void RenderEngine::RegisterMeshInstance(uint64_t instanceId, uint64_t meshId) {}

	void RenderEngine::UnregsisterMeshInstance(uint64_t instanceId) {}
#endif

	void RenderEngine::Submit(uint64_t meshId, uint32_t flags /*, Mat4 transform*/)
	{
		const auto sortKey = GenSortKey();

		m_geometryBucket.push_back(sortKey);
		if (flags & MESH_INST_FLAG_CASTS_SHADOW)
			m_shadowBucket.push_back(sortKey);
	}

	void RenderEngine::RegisterLight_Directional(uint64_t id /*, Position, Direction, Color, Intensity */) {}

	void RenderEngine::RegisterLight_Area(uint64_t id /*, Position, Radius, Color, Intensity */) {}

	void RenderEngine::RegisterLight_Spot(uint64_t id /*, Position, Direction, Angle, Color, Intensity */) {}

	void RenderEngine::UnregisterLight(uint64_t lightId) {}

	void RenderEngine::RegisterPostProcess(uint64_t id, const PostProcessFunc& func) {}

	void RenderEngine::UnregisterPostProcess(uint64_t postFxId) {}

	void RenderEngine::Flush()
	{
		const auto windowWidth = m_window->GetSurfaceWidth();
		const auto windowHeight = m_window->GetSurfaceHeight();

		SortBuckets();

		m_ctx.BeginFrame();

		auto cmd = m_ctx.RequestCmd();

		const auto rpInfo = m_ctx.GetSurfaceRenderPass(m_window);
		cmd->BeginRenderPass(rpInfo);

		cmd->BindPipeline(m_pipeline.Get());
		cmd->SetViewport(0, 0, float(windowWidth), float(windowHeight));
		cmd->SetScissor(0, 0, windowWidth, windowHeight);

		for (const auto& inst : m_geometryBucket)
		{
			cmd->Draw(3, 0);
		}

		cmd->EndRenderPass();

		m_ctx.EndFrame();
		m_ctx.Present();

		ClearBuckets();
	}

	void RenderEngine::SortBuckets()
	{
		std::sort(m_shadowBucket.begin(), m_shadowBucket.end(), std::less<uint64_t>());
		std::sort(m_geometryBucket.begin(), m_geometryBucket.end(), std::less<uint64_t>());
	}

	void RenderEngine::ClearBuckets()
	{
		m_geometryBucket.clear();
		m_shadowBucket.clear();
	}

} // namespace rde