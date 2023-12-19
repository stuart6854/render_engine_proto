#pragma once

#include "Mesh.hpp"
#include "PackedArray.hpp"

#include <VkMana/Context.hpp>
#include <VkMana/Pipeline.hpp>
#include <VkMana/WSI.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

namespace rde
{
	using MeshLoadFunc = std::function<void(uint64_t id, uint32_t lodLevel, VkMana::Context& ctx, MeshData&)>;

	using PostProcessFunc = std::function<void()>;

	constexpr auto MESH_INST_FLAG_CASTS_SHADOW = 1u << 0;

	class RenderEngine
	{
	public:
		bool Init(VkMana::WSI& window);

		/**
		 * @brief Set the function to be called when the engine wants to load a mesh.
		 *
		 * @param func
		 */
		void SetMeshLoadFunc(const MeshLoadFunc& func);

		/**
		 * @brief Register a mesh that can be rendered by a mesh instance
		 */
		void RegisterMesh(uint64_t id, const MeshInfo& meshInfo);
		void UnregisterMesh(uint64_t id);

		/**
		 * @brief Register a material that can be used a mesh instance
		 *
		 * @param id
		 */
		void RegisterMaterial(uint64_t id);
		void UnregisterMaterial(uint64_t materialId);

		/**
		 * @brief Register a instance to be rendered with a mesh instance
		 *
		 * @param instanceId
		 * @param meshId
		 */
		void RegisterMeshInstance(uint64_t instanceId, uint64_t meshId);
		void UnregisterMeshInstance(uint64_t instanceId);

		void SetMeshInstanceTransform(uint64_t instanceId, const glm::mat4& transform);

		/**
		 * @brief Add flags that control how a mesh instance is rendered.
		 *
		 * @param instanceId
		 * @param flag
		 */
		void AddMeshInstanceFlag(uint64_t instanceId, uint32_t flag);
		void RemoveMeshInstanceFlag(uint64_t instanceId, uint32_t flag);

		// void Submit(uint64_t meshId, uint32_t flags /*, Mat4 transform*/);

		/**
		 * @brief Register light
		 */
		void RegisterLight_Directional(uint64_t id /*, Position, Direction, Color, Intensity */);
		void RegisterLight_Area(uint64_t id /*, Position, Radius, Color, Intensity */);
		void RegisterLight_Spot(uint64_t id /*, Position, Direction, Angle, Color, Intensity */);
		void UnregisterLight(uint64_t lightId);

		void RegisterPostProcess(uint64_t id, const PostProcessFunc& func);
		void UnregisterPostProcess(uint64_t postFxId);

		/**
		 * @brief Render scene
		 */
		void Flush();

	private:
		void BuildMeshBuffers();

		void SortBuckets();
		void ClearBuckets();

	private:
		VkMana::WSI* m_window = nullptr;
		VkMana::Context m_ctx;

		VkMana::ImageHandle m_depthTarget = nullptr;
		VkMana::PipelineHandle m_pipeline = nullptr;

		MeshLoadFunc m_meshLoadFunc;

		struct RegisteredMesh
		{
			MeshInfo info;
			std::vector<MeshData> lodData;
			std::vector<std::atomic_bool> perLodIsLoaded;
		};
		std::unordered_map<uint64_t, RegisteredMesh> m_registeredMeshMap;

		/* Registered Instance Data */
		PackedArray<uint64_t> m_instanceMeshes;
		PackedArray<glm::mat4> m_instanceTransforms;
		PackedArray<uint32_t> m_instanceFlags;

		std::mutex m_cacheMutex;

		/**** Bindless Data ****/

		VkMana::BufferHandle m_vertexBuffer;
		VkMana::BufferHandle m_indexBuffer;
		bool m_meshBuffersDirty = true;

		/**** Buckets ****/
		std::vector<uint64_t> m_shadowBucket;
		std::vector<uint64_t> m_geometryBucket;

		// NOTE: Considerations https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/#bindless-descriptor-designs
		// - Storage Buffers
		// 	- TransformData
		// 	- MaterialData (How would one handle multiple material types?)
		//	-
	};

} // namespace rde