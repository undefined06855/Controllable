#include "Controller.hpp"
#include "xinput.hpp"

Controller g_controller;

Controller::Controller()
    : m_state(0)
    , m_lastDirection(Direction::None)
    , m_lastGamepadButton(GamepadButton::None) {}

// should be called once all input processing this frame is done
void Controller::update() {
    m_lastDirection = directionPressed();
    m_lastGamepadButton = gamepadButtonPressed();

    _XInputGetState(0, &m_state);
}

Direction Controller::directionJustPressed() {
    if (m_lastDirection != directionPressed()) return directionPressed();
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
    auto gamepad = m_state.Gamepad;

    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) return Direction::Up;
    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) return Direction::Down;
    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) return Direction::Left;
    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) return Direction::Right;

    static short deadzone = 6500;

    if (gamepad.sThumbLY > deadzone) return Direction::Up;
    if (gamepad.sThumbLY < -deadzone) return Direction::Down;
    if (gamepad.sThumbLX < -deadzone) return Direction::Left;
    if (gamepad.sThumbLX > deadzone) return Direction::Right;

    return Direction::None;
}

GamepadButton Controller::gamepadButtonPressed() {
    auto gamepad = m_state.Gamepad;

    if (gamepad.wButtons & XINPUT_GAMEPAD_A) return GamepadButton::A;
    if (gamepad.wButtons & XINPUT_GAMEPAD_B) return GamepadButton::B;
    if (gamepad.wButtons & XINPUT_GAMEPAD_X) return GamepadButton::X;
    if (gamepad.wButtons & XINPUT_GAMEPAD_Y) return GamepadButton::Y;
    if (gamepad.wButtons & XINPUT_GAMEPAD_START) return GamepadButton::Start;
    if (gamepad.wButtons & XINPUT_GAMEPAD_BACK) return GamepadButton::Select;
    if (gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) return GamepadButton::L;
    if (gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) return GamepadButton::R;

    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) return GamepadButton::Up;
    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) return GamepadButton::Down;
    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) return GamepadButton::Left;
    if (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) return GamepadButton::Right;


    static byte deadzone = 26;
    if (gamepad.bLeftTrigger > deadzone) return GamepadButton::ZL;
    if (gamepad.bRightTrigger > deadzone) return GamepadButton::ZR;

    return GamepadButton::None;
}

