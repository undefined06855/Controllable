#pragma once

typedef DWORD (WINAPI* XInputGetState_t)(DWORD dwUserIndex, XINPUT_STATE* pState);
extern XInputGetState_t _XInputGetState;

typedef DWORD (WINAPI* XInputSetState_t)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
extern XInputSetState_t _XInputSetState;
