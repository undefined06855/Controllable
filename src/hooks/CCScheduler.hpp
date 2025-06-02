#pragma once
#include <Geode/modify/CCScheduler.hpp>

class $modify(HookedCCScheduler, cocos2d::CCScheduler) {
    void scheduleSelector(cocos2d::SEL_SCHEDULE pfnSelector, cocos2d::CCObject *pTarget, float fInterval, bool bPaused);
};
