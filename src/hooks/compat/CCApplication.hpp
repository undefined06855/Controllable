#pragma once
#include <Geode/modify/CCApplication.hpp>

class $modify(HookedCCApplication, cocos2d::CCApplication) {
    void updateControllerKeys(CXBOXController* controller, int player);
};
