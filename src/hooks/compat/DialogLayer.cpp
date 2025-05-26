#include "DialogLayer.hpp"

bool HookedDialogLayer::init(DialogObject* dialogObject, cocos2d::CCArray* p1, int bg) {
    if (!DialogLayer::init(dialogObject, p1, bg)) return false;

    setUserObject("is-focusable"_spr, cocos2d::CCBool::create(true));
    setUserObject("is-dialog-layer"_spr, cocos2d::CCBool::create(true));

    return true;
}
