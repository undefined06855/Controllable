#include "GJItemIcon.hpp"

void HookedGJItemIcon::changeToLockedState(float p0) {
    GJItemIcon::changeToLockedState(p0);

    setUserObject("force-shadowed-selection"_spr, cocos2d::CCBool::create(true));
}
