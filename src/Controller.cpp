#include "Controller.hpp"
#include "ControllableManager.hpp"

using namespace controllable;

GamepadDirection Controller::directionJustPressed() {
    if (m_lastDirection != directionPressed()) return directionPressed();
    return GamepadDirection::None;
}

GamepadDirection Controller::directionJustReleased() {
    if (m_lastDirection != directionPressed()) return m_lastDirection;
    return GamepadDirection::None;
}

GamepadButton Controller::gamepadButtonJustPressed() {
    if (m_lastGamepadButton != gamepadButtonPressed()) return gamepadButtonPressed();
    return GamepadButton::None;
}

GamepadButton Controller::gamepadButtonJustReleased() {
    if (m_lastGamepadButton != gamepadButtonPressed()) return m_lastGamepadButton;
    return GamepadButton::None;
}

GamepadDirection Controller::directionPressed() {
    // d-pad
    if (m_state.m_buttonUp) return GamepadDirection::Up;
    if (m_state.m_buttonDown) return GamepadDirection::Down;
    if (m_state.m_buttonLeft) return GamepadDirection::Left;
    if (m_state.m_buttonRight) return GamepadDirection::Right;

    // 0 to 1
    float deadzone = cl::Manager::get().m_controllerJoystickDeadzone;

    // joystick
    if (m_state.m_joyLeftY > deadzone) return GamepadDirection::JoyUp;
    if (m_state.m_joyLeftY < -deadzone) return GamepadDirection::JoyDown;
    if (m_state.m_joyLeftX < -deadzone) return GamepadDirection::JoyLeft;
    if (m_state.m_joyLeftX > deadzone) return GamepadDirection::JoyRight;

    if (m_state.m_joyRightY > deadzone) return GamepadDirection::SecondaryJoyUp;
    if (m_state.m_joyRightY < -deadzone) return GamepadDirection::SecondaryJoyDown;
    if (m_state.m_joyRightX < -deadzone) return GamepadDirection::SecondaryJoyLeft;
    if (m_state.m_joyRightX > deadzone) return GamepadDirection::SecondaryJoyRight;

    return GamepadDirection::None;
}

GamepadButton Controller::gamepadButtonPressed() {
    if (m_state.m_buttonA) return GamepadButton::A;
    if (m_state.m_buttonB) return GamepadButton::B;
    if (m_state.m_buttonX) return GamepadButton::X;
    if (m_state.m_buttonY) return GamepadButton::Y;
    if (m_state.m_buttonStart) return GamepadButton::Start;
    if (m_state.m_buttonSelect) return GamepadButton::Select;
    if (m_state.m_buttonL) return GamepadButton::L;
    if (m_state.m_buttonR) return GamepadButton::R;
    if (m_state.m_buttonZL) return GamepadButton::ZL;
    if (m_state.m_buttonZR) return GamepadButton::ZR;
    if (m_state.m_buttonUp) return GamepadButton::Up;
    if (m_state.m_buttonDown) return GamepadButton::Down;
    if (m_state.m_buttonLeft) return GamepadButton::Left;
    if (m_state.m_buttonRight) return GamepadButton::Right;
    if (m_state.m_joyLeft) return GamepadButton::JoyLeft;
    if (m_state.m_joyRight) return GamepadButton::JoyRight;

    return GamepadButton::None;
}

cocos2d::CCPoint Controller::getLeftJoystick() {
    return { m_state.m_joyLeftX, m_state.m_joyLeftY };
}

cocos2d::CCPoint Controller::getRightJoystick() {
    return { m_state.m_joyRightX, m_state.m_joyRightY };
}
