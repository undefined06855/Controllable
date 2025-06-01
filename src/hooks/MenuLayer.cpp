#include "MenuLayer.hpp"
#include "../ControllableManager.hpp"

bool HookedMenuLayer::init() {
    if (!MenuLayer::init()) return false;

    // show warning if shaders failed and set to non legacy
    if (cl::Manager::get().m_failedToLoadShader
     && cl::Manager::get().m_selectionOutlineType == SelectionOutlineType::Shader) {

        auto pop = FLAlertLayer::create(
            "Controllable",
            "The <co>shader</c> for the <cy>selected button outline</c> did "
            "<cr>not</c> compile properly!"
            "\n\n"
            "The outline type has been forced to <cj>Legacy</c>. Please report "
            "this as a <cr>bug</c>!",
            "ok"
        );
        pop->m_scene = this;
        pop->show();

        geode::Mod::get()->setSettingValue<std::string>("selection-outline-type", "Legacy");
    }

    // fixes an issue where navigating down from the main play button would
    // focus the newgrounds button since its slightly larger
    auto bottomMenu = getChildByID("bottom-menu");
    if (!bottomMenu) return true;

    auto ngButton = bottomMenu->getChildByID("newgrounds-button");
    if (!ngButton) return true;

    ngButton->setContentSize({ 50.5f, 53.75f });

    return true;
}
