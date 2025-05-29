#include "LoadingLayer.hpp"
#include "../globals.hpp"
#include "../ControllableManager.hpp"

bool HookedLoadingLayer::init(bool p0) {
    if (!LoadingLayer::init(p0)) return false;

    cl::Manager::get().init();

    // will most likely instantly be overridden by the cceglview hooks but eh
    g_isUsingController = PlatformToolbox::isControllerConnected();

    return true;
}
