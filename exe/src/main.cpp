#include <rde/Core.hpp>
#include <rde/RenderEngine.hpp>

#include <spdlog/spdlog.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VkMana/WSI.hpp>

#include <glm/ext/matrix_transform.hpp>

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <thread>

constexpr auto MESH_ID_CUBE = 1;
constexpr auto MESH_ID_BUNNY = 2;
constexpr auto MESH_ID_ZELDA_BACKPACK = 3;
constexpr auto MESH_ID_SPARTAN_ARMOUR = 4;
constexpr auto MESH_ID_WARTHOG = 5;
constexpr auto MESH_ID_CITY_PROPS = 6;

class Window : public VkMana::WSI
{
public:
	~Window() { glfwDestroyWindow(m_window); }

	bool Init()
	{
		if (!glfwInit())
		{
			spdlog::log(spdlog::level::err, "Failed to init GLFW");
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = glfwCreateWindow(1280, 720, "RenderEngine", nullptr, nullptr);
		if (m_window == nullptr)
		{
			spdlog::log(spdlog::level::err, "Failed to create GLFW window");
			return false;
		}

		return true;
	}

	void NewFrame() { PollEvents(); }

	void PollEvents() override { glfwPollEvents(); }

	auto CreateSurface(vk::Instance instance) -> vk::SurfaceKHR override
	{
		VkSurfaceKHR surface = nullptr;
		glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
		return surface;
	}

	auto GetSurfaceWidth() -> uint32_t override
	{
		int32_t dim = 0;
		glfwGetFramebufferSize(m_window, &dim, nullptr);
		return dim;
	}

	auto GetSurfaceHeight() -> uint32_t override
	{
		int32_t dim = 0;
		glfwGetFramebufferSize(m_window, nullptr, &dim);
		return dim;
	}

	bool IsVSync() override { return true; }

	bool IsAlive() override { return !glfwWindowShouldClose(m_window); }

	void HideCursor() override {}

	void ShowCursor() override {}

	auto CreateCursor(uint32_t cursorType) -> void* override { return nullptr; }

	void SetCursor(void* cursor) override {}

private:
	GLFWwindow* m_window = nullptr;
};

static void LoadMesh(const std::filesystem::path& filename, VkMana::Context& ctx, rde::MeshData& outMeshData)
{
	constexpr auto MeshImportFlags = aiProcessPreset_TargetRealtime_Fast | aiProcess_PreTransformVertices;

	const auto& filenameStr = filename.string();

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(filenameStr.c_str(), MeshImportFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		spdlog::log(spdlog::level::err, "Failed to load mesh:");
		spdlog::log(spdlog::level::err, import.GetErrorString());
		return;
	}
	const auto rootDirectory = filename.parent_path();

	std::vector<rde::Vertex> vertices{};
	std::vector<uint16_t> indices;
	for (auto i = 0u; i < scene->mNumMeshes; ++i)
	{
		const auto* mesh = scene->mMeshes[i];

		vertices.reserve(vertices.size() + mesh->mNumVertices);
		for (auto j = 0u; j < mesh->mNumVertices; ++j)
		{
			auto& vertex = vertices.emplace_back();

			vertex.position = {
				mesh->mVertices[j].x,
				mesh->mVertices[j].y,
				mesh->mVertices[j].z,
			};
			if (mesh->HasTextureCoords(0))
			{
				vertex.texCoord = {
					mesh->mTextureCoords[0][j].x,
					mesh->mTextureCoords[0][j].y,
				};
			}
			if (mesh->HasNormals())
			{
				vertex.normal = {
					mesh->mNormals[j].x,
					mesh->mNormals[j].y,
					mesh->mNormals[j].z,
				};
			}
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent = {
					mesh->mTangents[j].x,
					mesh->mTangents[j].y,
					mesh->mTangents[j].z,
				};
			}
		}

		indices.reserve(indices.size() + mesh->mNumFaces * 3);
		for (auto j = 0u; j < mesh->mNumFaces; ++j)
		{
			const auto& face = mesh->mFaces[j];
			for (auto k = 0u; k < face.mNumIndices; ++k)
				indices.push_back(face.mIndices[k]);
		}
	}

	outMeshData.vertexCount = vertices.size();
	outMeshData.indexCount = indices.size();

	const auto vtxBufInfo = VkMana::BufferCreateInfo::Staging(vertices.size() * sizeof(rde::Vertex));
	const VkMana::BufferDataSource vtxDataSrc{ vtxBufInfo.Size, vertices.data() };
	outMeshData.vertexBuffer = ctx.CreateBuffer(vtxBufInfo, &vtxDataSrc);

	const auto idxBufInfo = VkMana::BufferCreateInfo::Staging(indices.size() * sizeof(uint16_t));
	const VkMana::BufferDataSource idxDataSrc{ idxBufInfo.Size, indices.data() };
	outMeshData.indexBuffer = ctx.CreateBuffer(idxBufInfo, &idxDataSrc);
}

/* Called from a worker thread. */
static void LoadMesh(uint64_t meshId, uint32_t lodLevel, VkMana::Context& ctx, rde::MeshData& outMeshData)
{
	std::filesystem::path meshFilename = "data/models/";
	if (meshId == MESH_ID_CUBE)
	{
		// #TODO: Load cube mesh
		meshFilename /= "cube.obj";
	}
	else if (meshId == MESH_ID_BUNNY)
	{
		// #TODO: Load bunny mesh
		meshFilename /= "stanford-bunny/stanford-bunny.obj";
	}
	else if (meshId == MESH_ID_ZELDA_BACKPACK)
	{
		// #TODO: Load zelda backpack mesh
		meshFilename /= "zelda-backpack/scene.gltf";
	}
	else if (meshId == MESH_ID_SPARTAN_ARMOUR)
	{
		// #TODO: Load spartan armour mesh
		meshFilename /= "spartan-armour/scene.gltf";
	}
	else if (meshId == MESH_ID_WARTHOG)
	{
		// #TODO: Load warthog mesh
		meshFilename /= "warthog/scene.gltf";
	}
	else if (meshId == MESH_ID_CITY_PROPS)
	{
		// #TODO: Load city props mesh
		meshFilename /= "city-props-collection/scene.gltf";
	}

	const auto extStr = meshFilename.extension().string();
	meshFilename.replace_filename(meshFilename.stem().string() + "_lod" + std ::to_string(lodLevel) + extStr);
	if (!exists(meshFilename))
	{
		spdlog::log(spdlog::level::err, "Mesh file does not exist: {}", meshFilename.string());
		return;
	}

	spdlog::log(spdlog::level::info, "Loading mesh (id={}, lod={}): {}", meshId, lodLevel, meshFilename.string());

	LoadMesh(meshFilename, ctx, outMeshData);
}

int main(int argc, char** argv)
{
	Window window{};
	if (!window.Init())
	{
		spdlog::log(spdlog::level::err, "Failed to initialise window");
		return 1;
	}

	rde::SetDebugCallback([](rde::RenderEngine* rd, rde::DebugLevel level, std::string_view msg) {
		switch (level)
		{
			case rde::DebugLevel::Info:
				spdlog::log(spdlog::level::info, msg);
				break;
			case rde::DebugLevel::Warn:
				spdlog::log(spdlog::level::warn, msg);
				break;
			case rde::DebugLevel::Error:
				spdlog::log(spdlog::level::err, msg);
				break;
		}
	});

	rde::RenderEngine engine{};
	if (!engine.Init(window))
	{
		spdlog::log(spdlog::level::err, "Failed to initialise RenderEngine");
		return 1;
	}

	engine.SetMeshLoadFunc(
		[](uint64_t meshId, uint32_t lodLevel, VkMana::Context& ctx, rde::MeshData& outMeshData) { LoadMesh(meshId, lodLevel, ctx, outMeshData); });

#pragma region Register Render Data

	engine.RegisterMesh(
		MESH_ID_CUBE,
		{
			.lodLevels = 1,
			.mipDistances = { 0.0f },
		});

	engine.RegisterMesh(
		MESH_ID_BUNNY,
		{
			.lodLevels = 2,
			.mipDistances = { 0.0f, 15.0f },
		});

	engine.RegisterMesh(
		MESH_ID_ZELDA_BACKPACK,
		{
			.lodLevels = 1,
			.mipDistances = { 0.0f },
		});

	uint32_t instId = 1;
	constexpr auto SIZE = 3u;
	for (auto y = 0; y < SIZE; ++y)
	{
		for (auto x = 0; x < SIZE; ++x)
		{
			engine.RegisterMeshInstance(instId, MESH_ID_CUBE);
			engine.SetMeshInstanceTransform(instId, glm::translate(glm::mat4(1.0f), glm::vec3{ x, 0, y } * 2.0f));
			++instId;
		}
	}
	for (auto y = 0; y < SIZE; ++y)
	{
		for (auto x = 0; x < SIZE; ++x)
		{
			engine.RegisterMeshInstance(instId, MESH_ID_BUNNY);
			engine.SetMeshInstanceTransform(instId, glm::translate(glm::mat4(1.0f), glm::vec3{ x, 1, y } * 2.0f));
			++instId;
		}
	}
	for (auto y = 0; y < SIZE; ++y)
	{
		for (auto x = 0; x < SIZE; ++x)
		{
			engine.RegisterMeshInstance(instId, MESH_ID_ZELDA_BACKPACK);
			engine.SetMeshInstanceTransform(instId, glm::translate(glm::mat4(1.0f), glm::vec3{ x, 2, y } * 2.0f));
			++instId;
		}
	}

#pragma endregion

	bool isRunning = true;
	while (isRunning)
	{
		window.NewFrame();
		if (!window.IsAlive())
		{
			isRunning = false;
			break;
		}

		engine.Flush();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}