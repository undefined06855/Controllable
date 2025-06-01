#include "GJItemIcon.hpp"

bool HookedGJItemIcon::init(UnlockType p0, int p1, cocos2d::ccColor3B p2, cocos2d::ccColor3B p3, bool p4, bool p5, bool p6/*, cocos2d::ccColor3B p7*/) {
    if (!GJItemIcon::init(p0, p1, p2, p3, p4, p5, p6/*, p7*/)) return false;

    setUserObject("force-shadowed-selection"_spr, cocos2d::CCBool::create(true));

    return true;
}
