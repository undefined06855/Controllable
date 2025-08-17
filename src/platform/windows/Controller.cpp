#include "Controller.hpp"
#include "../../ControllableManager.hpp"
#include "../../globals.hpp"
#include "xinput.hpp"

using namespace controllable;

Controller g_controller;

Controller::Controller()
    : m_state({})
    , m_lastDirection(GamepadDirection::None)
    , m_lastGamepadButton(GamepadButton::None)
    , m_vibrationTime(0.f)
    , m_connected(false) {}

// should be called before all input processing is done
void Controller::update(float dt) {
    m_lastDirection = directionPressed();
    m_lastGamepadButton = gamepadButtonPressed();

    XINPUT_STATE state = {};
    auto ret = _XInputGetState(0, &state); // TODO: multiple players?

    if (ret == ERROR_DEVICE_NOT_CONNECTED) {
        m_connected = false;
        g_isUsingController = false;
        return;
    }

    if (!m_connected) {
        // just connected controller
        g_isUsingController = true;
    }

    m_connected = true;

    m_state.m_buttonA = state.Gamepad.wButtons & XINPUT_GAMEPAD_A;
    m_state.m_buttonB = state.Gamepad.wButtons & XINPUT_GAMEPAD_B;
    m_state.m_buttonX = state.Gamepad.wButtons & XINPUT_GAMEPAD_X;
    m_state.m_buttonY = state.Gamepad.wButtons & XINPUT_GAMEPAD_Y;

    m_state.m_buttonStart = state.Gamepad.wButtons & XINPUT_GAMEPAD_START;
    m_state.m_buttonSelect = state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;

    m_state.m_buttonL = state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
    m_state.m_buttonR = state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

    // 0 to 255
    byte triggerDeadzone = 255 * cl::Manager::get().m_controllerTriggerDeadzone;
    m_state.m_buttonZL = state.Gamepad.bLeftTrigger > triggerDeadzone;
    m_state.m_buttonZR = state.Gamepad.bRightTrigger > triggerDeadzone;

    m_state.m_buttonUp = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
    m_state.m_buttonDown = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
    m_state.m_buttonLeft = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
    m_state.m_buttonRight = state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

    m_state.m_joyLeft = state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
    m_state.m_joyRight = state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;

    m_state.m_joyLeftX = state.Gamepad.sThumbLX / 32767.f;
    m_state.m_joyLeftY = state.Gamepad.sThumbLY / 32767.f;
    m_state.m_joyRightX = state.Gamepad.sThumbRX / 32767.f;
    m_state.m_joyRightY = state.Gamepad.sThumbRY / 32767.f;

    m_vibrationTime -= dt;
    if (m_vibrationTime < 0.f) {
        m_vibrationTime = 0.f;
        XINPUT_VIBRATION vibration = {
            .wLeftMotorSpeed = 0,
            .wRightMotorSpeed = 0,
        };

        _XInputSetState(0, &vibration);
    }
}

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

void Controller::vibrate(float duration, float left, float right) {
    m_vibrationTime = duration;

    XINPUT_VIBRATION vibration = {
        .wLeftMotorSpeed = static_cast<unsigned short>(left * 65535),
        .wRightMotorSpeed = static_cast<unsigned short>(right * 65535),
    };

    _XInputSetState(0, &vibration);
}
