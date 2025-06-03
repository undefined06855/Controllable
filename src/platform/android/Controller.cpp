#include "../../Controller.hpp"
#include "../../ControllableManager.hpp"
#include "../../globals.hpp"
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#include <jni.h>

Controller g_controller;

Controller::Controller()
    : m_state({})
    , m_lastDirection(GamepadDirection::None)
    , m_lastGamepadButton(GamepadButton::None)
    , m_vibrationTime(0.f)
    , m_connected(false) {}

#define JAVA_GAMEPAD_BOOL_FIELD(field) (bool)info.env->GetBooleanField(object, info.env->GetFieldID(gamepadClass, field, "Z"))
#define JAVA_GAMEPAD_FLOAT_FIELD(field) (float)info.env->GetFloatField(object, info.env->GetFieldID(gamepadClass, field, "F"))

// should be called before all input processing is done
void Controller::update(float dt) {
    m_lastDirection = directionPressed();
    m_lastGamepadButton = gamepadButtonPressed();

    auto info = cocos2d::JniMethodInfo();
    if (!cocos2d::JniHelper::getStaticMethodInfo(info, "com/geode/launcher/utils/GeodeUtils", "getControllerState", "(I)Lcom/geode/launcher/GeometryDashActivity$Gamepad;")) {
        geode::log::warn("Failed to get JNI method info!");
        cl::Manager::get().m_androidLauncherOutdated = true;
        return;
    }
    
    auto object = info.env->CallStaticObjectMethod(info.classID, info.methodID, 0);
    info.env->DeleteLocalRef(info.classID);

    if (!object) {
        m_connected = false;
        g_isUsingController = false;
        return;
    }

    if (!m_connected) {
        // just connected controller
        g_isUsingController = true;
    }

    m_connected = true;

    auto gamepadClass = cocos2d::JniHelper::getClassID("com/geode/launcher/GeometryDashActivity$Gamepad");

    m_state.m_buttonA = JAVA_GAMEPAD_BOOL_FIELD("mButtonA");
    m_state.m_buttonB = JAVA_GAMEPAD_BOOL_FIELD("mButtonB");
    m_state.m_buttonX = JAVA_GAMEPAD_BOOL_FIELD("mButtonX");
    m_state.m_buttonY = JAVA_GAMEPAD_BOOL_FIELD("mButtonY");
    m_state.m_buttonStart = JAVA_GAMEPAD_BOOL_FIELD("mButtonStart");
    m_state.m_buttonSelect = JAVA_GAMEPAD_BOOL_FIELD("mButtonSelect");
    m_state.m_buttonL = JAVA_GAMEPAD_BOOL_FIELD("mButtonL");
    m_state.m_buttonR = JAVA_GAMEPAD_BOOL_FIELD("mButtonR");
    m_state.m_buttonUp = JAVA_GAMEPAD_BOOL_FIELD("mButtonUp");
    m_state.m_buttonDown = JAVA_GAMEPAD_BOOL_FIELD("mButtonDown");
    m_state.m_buttonLeft = JAVA_GAMEPAD_BOOL_FIELD("mButtonLeft");
    m_state.m_buttonRight = JAVA_GAMEPAD_BOOL_FIELD("mButtonRight");
    m_state.m_joyLeft = JAVA_GAMEPAD_BOOL_FIELD("mButtonJoyLeft");
    m_state.m_joyRight = JAVA_GAMEPAD_BOOL_FIELD("mButtonJoyRight");
    
    auto deadzone = cl::Manager::get().m_controllerTriggerDeadzone;
    m_state.m_buttonZL = JAVA_GAMEPAD_FLOAT_FIELD("mTriggerZL") > deadzone;
    m_state.m_buttonZR = JAVA_GAMEPAD_FLOAT_FIELD("mTriggerZR") > deadzone;

    m_state.m_joyLeftX = JAVA_GAMEPAD_FLOAT_FIELD("mJoyLeftX");
    m_state.m_joyLeftY = JAVA_GAMEPAD_FLOAT_FIELD("mJoyLeftY");
    m_state.m_joyRightX = JAVA_GAMEPAD_FLOAT_FIELD("mJoyRightX");
    m_state.m_joyRightY = JAVA_GAMEPAD_FLOAT_FIELD("mJoyRightY");

    m_vibrationTime -= dt;
    if (m_vibrationTime < 0.f) {
        m_vibrationTime = 0.f;
    }
}

#undef JAVA_GAMEPAD_BOOL_FIELD
#undef JAVA_GAMEPAD_FLOAT_FIELD

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

    // this doesnt actually work for my controller so its gone untested
    // i hope this does actually work for somebody out there
    
    auto info = cocos2d::JniMethodInfo();
    if (!cocos2d::JniHelper::getStaticMethodInfo(info, "com/geode/launcher/utils/GeodeUtils", "setControllerVibration", "(IJII)V")) {
        geode::log::warn("Failed to get JNI method info for vibration!");
        return;
    }

    info.env->CallStaticVoidMethod(info.classID, info.methodID, 0, (long)duration, left * 255, right * 255);
    info.env->DeleteLocalRef(info.classID);
}
