#include "LevelEditorLayer.hpp"
#include "../../utils.hpp"

bool HookedLevelEditorLayer::init(GJGameLevel* level, bool p1) {
    if (!LevelEditorLayer::init(level, p1)) return false;

    if (cl::utils::isUsingController()) {
        geode::Notification::create("Controllers are not supported in the editor!", geode::NotificationIcon::Info)->show();
    }

    return true;
}
