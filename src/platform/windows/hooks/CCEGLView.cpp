#include "CCEGLView.hpp"
#include "../../../globals.hpp"

void HookedCCEGLView::onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y) {
    g_isUsingController = false;
    CCEGLView::onGLFWMouseMoveCallBack(window, x, y);
}

void HookedCCEGLView::onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int mods) {
    g_isUsingController = false;
    CCEGLView::onGLFWMouseCallBack(window, button, action, mods);
}

void HookedCCEGLView::onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    g_isUsingController = false;
    CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);
}
