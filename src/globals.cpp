#include "globals.hpp"

cocos2d::CCDrawNode* g_overlay = nullptr;

// ref and not weakref because weakref's buggy and raw pointers crash sometimes
geode::Ref<cocos2d::CCNode> g_button = nullptr;

// set in scrollScreen, fetched in ccscheduler update
float g_scrollNextFrame = 0.f;
float g_scrollTime = 0.f;

// set in updateControllerKeys, fetched in ccscheduler update
float g_sliderNextFrame = 0.f;
bool g_isAdjustingSlider = false;

bool g_isEditingText = false;
float g_editingTextRepeatTimer = 0.f;

bool g_isUsingController = false;
