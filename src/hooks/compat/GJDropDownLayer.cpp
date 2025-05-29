#include "GJDropDownLayer.hpp"

bool HookedGJDropDownLayer::init(const char* title, float p1, bool p2) {
    if (!GJDropDownLayer::init(title, p1, p2)) return false;

    setUserObject("skip-offscreen-checks"_spr, cocos2d::CCBool::create(true));

    return true;
}
