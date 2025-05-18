#include "utils.hpp"

std::vector<cocos2d::CCMenuItem*> cl::utils::gatherAllButtons(cocos2d::CCNode* node) {
    if (!node) return {};
    if (!node->isVisible()) return {};

    // ccscene is special - only choose nodes from the thing with the highest z
    // order that also isnt persisted
    if (node == cocos2d::CCScene::get() || geode::cast::typeinfo_cast<RewardsPage*>(node)) {
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

    // else for every other normal node...

    std::vector<cocos2d::CCMenuItem*> ret = {};

    // remember to check this node!
    if (node->getUserObject("is-button"_spr) && static_cast<cocos2d::CCMenuItem*>(node)->isEnabled() && !cl::utils::isNodeOffscreen(node)) {
        ret.push_back(static_cast<cocos2d::CCMenuItem*>(node));
    }

    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        for (auto button : cl::utils::gatherAllButtons(child)) {
            // simple checks
            if (!button->isEnabled()) continue;
            if (cl::utils::isNodeOffscreen(button)) continue;

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
    // +8 to ensure buttons that are on the same secondary axis or overlapping
    // dont get selected
    switch (direction) {
        case Direction::Up:
            tryFocusRect.origin.y += initialButtonRect.size.height + 8.f;
            break;
        case Direction::Down:
            tryFocusRect.origin.y -= initialButtonRect.size.height + 8.f;
            break;
        case Direction::Left:
            tryFocusRect.origin.x -= initialButtonRect.size.width + 8.f;
            break;
        case Direction::Right:
            tryFocusRect.origin.x += initialButtonRect.size.width + 8.f;
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

    // minimum size enforcement
    if (tryFocusRect.size.height < 24.f) {
        float diff = 24.f - tryFocusRect.size.height;
        tryFocusRect.origin.y -= diff / 2.f;
        tryFocusRect.size.height += diff;
    }

    if (tryFocusRect.size.width < 24.f) {
        float diff = 24.f - tryFocusRect.size.width;
        tryFocusRect.origin.x -= diff / 2.f;
        tryFocusRect.size.width += diff;
    }

    return tryFocusRect;
}

cocos2d::CCMenuItem* cl::utils::findMostImportantButton(std::vector<cocos2d::CCMenuItem*>& buttons) {
    int mostImportantImportantness = -1;
    cocos2d::CCMenuItem* mostImportantButton = nullptr;

    static const std::unordered_map<std::string, int> spriteImportantness = {
        { "GJ_arrow_03_001.png", 1 }, // back button, really just a fallback
        { "GJ_closeBtn_001.png", 1 }, // same

        { "GJ_infoIcon_001.png", 2 }, // info btn

        { "GJ_chatBtn_001.png", 5 }, // commenting
        { "GJ_playBtn2_001.png", 5 }, // play button
        { "GJ_createBtn_001.png", 5 }, // create button in creatorlayer
        { "GJ_playBtn_001.png", 10 } // menulayer
    };

    static const std::unordered_map<std::string, int> buttonSpriteImportantness = {
        { "ok", 5 },
        { "yes", 5 },
        { "sure", 5 },
        { "confirm", 5 },
        { "submit", 5 },
        { "view", 5 },

        { "no", -5 },
        { "cancel", -5 },
        { "exit", -5 },
    };

    for (auto button : buttons) {
        int importantness = 0;
        
        // check if this contains a sprite
        auto sprite = button->getChildByType<cocos2d::CCSprite*>(0);
        if (sprite) {
            std::string frameName = "";
            // taken from devtools
            if (auto texture = sprite->getTexture()) {
                auto cachedFrames = cocos2d::CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
                auto rect = sprite->getTextureRect();
                for (auto [key, frame] : geode::cocos::CCDictionaryExt<std::string, cocos2d::CCSpriteFrame*>(cachedFrames)) {
                    if (frame->getTexture() == texture && frame->getRect() == rect) {
                        frameName = key;
                    }
                }
            }

            if (spriteImportantness.contains(frameName)) {
                importantness += spriteImportantness.at(frameName);
            }
        }

        // check if this contains a buttonsprite
        auto text = button->getChildByType<ButtonSprite*>(0);
        if (text) {
            // lowercase the caption
            auto caption = text->m_caption;
            std::transform(caption.begin(), caption.end(), caption.begin(), [](auto c){ return std::tolower(c); });

            if (buttonSpriteImportantness.contains(caption)) {
                importantness += buttonSpriteImportantness.at(caption);
            }
        }

        // and see if its more important
        if (importantness > mostImportantImportantness) {
            mostImportantImportantness = importantness;
            mostImportantButton = button;
        }

        // tie, compare Y positions - we want the highest (usually)
        if (importantness == mostImportantImportantness) {
            auto mostImportantBounding = cl::utils::getNodeBoundingBox(mostImportantButton);
            auto buttonBounding = cl::utils::getNodeBoundingBox(button);
            if (mostImportantBounding.getMidY() < buttonBounding.getMidY()) {
                mostImportantImportantness = importantness;
                mostImportantButton = button;
            }
        }
    }

    return mostImportantButton;
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

bool cl::utils::isNodeOffscreen(cocos2d::CCNode *node) {
    if (!node) return true;

    auto winSize = cocos2d::CCDirector::get()->getWinSize();
    auto bb = cl::utils::getNodeBoundingBox(node);
    if (bb.getMinX() > winSize.width) return true;
    if (bb.getMaxX() < 0) return true;
    if (bb.getMinY() > winSize.height) return true;
    if (bb.getMaxY() < 0) return true;

    return false;
}
