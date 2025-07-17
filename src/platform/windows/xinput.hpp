#pragma once

using XInputGetState_t = DWORD (WINAPI*)(DWORD dwUserIndex, XINPUT_STATE* pState);
using XInputSetState_t = DWORD (WINAPI*)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

extern XInputGetState_t _XInputGetState;
extern XInputSetState_t _XInputSetState;
