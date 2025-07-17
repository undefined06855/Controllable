#include "steaminput.hpp"

SteamInput_t _SteamInput = nullptr;
Init_t _Init = nullptr;
RunFrame_t _RunFrame = nullptr;
GetConnectedControllers_t _GetConnectedControllers = nullptr;
GetInputTypeForHandle_t _GetInputTypeForHandle = nullptr;
ShowBindingPanel_t _ShowBindingPanel = nullptr;

$execute {
    auto steam = GetModuleHandle("steam_api64.dll");

    if (!steam) {
        geode::log::error("no steam? pirata?");
    }

    _SteamInput = (SteamInput_t)GetProcAddress(steam, "SteamAPI_SteamInput_v006");
    _Init = (Init_t)GetProcAddress(steam, "SteamAPI_ISteamInput_Init");
    _RunFrame = (RunFrame_t)GetProcAddress(steam, "SteamAPI_ISteamInput_RunFrame");
    _GetConnectedControllers = (GetConnectedControllers_t)GetProcAddress(steam, "SteamAPI_ISteamInput_GetConnectedControllers");
    _GetInputTypeForHandle = (GetInputTypeForHandle_t)GetProcAddress(steam, "SteamAPI_ISteamInput_GetInputTypeForHandle");
    _ShowBindingPanel = (ShowBindingPanel_t)GetProcAddress(steam, "SteamAPI_ISteamInput_ShowBindingPanel");
}
