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
	GLFWwindow* m_window;
};

static void LoadMesh(const std::filesystem::path& filename, rde::MeshData& outMeshData)
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

	for (auto i = 0u; i < scene->mNumMeshes; ++i)
	{
		const auto* mesh = scene->mMeshes[i];

		for (auto j = 0u; j < mesh->mNumVertices; ++j)
		{
			outMeshData.positions.push_back(mesh->mVertices[j].x);
			outMeshData.positions.push_back(mesh->mVertices[j].y);
			outMeshData.positions.push_back(mesh->mVertices[j].z);
			if (mesh->HasTextureCoords(0))
			{
				outMeshData.texCoords.push_back(mesh->mTextureCoords[0][j].x);
				outMeshData.texCoords.push_back(mesh->mTextureCoords[0][j].y);
			}
			if (mesh->HasNormals())
			{
				outMeshData.normals.push_back(mesh->mNormals[j].x);
				outMeshData.normals.push_back(mesh->mNormals[j].y);
				outMeshData.normals.push_back(mesh->mNormals[j].z);
			}
			if (mesh->HasTangentsAndBitangents())
			{
				outMeshData.tangents.push_back(mesh->mTangents[j].x);
				outMeshData.tangents.push_back(mesh->mTangents[j].y);
				outMeshData.tangents.push_back(mesh->mTangents[j].z);
			}
		}

		for (auto j = 0u; j < mesh->mNumFaces; ++j)
		{
			const auto& face = mesh->mFaces[j];
			for (auto k = 0u; k < face.mNumIndices; ++k)
				outMeshData.indices.push_back(face.mIndices[k]);
		}
	}
}

/* Call from a worker thread. */
static void LoadMesh(uint64_t meshId, uint32_t lodLevel, rde::MeshData& outMeshData)
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

	meshFilename.replace_filename(meshFilename.filename().string() + "_lod" + std ::to_string(lodLevel));
	if (!exists(meshFilename))
	{
		spdlog::log(spdlog::level::err, "Mesh file does not exist: {}", meshFilename.string());
		return;
	}

	LoadMesh(meshFilename, outMeshData);
}

int main(int argc, char** argv)
{
	using namespace rde;

	Window window{};
	if (!window.Init())
	{
		spdlog::log(spdlog::level::err, "Failed to initialise window");
		return 1;
	}

	rde::SetDebugCallback([](RenderEngine* rd, DebugLevel level, std::string_view msg) {
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

	RenderEngine engine{};
	if (!engine.Init(window))
	{
		spdlog::log(spdlog::level::err, "Failed to initialise RenderEngine");
		return 1;
	}

	engine.SetMeshLoadFunc([](uint64_t meshId, uint32_t lodLevel, MeshData& outMeshData) { LoadMesh(meshId, lodLevel, outMeshData); });

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
			.lodLevels = 1,
			.mipDistances = { 0.0f },
		});

	engine.RegisterMesh(
		MESH_ID_ZELDA_BACKPACK,
		{
			.lodLevels = 1,
			.mipDistances = { 0.0f },
		});

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

		constexpr auto SIZE = 1u;
		for (auto y = 0; y < SIZE; ++y)
		{
			for (auto x = 0; x < SIZE; ++x)
			{
				// Mat4 transform = Translation(x, 0, y);
				engine.Submit(MESH_ID_CUBE, MESH_INST_FLAG_CASTS_SHADOW /*, transform*/);
			}
		}
		// for (auto y = 0; y < SIZE; ++y)
		//{
		//	for (auto x = 0; x < SIZE; ++x)
		//	{
		//		// Mat4 transform = Translation(x, 5, y);
		//		engine.Submit(MESH_ID_BUNNY, MESH_INST_FLAG_CASTS_SHADOW /*, transform*/);
		//	}
		// }
		// for (auto y = 0; y < SIZE; ++y)
		//{
		//	for (auto x = 0; x < SIZE; ++x)
		//	{
		//		// Mat4 transform = Translation(x, 10, y);
		//		engine.Submit(MESH_ID_ZELDA_BACKPACK, MESH_INST_FLAG_CASTS_SHADOW /*, transform*/);
		//	}
		// }

		engine.Flush();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}