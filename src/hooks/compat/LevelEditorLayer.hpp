#pragma once
#include <Geode/modify/LevelEditorLayer.hpp>

class $modify(HookedLevelEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool p1);
};
