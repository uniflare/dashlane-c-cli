#pragma once

namespace Utility
{

	inline uint64_t GetUnixTimestamp()
	{
		using TSystemClock = std::chrono::system_clock;
		using TSeconds = std::chrono::seconds;
		using TDuration = std::chrono::system_clock::duration;

		const TDuration span = TSystemClock::now().time_since_epoch();
		return std::chrono::duration_cast<TSeconds>(span).count();
	}

}