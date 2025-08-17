#pragma once
#include <Geode/loader/Event.hpp>
#include "Controller.hpp"

namespace controllable {

enum class NavigationAction {
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
void CONTROLLABLE_DLL runNavigationAction(NavigationAction action);
void CONTROLLABLE_DLL runControllerAction(ControllerAction action, bool down);
CONTROLLABLE_DLL Controller& getController(int index = 0);

}
