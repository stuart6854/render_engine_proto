#pragma once

#include <cstdint>
#include <vector>

namespace rde
{
	struct MeshData
	{
		std::vector<uint16_t> indices;
		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> tangents;
		std::vector<float> texCoords;
	};

	struct MeshInfo
	{
		/* The number of lods available */
		uint32_t lodLevels;
		/* The distance at which each lod should be used. Must be an entry for each lod */
		std::vector<float> mipDistances;
	};

} // namespace rde