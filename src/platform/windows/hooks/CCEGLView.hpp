#pragma once
#include <Geode/modify/CCEGLView.hpp>

class $modify(HookedCCEGLView, cocos2d::CCEGLView) {
    void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y);
    void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
