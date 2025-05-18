#pragma once
#include <Geode/modify/CCLayer.hpp>

class $modify(HookedCCLayer, cocos2d::CCLayer) {
    void onEnter();
    void onExit();
};
