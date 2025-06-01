#include "GJChestSprite.hpp"

// why do these sprites have weird half transparent parts

void HookedGJChestSprite::switchToState(ChestSpriteState state, bool initial) {
    GJChestSprite::switchToState(state, initial);

    setUserObject("force-shadowed-selection"_spr, cocos2d::CCBool::create(true));
}
