#include "CCLayer.hpp"
#include "../utils.hpp"
#include "../globals.hpp"

// clear g_button to force find a new button on this layer if its the topmost one 
void HookedCCLayer::onEnter() {
    CCLayer::onEnter();

    if (getParent() == cocos2d::CCScene::get() || cl::utils::shouldTreatParentAsImportant(this)) {
        geode::log::debug("{}", getChildrenCount());
        
        // clear editing text or adjusting slider
        if (g_button) {
            if (cl::utils::getFocusableNodeType(g_button) == FocusableNodeType::TextInput) g_isEditingText = false;
            if (cl::utils::buttonIsActuallySliderThumb(g_button)) g_isAdjustingSlider = false;
        }

        // clear current button so we can find a new one
        // but push to history first
        // g_history.push_back({
        //     .m_button = g_button,
        //     .m_layerTypeinfo = cl::utils::getNodeClassName(this)
        // });
        cl::utils::clearCurrentButton();
    }

    // if (getParent() == cocos2d::CCScene::get() && cl::utils::getNodeClassName(this) == "MenuLayer") {
    //     if (g_history.size() != 1) {
    //         geode::log::warn("Excess elements in history! {} elements, expected 1!", g_history.size());
    //     }

    //     g_history.erase(g_history.begin() + 1, g_history.end());
    // }
}

// clear g_button if we have to
void HookedCCLayer::onExit() {
    // for (auto button : cl::utils::gatherAllButtons(this)) {
    //     // clear current button if its on this layer about to be removed
    //     if (g_button == button) {
    //         geode::log::debug("yeah");

    //         cl::utils::clearCurrentButton();
    //     }
    // }

    CCLayer::onExit();
}
