#pragma once
#include <Geode/modify/GJChestSprite.hpp>

class $modify(HookedGJChestSprite, GJChestSprite) {
    void switchToState(ChestSpriteState state, bool initial);
};
