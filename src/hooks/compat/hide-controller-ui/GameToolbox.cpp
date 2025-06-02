#include "GameToolbox.hpp"
#include "../../../ControllableManager.hpp"

void HookedGameToolbox::addBackButton(cocos2d::CCLayer* layer, cocos2d::CCMenuItem* button) {
    if (cl::Manager::get().m_otherRemoveGDIcons) {
        return;
    }

    return GameToolbox::addBackButton(layer, button);
}
