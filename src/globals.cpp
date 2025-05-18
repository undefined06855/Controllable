#include "globals.hpp"

cocos2d::CCDrawNode* g_overlay = nullptr;
std::vector<cocos2d::CCMenuItem*> g_cachedButtons = {};

cocos2d::CCMenuItem* g_button = nullptr;

// TODO: make g_isUsingController update
bool g_isUsingController = true;
