#pragma once
#include <Geode/modify/CCMenuItem.hpp>

class $modify(HookedCCMenuItem, cocos2d::CCMenuItem) {
    bool initWithTarget(cocos2d::CCObject* rec, cocos2d::SEL_MenuHandler selector);
};
