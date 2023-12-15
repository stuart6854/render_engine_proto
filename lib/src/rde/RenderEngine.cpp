#include "RenderEngine.hpp"

namespace rde
{
	void RenderEngine::RegisterMesh(uint64_t id /* MeshData */) {}

	void RenderEngine::UnregsisterMesh(uint64_t id) {}

	void RenderEngine::RegisterMaterial(uint64_t id) {}

	void RenderEngine::UnregsisterMaterial(uint64_t materialId) {}

	void RenderEngine::RegisterMeshInstance(uint64_t instanceId, uint64_t meshId) {}

	void RenderEngine::UnregsisterMeshInstance(uint64_t instanceId) {}

	void RenderEngine::RegisterLight_Directional(uint64_t id /*, Position, Direction, Color, Intensity */) {}

	void RenderEngine::RegisterLight_Area(uint64_t id /*, Position, Radius, Color, Intensity */) {}

	void RenderEngine::RegisterLight_Spot(uint64_t id /*, Position, Direction, Angle, Color, Intensity */) {}

	void RenderEngine::UnregisterLight(uint64_t lightId) {}

	void RenderEngine::RegisterPostProcess(uint64_t id, const PostProcessFunc& func) {}

	void RenderEngine::UnregisterPostProcess(uint64_t postFxId) {}

	void RenderEngine::Flush() {}

} // namespace rde