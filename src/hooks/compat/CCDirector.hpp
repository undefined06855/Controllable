#pragma once
#include <Geode/modify/CCDirector.hpp>

class $modify(HookedCCDirector, cocos2d::CCDirector) {
    void setNotificationNode(cocos2d::CCNode* node);
};
