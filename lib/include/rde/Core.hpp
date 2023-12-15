#pragma once

#include <functional>
#include <string_view>

namespace rde
{
	// #TODO: Do we need comments here?
	enum class ErrCode
	{
		Success = 0,
		/* Id already exists */
		ExistingId,
		/* Id is invalid */
		InvalidId,
	};

	class RenderEngine;

	enum class DebugLevel
	{
		Info,
		Warn,
		Error,
	};

	using DebugCallbackFunc = std::function<void(RenderEngine* rd, DebugLevel level, std::string_view msg)>;

	void SetDebugCallback(const DebugCallbackFunc& callbackFunc);

	void SendDebugMsg(RenderEngine* rd, DebugLevel level, std::string_view msg);

} // namespace rde