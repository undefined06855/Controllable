#pragma once
#include <Geode/modify/CCApplication.hpp>
#include "../utils.hpp"

class $modify(HookedCCApplication, cocos2d::CCApplication) {
    void updateControllerKeys(CXBOXController* controller, int player);

    void updateDrawNode();

    // direction
    void focusInDirection(Direction direction);
    cocos2d::CCMenuItem* attemptFindButton(Direction direction, cocos2d::CCRect rect);

    // face buttons
    void pressButton(GamepadButton button);
    void depressButton(GamepadButton button);

    // scroll (right joystick)
    void scrollScreen(float amount);
};
