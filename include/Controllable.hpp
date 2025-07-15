#pragma once
#include <Geode/loader/Event.hpp>
// source: src/events.cpp

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

enum class SimpleControllerEventType {
    None = 0,
    Up, Down, Left, Right, ConfirmDown, ConfirmUp, Return
};

enum class ControllerAction {
    None = 0,
    A, B, X, Y,
    Start, Select,
    L, R, ZL, ZR,
    JoyLeftPress, JoyRightPress,
    DPadUp, DPadDown, DPadLeft, DPadRight,
    LeftJoyUp, LeftJoyDown, LeftJoyLeft, LeftJoyRight,
    RightJoyUp, RightJoyDown, RightJoyLeft, RightJoyRight
};

bool CONTROLLABLE_DLL isUsingController();

namespace events {

class CONTROLLABLE_DLL ControllerActionEvent : public geode::Event {
protected:
    ControllerAction m_action;
    bool m_down;
public:
    ControllerActionEvent(ControllerAction Action, bool down);
    ControllerAction getAction();
    bool getDown();
};

class CONTROLLABLE_DLL SimpleControllerEvent : public geode::Event {
protected:
    SimpleControllerEventType m_event;
public:
    SimpleControllerEvent(SimpleControllerEventType event);
    SimpleControllerEventType getControllerEvent();
};

}

}
