#include "PauseLayer.hpp"
#include "../../../ControllableManager.hpp"

void HookedPauseLayer::customSetup() {
    // set controller connected to false to remove gd icons
    auto application = cocos2d::CCApplication::get();
    if (cl::Manager::get().m_otherRemoveGDIcons) {
        application->m_pControllerHandler->m_controllerConnected = false;
        application->m_pController2Handler->m_controllerConnected = false;
        application->m_bControllerConnected = false;
    }

    PauseLayer::customSetup();
}
