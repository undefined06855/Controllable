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

void Controller::vibrate(float duration, float left, float right) {
    m_vibrationTime = duration;

    XINPUT_VIBRATION vibration = {
        .wLeftMotorSpeed = static_cast<unsigned short>(left * 65535),
        .wRightMotorSpeed = static_cast<unsigned short>(right * 65535),
    };

    _XInputSetState(0, &vibration);
}
