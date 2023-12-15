#include "rde/Core.hpp"

namespace rde
{
	DebugCallbackFunc gDebugCallbackFunc = nullptr;

	void SetDebugCallback(const DebugCallbackFunc& callbackFunc)
	{
		gDebugCallbackFunc = callbackFunc;
	}

	void SendDebugMsg(RenderEngine* rd, DebugLevel level, std::string_view msg)
	{
		if (gDebugCallbackFunc)
			gDebugCallbackFunc(rd, level, msg);
	}

} // namespace rde