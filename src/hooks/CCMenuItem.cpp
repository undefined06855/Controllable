#include "CCMenuItem.hpp"

bool HookedCCMenuItem::initWithTarget(cocos2d::CCObject* rec, cocos2d::SEL_MenuHandler selector) {
    if (!CCMenuItem::initWithTarget(rec, selector)) return false;

    setUserObject("is-button"_spr, cocos2d::CCBool::create(true));

    return true;
}
