#include "MenuLayer.hpp"

bool HookedMenuLayer::init() {
    if (!MenuLayer::init()) return false;

    // fixes an issue where navigating down from the main play button would
    // focus the newgrounds button since its slightly larger

    // menulayer ids provided by geode - no node ids dep needed
    auto bottomMenu = getChildByID("bottom-menu");
    if (!bottomMenu) return true;

    auto ngButton = bottomMenu->getChildByID("newgrounds-button");
    if (!ngButton) return true;

    ngButton->setContentSize({ 50.5f, 53.75f });

    return true;
}
