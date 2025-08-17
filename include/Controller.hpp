#pragma once
#include "enums.hpp"

#ifdef GEODE_IS_WINDOWS
    #ifdef CONTROLLABLE_EXPORTING
        #define CONTROLLABLE_DLL __declspec(dllexport)
    #else
        #define CONTROLLABLE_DLL __declspec(dllimport)
    #endif
#else
    #define CONTROLLABLE_DLL __attribute__((visibility("default")))
#endif

namespace controllable {

struct ControllerState {
    bool m_buttonA, m_buttonB, m_buttonX, m_buttonY,
         m_buttonStart, m_buttonSelect,
         m_buttonL, m_buttonR, m_buttonZL, m_buttonZR,
         m_buttonUp, m_buttonDown, m_buttonLeft, m_buttonRight,
         m_joyLeft, m_joyRight;
    float m_joyLeftX, m_joyLeftY,
          m_joyRightX, m_joyRightY;
};

// note that implementation of this is platform-specific!

class CONTROLLABLE_DLL Controller {
public:
    Controller();

    ControllerState m_state;
    GamepadDirection m_lastDirection;
    GamepadButton m_lastGamepadButton;
    float m_vibrationTime;
    bool m_connected;

    void update(float dt);

    GamepadDirection directionJustPressed();
    GamepadDirection directionJustReleased();
    GamepadButton gamepadButtonJustPressed();
    GamepadButton gamepadButtonJustReleased();

    GamepadDirection directionPressed();
    GamepadButton gamepadButtonPressed();

    cocos2d::CCPoint getLeftJoystick();
    cocos2d::CCPoint getRightJoystick();

    void vibrate(float duration, float left, float right);
};

}

extern controllable::Controller g_controller;