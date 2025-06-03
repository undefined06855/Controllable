#pragma once
#include <Geode/modify/MenuLayer.hpp>

class $modify(HookedMenuLayer, MenuLayer) {
    bool init();

    void showShaderWarning();
    void showAndroidWarning();
    void adjustNGButton();
};
