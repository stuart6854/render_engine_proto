#pragma once

#include "Mesh.hpp"

#include <VkMana/Context.hpp>

#include <VkMana/Pipeline.hpp>
#include <VkMana/WSI.hpp>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace rde
{
	using MeshLoadFunc = std::function<void(uint64_t id, uint32_t lodLevel, MeshData&)>;

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

#if 0
		/**
		 * @brief Register a instance to be rendered with a mesh instance
		 *
		 * @param instanceId
		 * @param meshId
		 */
		void RegisterMeshInstance(uint64_t instanceId, uint64_t meshId);
		void UnregisterMeshInstance(uint64_t instanceId);

		/**
		 * @brief Add flags that control how a mesh instance is rendered.
		 *
		 * @param instanceId
		 * @param flag
		 */
		void AddMeshInstanceFlag(uint64_t instanceId, uint32_t flag);
		void RemoveMeshInstanceFlag(uint64_t instanceId, uint32_t flag);
#endif

		void Submit(uint64_t meshId, uint32_t flags /*, Mat4 transform*/);

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
		void SortBuckets();
		void ClearBuckets();

	private:
		VkMana::WSI* m_window = nullptr;
		VkMana::Context m_ctx;
		VkMana::PipelineHandle m_pipeline = nullptr;

		MeshLoadFunc m_meshLoadFunc;

		std::unordered_map<uint64_t, MeshData> m_registeredMeshMap;

		struct MeshInstance
		{
			uint64_t id;
			uint64_t meshId;
			uint32_t flags;
		};
		std::unordered_map<uint64_t, MeshInstance> m_registeredMeshInstancesMap;

		std::unordered_map<uint64_t, MeshData> m_meshLoadDataMap; // Key=Hash(MeshId+LodLevel)

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