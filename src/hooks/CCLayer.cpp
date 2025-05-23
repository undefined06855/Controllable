#include "CCLayer.hpp"
#include "../utils.hpp"
#include "../globals.hpp"

// clear g_button to force find a new button on this layer if its the topmost one 
void HookedCCLayer::onEnter() {
    CCLayer::onEnter();

    geode::log::debug("{}", this);

    if (getParent() == cocos2d::CCScene::get() || cl::utils::shouldTreatParentAsImportant(this)) {
        if (g_button) {
            if (cl::utils::getFocusableNodeType(g_button) == FocusableNodeType::TextInput) g_isEditingText = false;
            if (cl::utils::buttonIsActuallySliderThumb(g_button)) g_isAdjustingSlider = false;
        }

        // find a new button
        cl::utils::clearCurrentButton();
    }
}

// clear g_button if we have to
void HookedCCLayer::onExit() {
    for (auto button : cl::utils::gatherAllButtons(this)) {
        if (g_button == button) {
            geode::log::info("cleared current btn");
            cl::utils::clearCurrentButton();
        }
    }

    // TODO: preserve last layer's button

    CCLayer::onExit();
}
