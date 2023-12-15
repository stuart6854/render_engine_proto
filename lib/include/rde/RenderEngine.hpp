#pragma once

#include "Mesh.hpp"

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace rde
{
	using MeshLoadFunc = std::function<void(uint64_t id, uint32_t lodLevel)>;

	using PostProcessFunc = std::function<void()>;

	class RenderEngine
	{
	public:
		/**
		 * @brief Register a mesh that can be rendered by a mesh instance
		 */
		void RegisterMesh(uint64_t id, const MeshInfo& meshInfo);
		void UnregsisterMesh(uint64_t id);

		/**
		 * @brief Register a material that can be used a mesh instance
		 *
		 * @param id
		 */
		void RegisterMaterial(uint64_t id);
		void UnregsisterMaterial(uint64_t materialId);

		/**
		 * @brief Register a instance to be rendered with a mesh instance
		 *
		 * @param instanceId
		 * @param meshId
		 */
		void RegisterMeshInstance(uint64_t instanceId, uint64_t meshId);
		void UnregsisterMeshInstance(uint64_t instanceId);

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
		std::unordered_map<uint64_t, MeshData> m_registeredMeshMap;
	};

} // namespace rde