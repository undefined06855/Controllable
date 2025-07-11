#pragma once
#include <RenderTexture.hpp>
#include <NodeIdentification.hpp>
#include "enums.hpp"

extern geode::Ref<cocos2d::CCNode> g_button;
extern ni::FullIdentification g_lastButton;
extern std::vector<ni::FullIdentification> g_history;

extern bool g_isAdjustingSlider;
extern bool g_isEditingText;
extern bool g_isUsingController;

extern DebugInformation g_debugInformation;

extern cocos2d::SEL_SCHEDULE g_ckCallback;
extern cocos2d::CCObject* g_ckTarget;
extern bool g_startCKCallback;
