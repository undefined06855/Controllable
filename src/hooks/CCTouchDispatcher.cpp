#include "CCTouchDispatcher.hpp"
#include "../globals.hpp"

// for mouse down / touch events cross-platform only - mouse move is handled by
// platform specific hooks

void HookedCCTouchDispatcher::touches(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent, unsigned int uIndex) {
    CCTouchDispatcher::touches(pTouches, pEvent, uIndex);
    g_isUsingController = false;
}
