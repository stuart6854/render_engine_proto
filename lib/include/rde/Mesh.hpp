#pragma once

#include <VkMana/Buffer.hpp>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include <cstdint>
#include <vector>

namespace rde
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct MeshData
	{
		VkMana::BufferHandle vertexBuffer;
		VkMana::BufferHandle indexBuffer;
		uint64_t vertexCount = 0;
		uint64_t indexCount = 0;

		uint64_t firstVertex = 0; // Used internally
		uint64_t firstIndex = 0;  // Used internally
	};

	struct MeshInfo
	{
		/* The number of lods available */
		uint32_t lodLevels;
		/* The distance at which each lod should be used. Must be an entry for each lod */
		std::vector<float> mipDistances;
	};

} // namespace rde