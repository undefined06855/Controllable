#include "CCScheduler.hpp"
#include "../globals.hpp"
#include <alphalaneous.alphas_geode_utils/include/Utils.h>

void HookedCCScheduler::scheduleSelector(cocos2d::SEL_SCHEDULE pfnSelector, cocos2d::CCObject* pTarget, float fInterval, bool bPaused) {
    if (AlphaUtils::Cocos::getClassName(pTarget) == "ControllerChecker") {
        g_ckTarget = pTarget;
        g_ckCallback = pfnSelector;
        return;
    }

    CCScheduler::scheduleSelector(pfnSelector, pTarget, fInterval, bPaused);
}
