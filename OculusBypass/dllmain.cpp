#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <string>
#include <iostream>
#include <xinput.h>

#include "MinHook.h"
#if defined(_DEBUG)
#include "spdlog/spdlog.h"
#include "spdlog/sinks/wincolor_sink.h"

std::shared_ptr<spdlog::logger> Log;
#endif

typedef HMODULE(__stdcall* _LoadLibrary)(LPCWSTR lpFileName);
typedef HANDLE(__stdcall* _OpenEvent)(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName);
typedef DWORD(__stdcall* _SearchPath)(LPCWSTR lpPath, LPCWSTR lpFileName, LPCWSTR lpExtension, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart);
_LoadLibrary TrueLoadLibrary;
_OpenEvent TrueOpenEvent;
_SearchPath TrueSearchPath;

WCHAR selfModuleName[MAX_PATH];
WCHAR ovrModuleName[MAX_PATH];
const char* pBitDepth;

#if defined(_DEBUG)
void InitializeLogging()
{
	std::vector<spdlog::sink_ptr> sinks;
	AllocConsole();
	sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>());
	Log = std::make_shared<spdlog::logger>("OculusBypass", sinks.begin(), sinks.end());
	spdlog::register_logger(Log);
	Log->set_level(spdlog::level::debug);
	Log->flush_on(spdlog::level::debug);

	char processFileName[32767] = { 0 };
	GetModuleFileNameA(nullptr, processFileName, sizeof(processFileName));
	//Log->info("{} loaded into {}", Title, processFileName);
	//Log->info("Command line: {}", GetCommandLine());
}
#endif

HANDLE WINAPI HookOpenEvent(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
	if (wcscmp(lpName, L"OculusHMDConnected") == 0)
	{
#if defined(_DEBUG)
		Log->info("Returned false for OculusHMDConnected call");
#endif
		return NULL;
	}

	return TrueOpenEvent(dwDesiredAccess, bInheritHandle, lpName);
}

HMODULE WINAPI HookLoadLibrary(LPCWSTR lpFileName)
{
	LPCWSTR name = PathFindFileNameW(lpFileName);
	LPCWSTR ext = PathFindExtensionW(name);
	size_t length = ext - name;

	if (wcsncmp(name, ovrModuleName, length) == 0)
	{
#if defined(_DEBUG)
		Log->info("Blocked loading of the Oculus runtime library");
#endif
		return NULL;
	}

	return TrueLoadLibrary(lpFileName);
}

DWORD WINAPI HookSearchPath(LPCWSTR lpPath, LPCWSTR lpFileName, LPCWSTR lpExtension, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart)
{
	if (wcscmp(lpFileName, ovrModuleName) == 0)
	{
#if defined(_DEBUG)
		Log->info("Blocked Oculus runtime library search");
#endif
		return NULL;
	}

	return TrueSearchPath(lpPath, lpFileName, lpExtension, nBufferLength, lpBuffer, lpFilePart);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
#if defined(_WIN64)
	pBitDepth = "64";
#else
	pBitDepth = "32";
#endif
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
#if defined(_DEBUG)
			InitializeLogging();
#endif
			GetModuleFileName((HMODULE)hModule, selfModuleName, MAX_PATH);
			swprintf(ovrModuleName, MAX_PATH, L"LibOVRRT%hs_1.dll", pBitDepth);
			MH_Initialize();
			MH_CreateHook(LoadLibraryW, HookLoadLibrary, (PVOID*)&TrueLoadLibrary);
			MH_CreateHook(OpenEventW, HookOpenEvent, (PVOID*)&TrueOpenEvent);
			MH_CreateHook(SearchPathW, HookSearchPath, (PVOID*)&TrueSearchPath);
			MH_EnableHook(MH_ALL_HOOKS);
#if defined(_DEBUG)
			Log->info("Hooks initialized");
#endif
			break;
		case DLL_PROCESS_DETACH:
			MH_Uninitialize();
			break;
		default:
			break;
	}
	return TRUE;
}
