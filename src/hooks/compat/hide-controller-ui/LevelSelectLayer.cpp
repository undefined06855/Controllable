#include "LevelSelectLayer.hpp"
#include "../../../ControllableManager.hpp"

bool HookedLevelSelectLayer::init(int page) {
    // set controller connected to false to remove gd icons
    auto application = cocos2d::CCApplication::get();
    if (cl::Manager::get().m_otherRemoveGDIcons) {
        application->m_pControllerHandler->m_controllerConnected = false;
        application->m_pController2Handler->m_controllerConnected = false;
        application->m_bControllerConnected = false;
    }

    return LevelSelectLayer::init(page);
}
