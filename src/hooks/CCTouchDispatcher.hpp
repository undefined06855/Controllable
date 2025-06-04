#pragma once
#include <Geode/modify/CCTouchDispatcher.hpp>

class $modify(HookedCCTouchDispatcher, cocos2d::CCTouchDispatcher) {
    void touches(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent, unsigned int uIndex);
};
