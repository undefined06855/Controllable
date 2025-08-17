#include "../include/Controllable.hpp"
#include "ControllableManager.hpp"
#include "utils.hpp"

using namespace controllable;

bool controllable::isUsingController() {
    return cl::utils::isUsingController();
}

// index is for potentially multiple controllers
Controller& controllable::getController(int index) {
    return g_controller;
}

#define MANAGER_BUTTON_CASE(action, button) \
    case controllable::ControllerAction::action: \
        if (down) { \
            manager.pressButton(GamepadButton::button); \
        } else { \
            manager.depressButton(GamepadButton::button); \
        } \
        break;

#define MANAGER_JOY_CASE(action, direction) \
    case controllable::ControllerAction::action: \
        if (down) { \
            manager.pressDirection(GamepadDirection::direction); \
        } else { \
            manager.depressDirection(GamepadDirection::direction); \
        } \
        break;

void controllable::runControllerAction(ControllerAction action, bool down) {
    auto& manager = cl::Manager::get();

    switch (action) {
        MANAGER_BUTTON_CASE(A, A)
        MANAGER_BUTTON_CASE(B, B)
        MANAGER_BUTTON_CASE(X, X)
        MANAGER_BUTTON_CASE(Y, Y)
        MANAGER_BUTTON_CASE(Start, Start)
        MANAGER_BUTTON_CASE(Select, Select)
        MANAGER_BUTTON_CASE(L, L)
        MANAGER_BUTTON_CASE(R, R)
        MANAGER_BUTTON_CASE(ZL, ZL)
        MANAGER_BUTTON_CASE(ZR, ZR)
        MANAGER_BUTTON_CASE(JoyLeftPress, JoyLeft)
        MANAGER_BUTTON_CASE(JoyRightPress, JoyRight)
        // we could treat d-pad as either a button or joystick (fallback
        // works either way) but to make it actually navigate we should
        // treat it as joystick (it doesnt really do anything except
        // fallback as a button)
        MANAGER_JOY_CASE(DPadUp, Up)
        MANAGER_JOY_CASE(DPadDown, Down)
        MANAGER_JOY_CASE(DPadLeft, Left)
        MANAGER_JOY_CASE(DPadRight, Right)
        MANAGER_JOY_CASE(LeftJoyUp, JoyUp)
        MANAGER_JOY_CASE(LeftJoyDown, JoyDown)
        MANAGER_JOY_CASE(LeftJoyLeft, JoyLeft)
        MANAGER_JOY_CASE(LeftJoyRight, JoyRight)
        MANAGER_JOY_CASE(RightJoyUp, SecondaryJoyUp)
        MANAGER_JOY_CASE(RightJoyDown, SecondaryJoyDown)
        MANAGER_JOY_CASE(RightJoyLeft, SecondaryJoyLeft)
        MANAGER_JOY_CASE(RightJoyRight, SecondaryJoyRight)
        MANAGER_BUTTON_CASE(None, None)
    }
}

#undef MANAGER_BUTTON_CASE
#undef MANAGER_JOY_CASE

void controllable::runNavigationAction(NavigationAction action) {
    auto& manager = cl::Manager::get();

    switch (action) {
        case NavigationAction::None:
            manager.pressDirection(GamepadDirection::None, /* (disallow fallback) */ false);
            break;

        case NavigationAction::Up:
            manager.pressDirection(GamepadDirection::Up, /* (disallow fallback) */ false);
            break;

        case NavigationAction::Down:
            manager.pressDirection(GamepadDirection::Down, /* (disallow fallback) */ false);
            break;

        case NavigationAction::Left:
            manager.pressDirection(GamepadDirection::Left, /* (disallow fallback) */ false);
            break;

        case NavigationAction::Right:
            manager.pressDirection(GamepadDirection::Right, /* (disallow fallback) */ false);
            break;

        case NavigationAction::ConfirmDown:
            manager.pressButton(GamepadButton::A, /* (disallow fallback) */ false);
            break;

        case NavigationAction::ConfirmUp:
            manager.depressButton(GamepadButton::A, /* (disallow fallback) */ false);
            break;
        
        case NavigationAction::Return:
            manager.pressButton(GamepadButton::B, /* (disallow fallback) */ false);
            break;
    }
}
