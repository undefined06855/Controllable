#include "CCScheduler.hpp"
#include "../globals.hpp"

HookedCCScheduler::Fields::Fields()
    : m_scroll(0.f) {}

void HookedCCScheduler::update(float dt) {
    CCScheduler::update(dt);

    // TODO: scrolling seems to be reversed in some scroll layers?
    
    if (g_scrollNextFrame != 0.f) {
        auto mouseDispatcher = cocos2d::CCDirector::get()->getMouseDispatcher();
        float scrollSpeed = 25.f * std::pow(g_scrollTime, 6.f) + 330.f;
        mouseDispatcher->dispatchScrollMSG(g_scrollNextFrame * dt * scrollSpeed, 0.f);
        g_scrollTime += dt;
    }

    if (g_scrollNextFrame == 0.f) {
        g_scrollTime = 0.f;
    }
}
