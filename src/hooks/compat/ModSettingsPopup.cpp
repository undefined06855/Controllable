#include "ModSettingsPopup.hpp"
#include "../../utils.hpp"

void HookedModSettingsPopup::modify() {
    if (!cl::utils::isUsingController()) return;

    if (!geode::Mod::get()->getSavedValue<bool>("shown-settings-notif", false)) {
        geode::Mod::get()->setSavedValue("shown-settings-notif", true);

        geode::Loader::get()->queueInMainThread([]{
            FLAlertLayer::create(
                nullptr,
                "Controllable",
                "Note that <co>Controllable</c> does <cr>not</c> work well in "
                "<cy>Geode mod setting popups</c>, and you will most likely "
                "have to edit mod settings with "
                GEODE_DESKTOP("a <cj>keyboard and mouse</c>!")
                GEODE_MOBILE("your <cj>touchscreen</c>!")
                "\n\n"
                "This message will only be shown <cg>once</c>.",
                "ok",
                nullptr,
                380.f
            )->show();
        });
    }
}
