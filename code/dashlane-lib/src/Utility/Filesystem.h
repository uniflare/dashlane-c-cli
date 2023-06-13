#pragma once

namespace Utility
{

    inline std::filesystem::path GetApplicationDataFolder()
    {
        std::filesystem::path path;

#if defined(WINDOWS)
        wchar_t* wzPathBuffer;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, 0, &wzPathBuffer)))
		{
			path = wzPathBuffer;
			CoTaskMemFree(wzPathBuffer);
        }
#elif defined(MACOS)
        path = "/Library/Application Support";
#else
        path = "/.local/share";
#endif

        return path;
    }

#ifdef WINDOWS
	inline std::filesystem::path GetExecutablePath()
	{
		char rawPathName[MAX_PATH];
		GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
		return std::filesystem::path(rawPathName);
	}
#endif

#ifdef LINUX
	inline std::filesystem::path GetExecutablePath()
	{
		char rawPathName[PATH_MAX];
		realpath(PROC_SELF_EXE, rawPathName);
		return std::filesystem::path(rawPathName);
	}
#endif

#ifdef MACOS
	inline std::filesystem::path GetExecutablePath()
	{
		char rawPathName[PATH_MAX];
		char realPathName[PATH_MAX];
		uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);

		if (!_NSGetExecutablePath(rawPathName, &rawPathSize))
		{
			realpath(rawPathName, realPathName);
		}

		return  std::filesystem::path(realPathName);
	}
#endif

	inline std::filesystem::path GetExecutableDir()
	{
		return GetExecutablePath().parent_path();
	}

}