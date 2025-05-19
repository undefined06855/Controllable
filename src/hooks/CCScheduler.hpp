#pragma once
#include <Geode/modify/CCScheduler.hpp>

class $modify(HookedCCScheduler, cocos2d::CCScheduler) {
    struct Fields {
        float m_scroll;
        Fields();
    };

    void update(float dt);
};
