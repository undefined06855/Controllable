#pragma once
#include <Geode/modify/LevelBrowserLayer.hpp>

class $modify(HookedLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* searchObject);
};
