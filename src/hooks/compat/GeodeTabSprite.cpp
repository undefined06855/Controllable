#include "GeodeTabSprite.hpp"

void HookedGeodeTabSprite::modify() {
    setUserObject("force-shadowed-selection"_spr, cocos2d::CCBool::create(true));
}
