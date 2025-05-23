#pragma once
#include <Geode/modify/LoadingLayer.hpp>

extern const GLchar* g_newShaderFragment;
extern const GLchar* g_newShaderVertex;

class $modify(HookedLoadingLayer, LoadingLayer) {
    bool init(bool p0);
};
