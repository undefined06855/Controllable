#pragma once
#include <steamworks/isteaminput.h>

using SteamInput_t = ISteamInput*(*)();
using Init_t = void(*)(ISteamInput* self);
using RunFrame_t = void(*)(ISteamInput* self);
using GetConnectedControllers_t = int(*)(ISteamInput* self, InputHandle_t* handlesOut);
using GetInputTypeForHandle_t = ESteamInputType(*)(ISteamInput* self, InputHandle_t inputHandle);
using ShowBindingPanel_t = bool(*)(ISteamInput* self, InputHandle_t inputHandle);

extern SteamInput_t _SteamInput;
extern Init_t _Init;
extern RunFrame_t _RunFrame;
extern GetConnectedControllers_t _GetConnectedControllers;
extern GetInputTypeForHandle_t _GetInputTypeForHandle;
extern ShowBindingPanel_t _ShowBindingPanel;
