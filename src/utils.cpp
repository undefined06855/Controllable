#include "utils.hpp"

std::vector<cocos2d::CCMenuItem*> cl::utils::gatherAllButtons(cocos2d::CCNode* node) {
    return cl::utils::gatherAllButtons(node, node == cocos2d::CCScene::get());
}

std::vector<cocos2d::CCMenuItem*> cl::utils::gatherAllButtons(cocos2d::CCNode* node, bool important) {
    if (!node) return {};
    if (!node->isVisible()) return {};

    // important nodes are ccscene etc - nodes that contain popups
    if (important) {
        cocos2d::CCNode* highestZOrderChild = nullptr;
        for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
            // check if this is a persisted node - ignore
            bool persisted = false;
            for (auto node : geode::SceneManager::get()->getPersistedNodes()) {
                if (node == child) {
                    persisted = true;
                    break;
                }
            }
            if (persisted) continue;
            
            // havent found any yet we need something to fallback to
            if (!highestZOrderChild) {
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

    // check if this layer actually contains a popup/dropdown and it isnt
    // important - if so, treat node as an important layer (even if it isnt) to
    // find the topmost popup in this layer
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (geode::cast::typeinfo_cast<GJDropDownLayer*>(child)) {
            return cl::utils::gatherAllButtons(node, true);
        }
    }

    std::vector<cocos2d::CCMenuItem*> ret = {};

    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (
            child->getUserObject("is-button"_spr)
            && static_cast<cocos2d::CCMenuItem*>(child)->isEnabled()
            && !cl::utils::isNodeOffscreen(child)
            && child->isVisible()
        ) {
            ret.push_back(static_cast<cocos2d::CCMenuItem*>(child));
        }

        for (auto button : cl::utils::gatherAllButtons(child)) {
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

    // figure out the largest distance the rect should be
    float distance;
    switch (type) {
        case TryFocusRectType::Shrunken:
        case TryFocusRectType::Enlarged:
            if (direction == Direction::Up || direction == Direction::Down) {
                distance = 200.f;
            } else {
                distance = 100.f;
            }
            break;
        case TryFocusRectType::FurtherEnlarged:
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
        case TryFocusRectType::FurtherEnlarged:
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
    cocos2d::CCMenuItem* mostImportantButton = buttons[0]; // we need something to fall back on

    static const std::unordered_map<std::string_view, int> spriteImportantness = {
        { "GJ_arrow_03_001.png", 2 }, // back button, really just a fallback
        { "GJ_closeBtn_001.png", 2 }, // same

        { "GJ_infoIcon_001.png", 1 }, // info btn if there's literally nothing else

        { "GJ_chatBtn_001.png", 5 }, // commenting
        { "GJ_playBtn2_001.png", 5 }, // play button
        { "GJ_createBtn_001.png", 5 }, // create button in creatorlayer
        { "GJ_playBtn_001.png", 10 } // menulayer
    };

    static const std::unordered_map<std::string_view, int> buttonSpriteImportantness = {
        // most popups and stuff
        { "ok", 5 },
        { "yes", 5 },
        { "sure", 5 },
        { "confirm", 5 },
        { "submit", 5 },
        { "open", 5 },

        // for most layers this prioritises the button in the most top left
        // corner, not the button you're most likely going to be navigating to

        // levelcells
        { "view", 4 },
        { "get it", 4 },
        { "update", 4 },

        // settings
        { "account", 4 },
        { "save", 4 },
        { "links", 4 },

        // most popups - negative button
        { "no", -5 },
        { "cancel", -5 },
        { "exit", -6 },
    };

    for (auto button : buttons) {
        int importantness = 0;
        
        // check if this contains a sprite
        auto sprite = button->getChildByType<cocos2d::CCSprite*>(0);
        if (sprite) {
            auto frameName = cl::utils::getSpriteNodeFrameName(sprite);
            if (frameName.isOk() && spriteImportantness.contains(frameName.unwrap())) {
                importantness += spriteImportantness.at(frameName.unwrap());
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

    return cl::utils::isNodeClipped(node);
}

bool cl::utils::isNodeClipped(cocos2d::CCNode* node) {
    if (!node || !node->getParent()) return false;

    cocos2d::CCRect stencilBB;

    if (auto clip = cl::utils::findParentOfType<cocos2d::CCClippingNode*>(node)) {
        // ccclippingnode
        // this will not work for weirdly shaped stencils as it only checks its
        // bounding box and doesnt actually do any fancy checking with rendering
        auto oldParent = clip->m_pStencil->m_pParent;
        clip->m_pStencil->m_pParent = clip;
        stencilBB = cl::utils::getNodeBoundingBox(clip->m_pStencil);
        clip->m_pStencil->m_pParent = oldParent;
    } else if (auto scrollLayer = cl::utils::findParentOfType<geode::ScrollLayer*>(node)) {
        // scrolllayer
        stencilBB = cl::utils::getNodeBoundingBox(scrollLayer);
    } else if (auto tableView = cl::utils::findParentOfType<TableView*>(node)) {
        // tableview
        stencilBB = cl::utils::getNodeBoundingBox(tableView);
    } else {
        return false; // no clippinglayer or scrolllayer
    }

    auto bb = cl::utils::getNodeBoundingBox(node);

    if (!bb.intersectsRect(stencilBB)) {
        // child is ouside stencil bounding box
        return true;
    }

    return false;
}

template <class T>
T cl::utils::findParentOfType(cocos2d::CCNode* node) {
    if (auto cast = geode::cast::typeinfo_cast<T>(node)) return cast;
    else if (!node->getParent()) return nullptr;
    else return cl::utils::findParentOfType<T>(node->getParent());
}

geode::Result<std::string> cl::utils::getSpriteNodeFrameName(cocos2d::CCSprite* sprite) {
    std::string frameName = "";
    bool found = false;
    // taken from devtools
    if (auto texture = sprite->getTexture()) {
        auto cachedFrames = cocos2d::CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
        auto rect = sprite->getTextureRect();
        for (auto [key, frame] : geode::cocos::CCDictionaryExt<std::string, cocos2d::CCSpriteFrame*>(cachedFrames)) {
            if (frame->getTexture() == texture && frame->getRect() == rect) {
                frameName = key;
                found = true;
                break;
            }
        }
    }

    if (found) {
        return geode::Ok(frameName);
    } else {
        return geode::Err("Sprite does not have a frame name found in cache!");
    }
}

cocos2d::CCMenuItem* cl::utils::findNavArrow(NavigationArrowType type) {
    auto buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());

    static constexpr std::array<std::string_view, 5> arrowButtonNames = {
        "controllerBtn_DPad_Left_001.png",
        "controllerBtn_DPad_Right_001.png",
        "GJ_arrow_01_001.png",
        "GJ_arrow_02_001.png",
        "GJ_arrow_03_001.png"
    };

    geode::log::debug("Searching {} buttons for a nav button...", buttons.size());

    for (auto button : buttons) {
        auto sprite = button->getChildByType<cocos2d::CCSprite*>(0);
        if (sprite) {
            auto _frameName = cl::utils::getSpriteNodeFrameName(sprite);
            if (_frameName.isErr()) continue;
            auto frameName = _frameName.unwrap();

            if (std::find(arrowButtonNames.begin(), arrowButtonNames.end(), frameName) != arrowButtonNames.end()) {
                // is this facing the same direction?
                bool isFindingRight = type == NavigationArrowType::Right;
                bool isButtonRight = sprite->isFlipX();
                if (frameName == "controllerBtn_DPad_Right_001.png") isButtonRight = !isButtonRight;
                if (isFindingRight != isButtonRight) continue;

                // correct button now, is this potentially a back button?
                if (type == NavigationArrowType::Left) {
                    // it may be
                    auto bb = cl::utils::getNodeBoundingBox(button);
                    auto top = cocos2d::CCDirector::get()->getWinSize().height;
                    if (bb.getMaxX() < 30.f || bb.getMaxY() > top - 30.f) {
                        // probably is
                        continue;
                    }

                    // probably isnt!
                    return button;
                } else {
                    // it wont be
                    return button;
                }
            }
        }
    }

    return nullptr;
}
