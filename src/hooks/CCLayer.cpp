#include "CCLayer.hpp"
#include "../utils.hpp"
#include "../globals.hpp"

void HookedCCLayer::onEnter() {
    CCLayer::onEnter();

    if (getParent() == cocos2d::CCScene::get() || cl::utils::shouldTreatParentAsImportant(this)) {
        geode::log::debug("{}", getChildrenCount());
        
        // clear editing text or adjusting slider
        if (g_button) {
            if (cl::utils::getFocusableNodeType(g_button) == FocusableNodeType::TextInput) g_isEditingText = false;
            if (cl::utils::buttonIsActuallySliderThumb(g_button)) g_isAdjustingSlider = false;
        }

        // clear current button so we can find a new one if this is a child of
        // an important layer
        cl::utils::clearCurrentButton();
    }
}

void HookedCCLayer::onExit() {
    for (auto button : cl::utils::gatherAllButtons(this)) {
        // clear current button if its on this layer about to be removed
        if (g_button == button) {
            cl::utils::clearCurrentButton();
        }
    }

    CCLayer::onExit();
}
