#include "../../Controller.hpp"
#include "../../ControllableManager.hpp"
#include "../../globals.hpp"
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#include "jni.h"

Controller g_controller;

ControllerState g_callbackControllerState;

void JNI_GeodeUtils_setControllerState(JNIEnv* env, jobject, jint index, jobject gamepad);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
Controller::Controller()
    : m_state({})
    , m_lastDirection(GamepadDirection::None)
    , m_lastGamepadButton(GamepadButton::None)
    , m_vibrationTime(0.f)
    , m_connected(false) {

    // taken from cbf
    static const JNINativeMethod methods[] = {
        {
            "setControllerState",
            "(ILcom/geode/launcher/GeometryDashActivity$Gamepad;)V",
            reinterpret_cast<void*>(&JNI_GeodeUtils_setControllerState)
        }
    };

    // register native function for callback
    // this is called on the nearest frame whenever a controller input is detected

    JNIEnv* env;
    auto ret = cocos2d::JniHelper::getJavaVM()->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

    if (ret != JNI_OK) {
        geode::log::warn("Failed to get java env!");
        return;
    }

    auto geodeUtils = cocos2d::JniHelper::getClassID("com/geode/launcher/utils/GeodeUtils");
    ret = env->RegisterNatives(geodeUtils, methods, 1);
    if (ret) {
        geode::log::warn("Failed to set native function for setControllerState!");
        cl::Manager::get().m_androidLauncherOutdated = true;
        return;
    }

    // and enable callbacks

    auto info = cocos2d::JniMethodInfo();
    if (!cocos2d::JniHelper::getStaticMethodInfo(info, "com/geode/launcher/utils/GeodeUtils", "enableControllerCallbacks", "()V")) {
        geode::log::warn("Failed to get JNI method info for enableControllerCallbacks!");
        return;
    }

    info.env->CallStaticVoidMethod(info.classID, info.methodID);
    info.env->DeleteLocalRef(info.classID);
}

#pragma clang diagnostic pop

// should be called before all input processing is done
void Controller::update(float dt) {
    m_lastDirection = directionPressed();
    m_lastGamepadButton = gamepadButtonPressed();

    // sync callback controller state with controller state
    m_state = g_callbackControllerState;

    m_vibrationTime -= dt;
    if (m_vibrationTime < 0.f) {
        m_vibrationTime = 0.f;
    }
}

#define JAVA_GAMEPAD_BOOL_FIELD(field) (bool)env->GetBooleanField(gamepad, env->GetFieldID(gamepadClass, field, "Z"))
#define JAVA_GAMEPAD_FLOAT_FIELD(field) (float)env->GetFloatField(gamepad, env->GetFieldID(gamepadClass, field, "F"))

void JNI_GeodeUtils_setControllerState(JNIEnv* env, jobject, jint index, jobject gamepad) {
    geode::log::info("controller {} update", index);
    if (index != 0) return;

    if (!gamepad) {
        g_controller.m_connected = false;
        g_isUsingController = false;
        return;
    }

    if (!g_controller.m_connected) {
        // just connected controller
        g_isUsingController = true;
    }

    g_controller.m_connected = true;
    g_isUsingController = true;

    auto gamepadClass = cocos2d::JniHelper::getClassID("com/geode/launcher/GeometryDashActivity$Gamepad");

    g_callbackControllerState.m_buttonA = JAVA_GAMEPAD_BOOL_FIELD("mButtonA");
    g_callbackControllerState.m_buttonB = JAVA_GAMEPAD_BOOL_FIELD("mButtonB");
    g_callbackControllerState.m_buttonX = JAVA_GAMEPAD_BOOL_FIELD("mButtonX");
    g_callbackControllerState.m_buttonY = JAVA_GAMEPAD_BOOL_FIELD("mButtonY");
    g_callbackControllerState.m_buttonStart = JAVA_GAMEPAD_BOOL_FIELD("mButtonStart");
    g_callbackControllerState.m_buttonSelect = JAVA_GAMEPAD_BOOL_FIELD("mButtonSelect");
    g_callbackControllerState.m_buttonL = JAVA_GAMEPAD_BOOL_FIELD("mButtonL");
    g_callbackControllerState.m_buttonR = JAVA_GAMEPAD_BOOL_FIELD("mButtonR");
    g_callbackControllerState.m_buttonUp = JAVA_GAMEPAD_BOOL_FIELD("mButtonUp");
    g_callbackControllerState.m_buttonDown = JAVA_GAMEPAD_BOOL_FIELD("mButtonDown");
    g_callbackControllerState.m_buttonLeft = JAVA_GAMEPAD_BOOL_FIELD("mButtonLeft");
    g_callbackControllerState.m_buttonRight = JAVA_GAMEPAD_BOOL_FIELD("mButtonRight");
    g_callbackControllerState.m_joyLeft = JAVA_GAMEPAD_BOOL_FIELD("mButtonJoyLeft");
    g_callbackControllerState.m_joyRight = JAVA_GAMEPAD_BOOL_FIELD("mButtonJoyRight");
    
    auto deadzone = cl::Manager::get().m_controllerTriggerDeadzone;
    g_callbackControllerState.m_buttonZL = JAVA_GAMEPAD_FLOAT_FIELD("mTriggerZL") > deadzone;
    g_callbackControllerState.m_buttonZR = JAVA_GAMEPAD_FLOAT_FIELD("mTriggerZR") > deadzone;

    g_callbackControllerState.m_joyLeftX = JAVA_GAMEPAD_FLOAT_FIELD("mJoyLeftX");
    g_callbackControllerState.m_joyLeftY = JAVA_GAMEPAD_FLOAT_FIELD("mJoyLeftY");
    g_callbackControllerState.m_joyRightX = JAVA_GAMEPAD_FLOAT_FIELD("mJoyRightX");
    g_callbackControllerState.m_joyRightY = JAVA_GAMEPAD_FLOAT_FIELD("mJoyRightY");
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
