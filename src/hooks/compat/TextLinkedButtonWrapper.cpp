#include "TextLinkedButtonWrapper.hpp"

void HookedTextLinkedButtonWrapper::modify() {
    setUserObject("requires-selected-before-unselected"_spr, cocos2d::CCBool::create(true));
}
