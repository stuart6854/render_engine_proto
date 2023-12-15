#include <rde/Core.hpp>
#include <rde/RenderEngine.hpp>

#include <spdlog/spdlog.h>

#include <string_view>
#include <thread>

int main(int argc, char** argv)
{
	using namespace rde;

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

#pragma region Register Render Data

	constexpr auto MESH_ID_CUBE = 1;

#pragma endregion

	bool isRunning = true;
	while (isRunning)
	{
		engine.Flush();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}