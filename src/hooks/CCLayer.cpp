#include "CCLayer.hpp"
#include "../utils.hpp"
#include "../globals.hpp"

// clear g_button to force find a new button on this layer if its the topmost one 
void HookedCCLayer::onEnter() {
    g_button = nullptr;
    CCLayer::onEnter();
}

// clear g_button if we have to
void HookedCCLayer::onExit() {
    for (auto button : cl::utils::gatherAllButtons(this)) {
        if (g_button == button) {
            geode::log::info("cleared current btn");
            g_button = nullptr;
        }
    }

    // TODO: preserve last layer's button

    CCLayer::onExit();
}
