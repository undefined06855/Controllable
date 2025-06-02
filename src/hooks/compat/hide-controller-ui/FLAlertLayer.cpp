#include "FLAlertLayer.hpp"
#include "../../../ControllableManager.hpp"

bool HookedFLAlertLayer::init(FLAlertLayerProtocol* p0, char const* p1, gd::string p2, char const* p3, char const* p4, float p5, bool p6, float p7, float p8) {
    // set controller connected to false to remove gd icons
    auto application = cocos2d::CCApplication::get();
    if (cl::Manager::get().m_otherRemoveGDIcons) {
        application->m_pControllerHandler->m_controllerConnected = false;
        application->m_pController2Handler->m_controllerConnected = false;
        application->m_bControllerConnected = false;
    }

    return FLAlertLayer::init(p0, p1, p2, p3, p4, p5, p6, p7, p8);
}
