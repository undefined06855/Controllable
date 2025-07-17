#include "MenuLayer.hpp"
#include "../steaminput.hpp"

bool WinHookedMenuLayer::init() {
    if (!MenuLayer::init()) return false;

    if (geode::Mod::get()->getSettingValue<bool>("other-suppress-steam-input")) return true;

    // check steam input
    if (!_SteamInput) return true;

    auto si = _SteamInput();
    _Init(si);
    _RunFrame(si); // shouldnt be needed?

    InputHandle_t handles[STEAM_INPUT_MAX_COUNT];
    _GetConnectedControllers(si, handles);

    geode::log::debug("handles: {}", handles);

    // sometimes this will be non-zero even if steam input is disabled, this is
    // a bug with steam input? but when this happens steam input is actually
    // certainly not enabled and not overriding the controller so who knows 
    if (handles[0] == 0) {
        geode::log::debug("Controller 0 does NOT use Steam Input!");
        return true;
    }

    geode::log::warn("Controller 0 uses Steam Input!");

    auto type = _GetInputTypeForHandle(si, handles[0]);
    auto handle = handles[0];

    static std::unordered_map<ESteamInputType, const char*> controllerNames = {
        { k_ESteamInputType_Unknown, "Unknown" },
        { k_ESteamInputType_SteamController, "Steam" },
        { k_ESteamInputType_XBox360Controller, "Xbox 360" },
        { k_ESteamInputType_XBoxOneController, "Xbox One" },
        { k_ESteamInputType_GenericGamepad, "Generic" }, // DirectInput controllers
        { k_ESteamInputType_PS4Controller, "PS4" },
        { k_ESteamInputType_AppleMFiController, "Apple MFi" }, // Unused
        { k_ESteamInputType_AndroidController, "Android" }, // Unused
        { k_ESteamInputType_SwitchJoyConPair, "Joy-Con Pair" }, // Unused
        { k_ESteamInputType_SwitchJoyConSingle, "Joy-Con" }, // Unused
        { k_ESteamInputType_SwitchProController, "Switch Pro" },
        { k_ESteamInputType_MobileTouch, "Mobile" }, // Steam Link App On-screen Virtual Controller
        { k_ESteamInputType_PS3Controller, "PS3" }, // Currently uses PS4 Origins
        { k_ESteamInputType_PS5Controller, "PS5" }, // Added in SDK 151
        { k_ESteamInputType_SteamDeckController, "Steam Deck" }, // Added in SDK 153
    };

    auto name = controllerNames[type];

    FLAlertLayer* popup = nullptr;

    switch (type) {
        case k_ESteamInputType_XBox360Controller:
        case k_ESteamInputType_XBoxOneController: {
            // steam input should NOT be enabled
            popup = geode::createQuickPopup(
                "Controllable",
                fmt::format(
                    "<co>Steam Input</c> is <cr>enabled</c>, which will conflict "
                    "with Controllable's built in support! Since you have an <cy>"
                "{}</c> Controller, <co>Steam Input</c> is <cr>not</c> "
                    "necessary.",
                    name
                ).c_str(),
                "Open Steam Input", nullptr, [si, handle, this](auto, bool){
                    _ShowBindingPanel(si, handle);
                    
                    auto prompt = geode::createQuickPopup(
                        "Controllable",
                        "You may have to <co>restart</c> the game to apply "
                        "<cj>Steam Input</c> settings!"
                        "\n"
                        "Note that in some cases Steam Input may still be "
                        "enabled after disabling. This is a <cr>bug</c> in "
                        "Steam Input, and cannot be fixed by Controllable.",
                        "Ignore", "Restart", [](auto, bool restart) {
                            if (restart) geode::utils::game::restart(true);
                        },
                        false
                    );
                    prompt->m_scene = this;
                    prompt->show();
                },
                false
            );

            geode::Mod::get()->setSettingValue("other-suppress-steam-input", true);

            break;
        }

        case k_ESteamInputType_Unknown:
        case k_ESteamInputType_GenericGamepad: {
            // steam input maybe?
            popup = geode::createQuickPopup(
                "Controllable",
                "<co>Steam Input</c> is <cr>enabled</c>, which may conflict "
                "with Controllable's built in support! Since Steam cannot tell "
                "what controller you have, <co>Steam Input</c> may or may not"
                "be necessary.",
                "Ignore", "Open Steam Input", [si, handle, this](auto, bool open) {
                    if (open) {
                        _ShowBindingPanel(si, handle);
                        
                        auto prompt = geode::createQuickPopup(
                            "Controllable",
                            "You may have to <co>restart</c> the game to apply "
                            "<cj>Steam Input</c> settings!"
                            "\n"
                            "Note that in some cases Steam Input may still be "
                            "enabled after disabling. This is a <cr>bug</c> in "
                            "Steam Input, and cannot be fixed by Controllable.",
                            "Ignore", "Restart", [](auto, bool restart) {
                                if (restart) geode::utils::game::restart(true);
                            },
                            false
                        );
                        prompt->m_scene = this;
                        prompt->show();
                    }
                },
                false
            );

            geode::Mod::get()->setSettingValue("other-suppress-steam-input", true);

            break;
        }

        case k_ESteamInputType_SteamController:
        case k_ESteamInputType_SwitchProController:
        case k_ESteamInputType_PS3Controller:
        case k_ESteamInputType_PS5Controller:
        case k_ESteamInputType_SteamDeckController:
        case k_ESteamInputType_PS4Controller: {
            // steam input SHOULD be enabled - but just to forward
            popup = geode::createQuickPopup(
                "Controllable",
                fmt::format(
                    "<co>Steam Input</c> is <cr>enabled</c>, which <co>may</c> "
                    "conflict with Controllable's built in support - however "
                    "since you have a normally unsupported {} Controller, "
                    "<co>Steam Input</c> <cg>is</c> necessary for the "
                    "controller to work. "
                    "\nEnsure your Steam Input is set to a controller layout "
                    "which does not simulate keyboard inputs.",
                    name
                ).c_str(),
                "Ignore", "Open Steam Input", [si, handle, this](auto, bool open) {
                    if (open) {
                        _ShowBindingPanel(si, handle);

                        auto prompt = geode::createQuickPopup(
                            "Controllable",
                            "You may have to <co>restart</c> the game to apply "
                            "<cj>Steam Input</c> settings!"
                            "\n"
                            "Note that in some cases Steam Input may still be "
                            "enabled after disabling. This is a <cr>bug</c> in "
                            "Steam Input, and cannot be fixed by Controllable.",
                            "Ignore", "Restart", [](auto, bool restart) {
                                if (restart) geode::utils::game::restart(true);
                            },
                            false
                        );
                        prompt->m_scene = this;
                        prompt->show();
                    }
                },
                false
            );

            geode::Mod::get()->setSettingValue("other-suppress-steam-input", true);

            break;
        }

        case k_ESteamInputType_MobileTouch: {
            // who the fuck using steam link
            popup = FLAlertLayer::create(
                "Controllable",
                "Controllable is <cr>not</c> needed for Steam Link to function!"
                "\nIf you really want to use Controllable, ensure your buttons "
                "are set to simulate <cj>controller inputs</c> and not "
                "keyboard keys.",
                "ok"
            );

            geode::Mod::get()->setSettingValue("other-suppress-steam-input", true);

            break;
        }

        case k_ESteamInputType_AppleMFiController:
        case k_ESteamInputType_AndroidController:
        case k_ESteamInputType_SwitchJoyConPair:
        case k_ESteamInputType_SwitchJoyConSingle:
            // unused
            break;

        case k_ESteamInputType_Count:
        case k_ESteamInputType_MaximumPossibleValue:
            // ??
            break;
    }

    if (popup) {
        popup->m_scene = this;
        popup->show();
    }

    return true;
}
