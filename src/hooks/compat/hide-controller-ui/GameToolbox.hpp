#pragma once
#include <Geode/modify/GameToolbox.hpp>

class $modify(HookedGameToolbox, GameToolbox) {
    static void addBackButton(cocos2d::CCLayer* layer, cocos2d::CCMenuItem* button);
};
