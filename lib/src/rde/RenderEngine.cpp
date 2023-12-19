#include "rde/RenderEngine.hpp"

#include "Logging.hpp"
#include "rde/Core.hpp"
#include "rde/Mesh.hpp"

#include <VkMana/ShaderCompiler.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>

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
			const auto pipelineLayout = m_ctx.CreatePipelineLayout(layoutInfo);

			VkMana::ShaderCompileInfo compileInfo{
				.SrcLanguage = VkMana::SourceLanguage::HLSL,
				.SrcFilename = "data/shaders/Geometry.hlsl",
				.Stage = vk::ShaderStageFlagBits::eVertex,
				.EntryPoint = "VSMain",
				.Debug = false,
			};
			const auto vertexSpirvOpt = CompileShader(compileInfo);
			if (!vertexSpirvOpt)
			{
				VM_ERR("Failed to read/compile Vertex shader.");
				return false;
			}

			compileInfo.Stage = vk::ShaderStageFlagBits::eFragment;
			compileInfo.EntryPoint = "PSMain";
			const auto fragmentSpirvOpt = CompileShader(compileInfo);
			if (!fragmentSpirvOpt)
			{
				VM_ERR("Failed to read/compile Fragment shader.");
				return false;
			}

			const VkMana::GraphicsPipelineCreateInfo pipelineInfo{
				.Vertex = { vertexSpirvOpt.value(), "VSMain" },
				.Fragment = { fragmentSpirvOpt.value(), "PSMain" },
				.VertexAttributes = {
					vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
					vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)),
					vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
					vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent)),
				},
				.VertexBindings = {
					vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex),
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

	void RenderEngine::RegisterMesh(uint64_t id, const MeshInfo& meshInfo)
	{
		auto [it, success] = m_registeredMeshMap.emplace(id, RegisteredMesh{});
		auto& registeredMesh = it->second;
		registeredMesh.info = meshInfo;
		registeredMesh.lodData.resize(registeredMesh.info.lodLevels);
		registeredMesh.perLodIsLoaded = std::vector<std::atomic_bool>(registeredMesh.info.lodLevels);

		m_meshLoadFunc(id, meshInfo.lodLevels - 1, m_ctx, registeredMesh.lodData[meshInfo.lodLevels - 1]);
		registeredMesh.perLodIsLoaded[meshInfo.lodLevels - 1] = true;
	}

	void RenderEngine::UnregisterMesh(uint64_t id)
	{
		m_registeredMeshMap.erase(id);

		m_meshBuffersDirty = true;
	}

	void RenderEngine::RegisterMaterial(uint64_t id) {}

	void RenderEngine::UnregisterMaterial(uint64_t materialId) {}

	void RenderEngine::RegisterMeshInstance(uint64_t instanceId, uint64_t meshId)
	{
		// #TODO: Check if meshId is valid?
		m_instanceMeshes.AddValue(instanceId, meshId);
		m_instanceTransforms.AddValue(instanceId, glm::mat4(1.0f));
		m_instanceFlags.AddValue(instanceId, 0);
	}

	void RenderEngine::UnregisterMeshInstance(uint64_t instanceId)
	{
		m_instanceMeshes.RemoveValue(instanceId);
		m_instanceTransforms.RemoveValue(instanceId);
		m_instanceFlags.RemoveValue(instanceId);
	}

	void RenderEngine::SetMeshInstanceTransform(uint64_t instanceId, const glm::mat4& transform)
	{
		if (!m_instanceTransforms.Contains(instanceId))
		{
			RDE_ERROR("Instance with id {} does not exist. It must be registered with RenderEngine::RegisterMeshInstance().", instanceId);
			return;
		}

		m_instanceTransforms.SetValue(instanceId, transform);
	}

#if 0
	void RenderEngine::Submit(uint64_t meshId, uint32_t flags /*, Mat4 transform*/)
	{
		const auto meshInfoIt = m_registeredMeshMap.find(meshId);
		if (meshInfoIt == m_registeredMeshMap.end())
		{
			RDE_ERROR("Mesh with id {} does not exist. It must be registered with RenderEngine::RegisterMesh().");
			return;
		}

		const auto& meshInfo = meshInfoIt->second;
		const auto& meshLodData = meshInfo.lodData[0];

		if (!meshInfo.perLodIsLoaded[0])
		{
			// This lod has not yet been loaded, so lets start loading it
			// #TODO: Load mesh lod
			// TODO: Should we try render with the next best lod (if available)?
			return;
		}

		const auto sortKey = GenSortKey();

		m_geometryBucket.push_back(sortKey);
		if (flags & MESH_INST_FLAG_CASTS_SHADOW)
			m_shadowBucket.push_back(sortKey);
	}
#endif

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

		if (m_meshBuffersDirty)
		{
			BuildMeshBuffers();
			m_meshBuffersDirty = false;
		}

		auto cmd = m_ctx.RequestCmd();

		const auto rpInfo = m_ctx.GetSurfaceRenderPass(m_window);
		cmd->BeginRenderPass(rpInfo);

		cmd->BindPipeline(m_pipeline.Get());
		cmd->SetViewport(0, float(windowHeight), float(windowWidth), -float(windowHeight));
		cmd->SetScissor(0, 0, windowWidth, windowHeight);

		if (m_vertexBuffer && m_indexBuffer)
		{
			cmd->BindIndexBuffer(m_indexBuffer.Get());
			cmd->BindVertexBuffers(0, { m_vertexBuffer.Get() }, { 0 });

			const auto proj = glm::perspectiveLH_ZO(glm::radians(60.0f), float(windowWidth) / float(windowHeight), 0.1f, 1000.0f);
			const auto view = glm::lookAtLH(glm::vec3{ -2, 5, -5.0f }, glm::vec3{ 50, 2, 50 }, glm::vec3{ 0, 1, 0 });
			const auto viewProj = proj * view;
			cmd->SetPushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), glm::value_ptr(viewProj));

			for (auto i = 0u; i < m_instanceMeshes.GetSize(); ++i)
			{
				const auto meshId = m_instanceMeshes.GetArray()[i];
				const auto& meshInfo = m_registeredMeshMap[meshId];
				const auto& lodData = meshInfo.lodData[meshInfo.info.lodLevels - 1];

				auto model = m_instanceTransforms.GetArray()[i];
				cmd->SetPushConstants(vk::ShaderStageFlagBits::eVertex, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));

				cmd->DrawIndexed(lodData.indexCount, lodData.firstIndex, lodData.firstVertex);
			}
		}

		cmd->EndRenderPass();

		m_ctx.Submit(cmd);

		m_ctx.EndFrame();
		m_ctx.Present();

		ClearBuckets();
	}

	void RenderEngine::BuildMeshBuffers()
	{
		RDE_INFO("Building vertex & index buffers...");

		uint64_t totalVertices = 0;
		uint64_t totalIndices = 0;

		for (auto& [id, mesh] : m_registeredMeshMap)
		{
			for (auto& lodData : mesh.lodData)
			{
				lodData.firstVertex = totalVertices;
				lodData.firstIndex = totalIndices;

				totalVertices += lodData.vertexCount;
				totalIndices += lodData.indexCount;
			}
		}

		if (totalVertices == 0 || totalIndices == 0)
		{
			RDE_INFO("No vertices/indices to build buffers with.", totalVertices, totalIndices);
			return;
		}

		const auto vtxBufInfo = VkMana::BufferCreateInfo::Vertex(totalVertices * sizeof(Vertex));
		m_vertexBuffer = m_ctx.CreateBuffer(vtxBufInfo);

		const auto idxBufInfo = VkMana::BufferCreateInfo::Index(totalIndices * sizeof(uint16_t));
		m_indexBuffer = m_ctx.CreateBuffer(idxBufInfo);

		auto cmd = m_ctx.RequestCmd();

		for (auto& [id, mesh] : m_registeredMeshMap)
		{
			for (auto i = 0u; i < mesh.info.lodLevels; ++i)
			{
				if (!mesh.perLodIsLoaded[i])
					continue;

				const auto& lodData = mesh.lodData[i];

				VkMana::BufferCopyInfo vtxCopyInfo{
					.SrcBuffer = lodData.vertexBuffer.Get(),
					.DstBuffer = m_vertexBuffer.Get(),
					.Size = lodData.vertexCount * sizeof(Vertex),
					.SrcOffset = 0,
					.DstOffset = lodData.firstVertex * sizeof(Vertex),
				};
				cmd->CopyBuffer(vtxCopyInfo);

				VkMana::BufferCopyInfo idxCopyInfo{
					.SrcBuffer = lodData.indexBuffer.Get(),
					.DstBuffer = m_indexBuffer.Get(),
					.Size = lodData.indexCount * sizeof(uint16_t),
					.SrcOffset = 0,
					.DstOffset = lodData.firstIndex * sizeof(uint16_t),
				};
				cmd->CopyBuffer(idxCopyInfo);
			}
		}

		m_ctx.SubmitStaging(cmd);

		RDE_INFO("Vertex/Index buffers built. {} vertices, {} indices.", totalVertices, totalIndices);
	}

	void RenderEngine::SortBuckets()
	{
		std::ranges::sort(m_shadowBucket, std::less());
		std::ranges::sort(m_geometryBucket, std::less());
	}

	void RenderEngine::ClearBuckets()
	{
		m_geometryBucket.clear();
		m_shadowBucket.clear();
	}

} // namespace rde