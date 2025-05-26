#include "../../Controller.hpp"
#include "xinput.hpp"

Controller g_controller;

Controller::Controller()
    : m_state({})
    , m_lastDirection(Direction::None)
    , m_lastGamepadButton(GamepadButton::None) {}

// should be called once all input processing this frame is done
void Controller::update() {
    m_lastDirection = directionPressed();
    m_lastGamepadButton = gamepadButtonPressed();

    XINPUT_STATE state;
    _XInputGetState(0, &state); // TODO: multiple players?

    m_state.m_buttonA = state.Gamepad.wButtons & XINPUT_GAMEPAD_A;
    m_state.m_buttonB = state.Gamepad.wButtons & XINPUT_GAMEPAD_B;
    m_state.m_buttonX = state.Gamepad.wButtons & XINPUT_GAMEPAD_X;
    m_state.m_buttonY = state.Gamepad.wButtons & XINPUT_GAMEPAD_Y;

    m_state.m_buttonStart = state.Gamepad.wButtons & XINPUT_GAMEPAD_START;
    m_state.m_buttonSelect = state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;

    m_state.m_buttonL = state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
    m_state.m_buttonR = state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

    m_state.m_buttonUp = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
    m_state.m_buttonDown = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
    m_state.m_buttonLeft = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
    m_state.m_buttonRight = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

    // add setting for trigger deadzone
    static byte triggerDeadzone = 26;
    m_state.m_buttonZL = state.Gamepad.bLeftTrigger > triggerDeadzone;
    m_state.m_buttonZR = state.Gamepad.bRightTrigger > triggerDeadzone;

    m_state.m_joyLeftX = state.Gamepad.sThumbLX / 32767.f;
    m_state.m_joyLeftY = state.Gamepad.sThumbLY / 32767.f;
    m_state.m_joyRightX = state.Gamepad.sThumbRX / 32767.f;
    m_state.m_joyRightY = state.Gamepad.sThumbRY / 32767.f;
}

Direction Controller::directionJustPressed() {
    if (m_lastDirection != directionPressed()) return directionPressed();
    return Direction::None;
}

Direction Controller::directionJustReleased() {
    if (m_lastDirection != directionPressed()) return m_lastDirection;
    return Direction::None;
}

GamepadButton Controller::gamepadButtonJustPressed() {
    if (m_lastGamepadButton != gamepadButtonPressed()) return gamepadButtonPressed();
    return GamepadButton::None;
}

GamepadButton Controller::gamepadButtonJustReleased() {
    if (m_lastGamepadButton != gamepadButtonPressed()) return m_lastGamepadButton;
    return GamepadButton::None;
}


Direction Controller::directionPressed() {
    // d-pad
    if (m_state.m_buttonUp) return Direction::Up;
    if (m_state.m_buttonDown) return Direction::Down;
    if (m_state.m_buttonLeft) return Direction::Left;
    if (m_state.m_buttonRight) return Direction::Right;

    // add setting for stick deadzone
    static float stickActivationDeadzone = .4f;

    // joystick
    if (m_state.m_joyLeftY > stickActivationDeadzone) return Direction::Up;
    if (m_state.m_joyLeftY < -stickActivationDeadzone) return Direction::Down;
    if (m_state.m_joyLeftX < -stickActivationDeadzone) return Direction::Left;
    if (m_state.m_joyLeftX > stickActivationDeadzone) return Direction::Right;

    return Direction::None;
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

    return GamepadButton::None;
}

cocos2d::CCPoint Controller::getLeftJoystick() {
    return { m_state.m_joyLeftX, m_state.m_joyLeftY };
}

cocos2d::CCPoint Controller::getRightJoystick() {
    return { m_state.m_joyRightX, m_state.m_joyRightY };
}
