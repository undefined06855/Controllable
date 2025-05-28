#pragma once
#include <RenderTexture.hpp>
#include "enums.hpp"

extern geode::Ref<cocos2d::CCNode> g_button;

extern float g_scrollNextFrame;
extern float g_scrollTime;

extern float g_sliderNextFrame;
extern bool g_isAdjustingSlider;

extern bool g_isEditingText;
extern float g_editingTextRepeatTimer;

extern bool g_isUsingController;

extern std::vector<HistoryButton> g_history;
