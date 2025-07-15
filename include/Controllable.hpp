#pragma once
#include <Geode/loader/Event.hpp>
// source: src/events.cpp

namespace controllable {

enum class Direction {
    None = 0,
    Up, Down, Left, Right
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

bool isUsingController();

namespace events {

class ControllerActionEvent : public geode::Event {
protected:
    ControllerAction m_action;
    bool m_down;
public:
    ControllerActionEvent(ControllerAction Action, bool down);
    ControllerAction getAction();
    bool getDown();
};

class SimpleDirectionPressEvent : public geode::Event {
protected:
    Direction m_direction;
public:
    SimpleDirectionPressEvent(Direction direction);
    Direction getDirection();
};

}

}
