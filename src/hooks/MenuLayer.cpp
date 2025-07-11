#include "MenuLayer.hpp"
#include "../ControllableManager.hpp"
#include "../globals.hpp"

bool HookedMenuLayer::init() {
    if (!MenuLayer::init()) return false;

    showShaderWarning();
    showAndroidWarning();
    adjustNGButton();

    g_history.clear();

    return true;
}

void HookedMenuLayer::showShaderWarning() {
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
}

void HookedMenuLayer::showAndroidWarning() {
    // show warning if android launcher out of date
    // not sure how this could happen but it might
    if (cl::Manager::get().m_androidLauncherOutdated) {
        auto pop = FLAlertLayer::create(
            "Controllable",
            "Your <ca>Geode</c> Android Launcher version is too <cr>old</c> to "
            "support controllers! The launcher should update "
            "<co>automatically</c>, so simply <cj>restart</c> your game to "
            "update.",
            "ok"
        );
        pop->m_scene = this;
        pop->show();
    }
}

void HookedMenuLayer::adjustNGButton() {
    // fixes an issue where navigating down from the main play button would
    // focus the newgrounds button since its slightly larger
    auto bottomMenu = getChildByID("bottom-menu");
    if (!bottomMenu) return;

    auto ngButton = bottomMenu->getChildByID("newgrounds-button");
    if (!ngButton) return;

    ngButton->setContentSize({ 50.5f, 53.75f });
}
