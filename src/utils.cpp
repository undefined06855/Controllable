#include "utils.hpp"

std::vector<cocos2d::CCMenuItem*> cl::utils::gatherAllButtons(cocos2d::CCNode* node) {
    if (!node) return {};
    if (!node->isVisible()) return {};

    // ccscene is special - only choose nodes from the thing with the highest z
    // order that also isnt persisted
    if (node == cocos2d::CCScene::get()) {
        cocos2d::CCNode* highestZOrderChild = nullptr;
        for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
            bool persisted = false;
            for (auto node : geode::SceneManager::get()->getPersistedNodes()) {
                if (node == child) {
                    persisted = true;
                    break;
                }
            }

            if (persisted) continue;
            
            if (highestZOrderChild == nullptr) {
                highestZOrderChild = child;
                continue;
            }

            if (child->getZOrder() >= highestZOrderChild->getZOrder()) {
                highestZOrderChild = child;
            }
        }

        return cl::utils::gatherAllButtons(highestZOrderChild);
    }

    // else every other normal node...

    std::vector<cocos2d::CCMenuItem*> ret = {};

    if (node->getUserObject("is-button"_spr)) {
        ret.push_back(static_cast<cocos2d::CCMenuItem*>(node));
    }

    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        for (auto button : cl::utils::gatherAllButtons(child)) {
            // is this offscreen?
            auto winSize = cocos2d::CCDirector::get()->getWinSize();
            auto buttonRect = cl::utils::getNodeBoundingBox(button);
            if (buttonRect.getMinX() > winSize.width) continue;
            if (buttonRect.getMaxX() < 0) continue;
            if (buttonRect.getMinY() > winSize.height) continue;
            if (buttonRect.getMaxY() < 0) continue;

            ret.push_back(button);
        }
    }

    return ret;
}

cocos2d::CCRect cl::utils::getNodeBoundingBox(cocos2d::CCNode* node) {
    if (!node || !node->getParent()) return { 0.f, 0.f, 0.f, 0.f };

    auto ret = node->boundingBox();

    auto bl = node->getParent()->convertToWorldSpace({ ret.getMinX(), ret.getMinY() });
    auto tr = node->getParent()->convertToWorldSpace({ ret.getMaxX(), ret.getMaxY() });

    return cocos2d::CCRect{ bl.x, bl.y, tr.x - bl.x, tr.y - bl.y };
}

cocos2d::CCRect cl::utils::createTryFocusRect(cocos2d::CCRect initialButtonRect, TryFocusRectType type, Direction direction) {
    cocos2d::CCRect tryFocusRect = initialButtonRect;

    // adjust initial pos to ensure the rect is to one side of the button
    // +1 to ensure buttons that are on the same secondary axis or overlapping
    // dont get selected
    switch (direction) {
        case Direction::Up:
            tryFocusRect.origin.y += initialButtonRect.size.height + 5.f;
            break;
        case Direction::Down:
            tryFocusRect.origin.y -= initialButtonRect.size.height + 5.f;
            break;
        case Direction::Left:
            tryFocusRect.origin.x -= initialButtonRect.size.width + 5.f;
            break;
        case Direction::Right:
            tryFocusRect.origin.x += initialButtonRect.size.width + 5.f;
            break;
        case Direction::None:
            break;
    }

    float distance;
    switch (type) {
        case TryFocusRectType::Shrunken:
        case TryFocusRectType::Enlarged:
            distance = 200.f;
            break;
        case TryFocusRectType::Extreme:
            distance = 1000.f;
            break;
    }

    // add distance
    switch (direction) {
        case Direction::Up:
            tryFocusRect.size.height += distance;
            break;
        case Direction::Down:
            tryFocusRect.origin.y -= distance;
            tryFocusRect.size.height += distance;
            break;
        case Direction::Left:
            tryFocusRect.origin.x -= distance;
            tryFocusRect.size.width += distance;
            break;
        case Direction::Right:
            tryFocusRect.size.width += distance;
            break;
        case Direction::None:
            break;
    }

    // other adjustments
    switch (type) {
        case TryFocusRectType::Shrunken:
            tryFocusRect.origin += cocos2d::CCPoint{ 5.f, 5.f };
            tryFocusRect.size -= cocos2d::CCPoint{ 10.f, 10.f };
            break;
        case TryFocusRectType::Enlarged:
            tryFocusRect.origin -= cocos2d::CCPoint{ 5.f, 5.f };
            tryFocusRect.size += cocos2d::CCPoint{ 10.f, 10.f };
            break;
        case TryFocusRectType::Extreme:
            switch (direction) {
                case Direction::None:
                case Direction::Up:
                case Direction::Down:
                    tryFocusRect.origin.x -= distance;
                    tryFocusRect.size.width += distance * 2.f;
                    break;
                case Direction::Left:
                case Direction::Right:
                    tryFocusRect.origin.y -= distance;
                    tryFocusRect.size.height += distance * 2.f;
                    break;
            }
            break;
    }

    return tryFocusRect;
}

cocos2d::CCMenuItem* cl::utils::findMostImportantButton(std::vector<cocos2d::CCMenuItem*>& buttons) {
    // TODO: find most important button depending on
    // - texture
    // - buttonsprite text "ok" etc
    // - position?
    return buttons[0];
}

GamepadButton cl::utils::directionToButton(Direction direction) {
    switch(direction) {
        case Direction::None:
            return GamepadButton::None;
        case Direction::Up:
            return GamepadButton::Up;
        case Direction::Down:
            return GamepadButton::Down;
        case Direction::Left:
            return GamepadButton::Left;
        case Direction::Right:
            return GamepadButton::Right;
    }
}

bool cl::utils::isPlayingLevel() {
    if (!GJBaseGameLayer::get()) return false; // no playlayer
    if (LevelEditorLayer::get()) return true; // leveleditorlayer
    if (cocos2d::CCScene::get()->getChildrenCount() == geode::SceneManager::get()->getPersistedNodes().size() + 1) return true; // only playlayer
    return false; // playlayer and something else
}
