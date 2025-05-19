#include "globals.hpp"

cocos2d::CCDrawNode* g_overlay = nullptr;

cocos2d::CCMenuItem* g_button = nullptr;

// set in scrollScreen, fetched in ccscheduler update
float g_scrollNextFrame = 0.f;
float g_scrollTime = 0.f;

// set in updateControllerKeys, fetched in ccscheduler update
float g_sliderNextFrame = 0.f;
bool g_isAdjustingSlider = false;

// TODO: make g_isUsingController update
bool g_isUsingController = true;
