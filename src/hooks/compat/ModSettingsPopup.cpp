#include "ModSettingsPopup.hpp"

void HookedModSettingsPopup::modify() {
    if (!geode::Mod::get()->getSavedValue<bool>("shown-settings-notif", false)) {
        geode::Mod::get()->setSavedValue("shown-settings-notif", true);

        geode::Loader::get()->queueInMainThread([]{
            FLAlertLayer::create(
                "Controllable",
                "Note that <co>Controllable</c> does <cr>not</c> work (well) "
                "in <cy>Geode mod setting popups</c>!"
                "\n\n"
                "This message will only be shown <cg>once</c>.",
                "ok"
            )->show();
        });
    }
}
