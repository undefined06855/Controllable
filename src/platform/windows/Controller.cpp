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
    _XInputGetState(0, &state);

    m_state.buttonA = state.Gamepad.wButtons & XINPUT_GAMEPAD_A;
    m_state.buttonB = state.Gamepad.wButtons & XINPUT_GAMEPAD_B;
    m_state.buttonX = state.Gamepad.wButtons & XINPUT_GAMEPAD_X;
    m_state.buttonY = state.Gamepad.wButtons & XINPUT_GAMEPAD_Y;

    m_state.buttonStart = state.Gamepad.wButtons & XINPUT_GAMEPAD_START;
    m_state.buttonSelect = state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;

    m_state.buttonL = state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
    m_state.buttonR = state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

    static byte deadzone = 26;

    m_state.buttonZL = state.Gamepad.bLeftTrigger > deadzone;
    m_state.buttonZR = state.Gamepad.bRightTrigger > deadzone;

    m_state.joyLeftX = state.Gamepad.sThumbLX / 32767.f;
    m_state.joyLeftY = state.Gamepad.sThumbLY / 32767.f;
    m_state.joyRightX = state.Gamepad.sThumbRX / 32767.f;
    m_state.joyRightY = state.Gamepad.sThumbRY / 32767.f;
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
    if (m_state.buttonUp) return Direction::Up;
    if (m_state.buttonDown) return Direction::Down;
    if (m_state.buttonLeft) return Direction::Left;
    if (m_state.buttonRight) return Direction::Right;

    static float deadzone = .1f;

    if (m_state.joyLeftY > deadzone) return Direction::Up;
    if (m_state.joyLeftY < -deadzone) return Direction::Down;
    if (m_state.joyLeftX < -deadzone) return Direction::Left;
    if (m_state.joyLeftX > deadzone) return Direction::Right;

    return Direction::None;
}

GamepadButton Controller::gamepadButtonPressed() {
    if (m_state.buttonA) return GamepadButton::A;
    if (m_state.buttonB) return GamepadButton::B;
    if (m_state.buttonX) return GamepadButton::X;
    if (m_state.buttonY) return GamepadButton::Y;
    if (m_state.buttonStart) return GamepadButton::Start;
    if (m_state.buttonSelect) return GamepadButton::Select;
    if (m_state.buttonL) return GamepadButton::L;
    if (m_state.buttonR) return GamepadButton::R;
    if (m_state.buttonZL) return GamepadButton::L;
    if (m_state.buttonZR) return GamepadButton::R;

    if (m_state.buttonUp) return GamepadButton::Up;
    if (m_state.buttonDown) return GamepadButton::Down;
    if (m_state.buttonLeft) return GamepadButton::Left;
    if (m_state.buttonRight) return GamepadButton::Right;

    return GamepadButton::None;
}

float Controller::getRightJoyY() {
    if (m_state.joyRightY < .1f && m_state.joyRightY > -.1f) return 0.f; // deadzone
    
    return m_state.joyRightY;
}
