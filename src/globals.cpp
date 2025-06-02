#include "globals.hpp"

// ref and not weakref because weakref's buggy and raw pointers crash sometimes
geode::Ref<cocos2d::CCNode> g_button = nullptr;

bool g_isAdjustingSlider = false;
bool g_isEditingText = false;
bool g_isUsingController = false;

DebugInformation g_debugInformation = {};

cocos2d::SEL_SCHEDULE g_ckCallback = nullptr;
cocos2d::CCObject* g_ckTarget = nullptr;
