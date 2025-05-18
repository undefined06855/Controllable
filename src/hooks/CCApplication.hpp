#pragma once
#include <Geode/modify/CCApplication.hpp>
#include "../utils.hpp"

void HookedCCApplication_updateControllerKeys(cocos2d::CCApplication* self, CXBOXController* controller, int player);
void HookedCCApplication_updateDrawNode(cocos2d::CCApplication* self);

// direction
void HookedCCApplication_focusInDirection(cocos2d::CCApplication* self, Direction direction);
cocos2d::CCMenuItem* HookedCCApplication_attemptFindButton(cocos2d::CCApplication* self, Direction direction, cocos2d::CCRect rect);

// face buttons
void HookedCCApplication_pressButton(cocos2d::CCApplication* self, GamepadButton button);
void HookedCCApplication_depressButton(cocos2d::CCApplication* self, GamepadButton button);

$execute {
    // TODO: figure out why $modify doesnt work
    (void)geode::Mod::get()->hook(
        (void*)(geode::base::getCocos() + 0x732f0),
        &HookedCCApplication_updateControllerKeys,
        "cocos2d::CCApplication::updateControllerKeys"
    );
}
