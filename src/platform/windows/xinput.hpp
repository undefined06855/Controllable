#pragma once

typedef DWORD (WINAPI* XInputGetState_t)(DWORD dwUserIndex, XINPUT_STATE* pState);
extern XInputGetState_t _XInputGetState;
