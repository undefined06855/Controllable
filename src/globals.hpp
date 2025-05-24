#pragma once
#include <RenderTexture.hpp>

extern cocos2d::CCDrawNode* g_overlay;

extern geode::Ref<cocos2d::CCNode> g_button;
extern std::shared_ptr<RenderTexture::Sprite> g_buttonOverlay;

extern float g_scrollNextFrame;
extern float g_scrollTime;

extern float g_sliderNextFrame;
extern bool g_isAdjustingSlider;

extern bool g_isEditingText;
extern float g_editingTextRepeatTimer;

extern bool g_isUsingController;
