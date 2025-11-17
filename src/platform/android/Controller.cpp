#include "Controller.hpp"
#include "../../ControllableManager.hpp"
#include "../../globals.hpp"
#include <Geode/utils/AndroidEvent.hpp>

using namespace controllable;

Controller g_controller;

ControllerState g_callbackControllerState;

Controller::Controller()
    : m_state({})
    , m_lastDirection(GamepadDirection::None)
    , m_lastGamepadButton(GamepadButton::None)
    , m_vibrationTime(0.f)
    , m_connected(false) {

    // remember for device id
    // https://discord.com/channels/911701438269386882/1248524007859290205/1420442689131905084

    
    if (!geode::utils::getLauncherVersion()) {
        // yo launcher out of date
        cl::Manager::get().m_androidLauncherOutdated = true;
        return;
    }
    
    new geode::EventListener<geode::EventFilter<geode::AndroidInputDeviceInfoEvent>>([this](geode::AndroidInputDeviceInfoEvent* event) {
        return geode::ListenerResult::Propagate;
    });
        
    new geode::EventListener<geode::EventFilter<geode::AndroidInputDeviceEvent>>([this](geode::AndroidInputDeviceEvent* event) {
        m_connected = event->status() == geode::AndroidInputDeviceEvent::Status::Removed;
        
        return geode::ListenerResult::Propagate;
    });

    new geode::EventListener<geode::EventFilter<geode::AndroidInputJoystickEvent>>([this](geode::AndroidInputJoystickEvent* event) {

        return geode::ListenerResult::Propagate;
    });

    
    new geode::EventListener<geode::EventFilter<geode::AndroidInputTimestampEvent>>([this](geode::AndroidInputTimestampEvent* event) {
        
        return geode::ListenerResult::Propagate;
    });

    
    g_controller.m_connected = true;
    g_isUsingController = true;

    auto gamepadClass = cocos2d::JniHelper::getClassID("com/geode/launcher/utils/GeodeUtils$Gamepad");

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

void () {
    if (index != 0) return;

}

void JNI_GeodeUtils_setControllerConnected(JNIEnv* env, jobject, jint index, jboolean connected) {
    if (index != 0) return;

    g_controller.m_connected = connected;
    g_isUsingController = connected;
}

#undef JAVA_GAMEPAD_BOOL_FIELD
#undef JAVA_GAMEPAD_FLOAT_FIELD

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
