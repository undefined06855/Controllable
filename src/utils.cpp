#include "utils.hpp"
#include "ControllableManager.hpp"
#include "enums.hpp"
#include "globals.hpp"
#include "Controller.hpp"

void cl::utils::clearCurrentButton() {
    if (!g_button) return;

    // deselect old button if we're in hover selection type
    if (cl::Manager::get().m_selectionOutlineType == SelectionOutlineType::Hover) {
        if (auto cast = geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(g_button.data())) {
            cast->unselected();
        }
    }

    g_button = nullptr;
}

void cl::utils::setCurrentButton(cocos2d::CCNode* node) {
    if (!node) return;

    cl::utils::clearCurrentButton();

    g_button = node;

    // select the button if we're in hover selection type
    if (cl::Manager::get().m_selectionOutlineType == SelectionOutlineType::Hover) {
        if (auto cast = geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(g_button.data())) {
            cast->selected();
        }
    }

    // start hovering button if we're pressing A
    if (g_controller.gamepadButtonPressed() == GamepadButton::A) {
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Select);
    }
}

std::vector<cocos2d::CCNode*> cl::utils::gatherAllButtons(cocos2d::CCNode* node, bool allowSkipOffscreenChecks) {
    auto ret = cl::utils::gatherAllButtons(node, node == cocos2d::CCScene::get(), true);
    if (ret.size() == 0 && allowSkipOffscreenChecks) {
        // if we find no buttons, retry with offscreen checks disabled
        ret = cl::utils::gatherAllButtons(node, node == cocos2d::CCScene::get(), false);
    }

    return ret;
}

std::vector<cocos2d::CCNode*> cl::utils::gatherAllButtons(cocos2d::CCNode* node, bool important, bool doOffscreenChecks) {
    if (!node) return {};
    if (!node->isVisible()) return {};

    // important nodes are ccscene etc - nodes that contain popups
    if (important) {
        cocos2d::CCNode* highestZOrderChild = nullptr;
        for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
            if (cl::utils::shouldNotTreatAsPopup(child)) {
                continue;
            }
            
            // havent found any yet we need something to fallback to
            if (!highestZOrderChild) {
                highestZOrderChild = child;
                continue;
            }


            if (child->getZOrder() >= highestZOrderChild->getZOrder()) {
                highestZOrderChild = child;
            }
        }

        return cl::utils::gatherAllButtons(highestZOrderChild, false, doOffscreenChecks);
    }

    // else for every other normal node...

    // check if this layer actually contains a popup/dropdown and it isnt
    // important - if so, treat node as an important layer (even if it isnt) to
    // find the topmost popup in this layer
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (cl::utils::shouldTreatParentAsImportant(child)) {
            return cl::utils::gatherAllButtons(node, true, doOffscreenChecks);
        }
    }

    // parent is important, this is top z layer, this is first pass, we only
    // want this dialog layer to be selectable so we only have this dialog layer
    // to return
    if (node->getUserObject("is-dialog-layer"_spr)) {
        return { node };
    }

    // this node we should skip offscreen checks regardless of if theyve been
    // set or not, since this layer will most likely start offscreen
    if (node->getUserObject("skip-offscreen-checks"_spr)) {
        doOffscreenChecks = false;
    }

    std::vector<cocos2d::CCNode*> ret = {};

    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (
            child->getUserObject("is-focusable"_spr)
            && (doOffscreenChecks ? !cl::utils::isNodeOffscreen(child) : true)
            && child->isVisible()
        ) {
            // note: dialog layers are handled specially above
            auto asButton = geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(child);
            auto asInput = geode::cast::typeinfo_cast<CCTextInputNode*>(child);
            if (
                   (asButton && asButton->isEnabled())
                || (asInput)
            ) {
                ret.push_back(child);
            }
        }

        for (auto button : cl::utils::gatherAllButtons(child, false, doOffscreenChecks)) {
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

    auto rect = cocos2d::CCRect{ bl.x, bl.y, tr.x - bl.x, tr.y - bl.y };

    // TODO: doesnt work for textinputs in vaults?
    if (cl::utils::getFocusableNodeType(node) == FocusableNodeType::TextInput) {
        rect.origin.y -= rect.size.height / 2.f;
    }

    return rect;
}

cocos2d::CCRect cl::utils::createTryFocusRect(cocos2d::CCRect initialButtonRect, TryFocusRectType type, Direction direction) {
    cocos2d::CCRect tryFocusRect = initialButtonRect;

    // minimum size enforcement for the button
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

    // adjust initial pos to ensure the rect is to one side of the button
    // x1.5 to ensure buttons that are on the same secondary axis or overlapping
    // dont get selected (maximum 40 points extra)
    switch (direction) {
        case Direction::Up:
            tryFocusRect.origin.y += tryFocusRect.size.height;
            tryFocusRect.origin.y += std::min(40.f, tryFocusRect.size.height * 0.5f);
            break;
        case Direction::Down:
            tryFocusRect.origin.y -= tryFocusRect.size.height;
            tryFocusRect.origin.y -= std::min(40.f, tryFocusRect.size.height * 0.5f);
            break;
        case Direction::Left:
            tryFocusRect.origin.x -= tryFocusRect.size.width;
            tryFocusRect.origin.x -= std::min(40.f, tryFocusRect.size.width * 0.5f);
            break;
        case Direction::Right:
            tryFocusRect.origin.x += tryFocusRect.size.width;
            tryFocusRect.origin.x += std::min(40.f, tryFocusRect.size.width * 0.5f);
            break;
        case Direction::None:
            break;
    }

    // figure out the largest distance the rect should be
    float distance;
    switch (type) {
        case TryFocusRectType::Shrunken:
            distance = 60.f;
            break;
        case TryFocusRectType::Enlarged:
            distance = 100.f;
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
            tryFocusRect.origin += cocos2d::CCPoint{ 7.5f, 7.5f };
            tryFocusRect.size -= cocos2d::CCPoint{ 15.f, 15.f };
            break;
        case TryFocusRectType::Enlarged:
        case TryFocusRectType::FurtherEnlarged:
            tryFocusRect.origin -= cocos2d::CCPoint{ 5.f, 5.f };
            tryFocusRect.size += cocos2d::CCPoint{ 10.f, 10.f };
            break;
        case TryFocusRectType::Extreme:
            switch (direction) {
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
                case Direction::None:
                    break;
            }
            break;
    }

    return tryFocusRect;
}

cocos2d::CCNode* cl::utils::findMostImportantButton(std::vector<cocos2d::CCNode*>& buttons) {
    int mostImportantImportantness = -1;
    cocos2d::CCNode* mostImportantButton = buttons[0]; // we need something to fall back on at this point

    static const std::unordered_map<std::string_view, int> spriteImportantness = {
        { "GJ_arrow_01_001.png", 2 }, // back/exit buttons, really just a fallback
        { "GJ_arrow_02_001.png", 2 },
        { "GJ_arrow_03_001.png", 2 },
        { "GJ_closeBtn_001.png", 2 },

        { "GJ_infoIcon_001.png", 1 }, // info btn if there's literally nothing else

        { "GJ_chatBtn_001.png", 5 }, // commenting
        { "GJ_playBtn2_001.png", 5 }, // play button
        { "GJ_createBtn_001.png", 5 }, // create button in creatorlayer
        { "controllerBtn_Start_001.png", 5 }, // start button
        { "GJ_playBtn_001.png", 10 }, // menulayer

        { "GJ_secretLock4_001.png", 5 }, // the wraith doesnt have ids
    };

    static const std::unordered_map<std::string_view, int> buttonSpriteImportantness = {
        // most popups and stuff
        { "ok", 5 },
        { "yes", 5 },
        { "sure", 5 },
        { "confirm", 5 },
        { "submit", 5 },
        { "open", 5 },
        { "add", 5 }, // ck in mind

        // for most layers this prioritises the button in the most top left
        // corner, not the button you're most likely going to be navigating to

        // levelcells
        { "view", 4 },
        { "get it", 4 },
        { "update", 4 },

        // settings
        { "account", 4 },
        { "save", 4 }, // TODO: doesnt work?
        { "links", 4 },
        { "refresh login", 4 },
        { "next", 4 },

        // most popups - negative button
        { "no", -5 },
        { "cancel", -5 },
        { "exit", -6 }
    };

    static const std::unordered_map<std::string_view, int> buttonIDImportantness = {
        { "level-button", 5 }, // levelselectlayer
        { "tower-button", 5 },
        { "secret-door-button", 5 },

        { "vaultkeeper-button", 5 }, // vaults
        { "enter-btn", 5 }, // the tower
        { "level-5001-button", 5 }, // the tower (TODO: hmm not sure)
    };

    for (auto button : buttons) {
        if (!geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(button)) continue;
        int importantness = 0;
        
        // check if this contains a sprite
        auto sprite = button->getChildByType<cocos2d::CCSprite*>(0);
        if (sprite) {
            auto _frameName = cl::utils::getSpriteNodeFrameName(sprite);
            auto frameName = _frameName.unwrapOr("");
            if (_frameName.isOk() && spriteImportantness.contains(frameName)) {
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

        // check if this has an id
        auto id = button->getID();
        if (id != "") {
            if (buttonIDImportantness.contains(id)) {
                importantness += buttonIDImportantness.at(id);
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

            // this literally will not happen i do not believe it
            // y position tie - choose leftmost one
            if (mostImportantBounding.getMidY() == buttonBounding.getMidY()) {
                if (mostImportantBounding.getMidX() > buttonBounding.getMidX()) {
                    mostImportantImportantness = importantness;
                    mostImportantButton = button;
                }
            }
        }
    }

    return mostImportantButton;
}

GamepadButton cl::utils::directionToButton(GamepadDirection direction) {
    switch (direction) {
        case GamepadDirection::None:
            return GamepadButton::None;

        case GamepadDirection::Up:
            return GamepadButton::Up;
        case GamepadDirection::Down:
            return GamepadButton::Down;
        case GamepadDirection::Left:
            return GamepadButton::Left;
        case GamepadDirection::Right:
            return GamepadButton::Right;

        case GamepadDirection::JoyUp:
            return GamepadButton::JoyUp;
        case GamepadDirection::JoyDown:
            return GamepadButton::JoyDown;
        case GamepadDirection::JoyLeft:
            return GamepadButton::JoyLeft;
        case GamepadDirection::JoyRight:
            return GamepadButton::JoyRight;

        case GamepadDirection::SecondaryJoyUp:
            return GamepadButton::SecondaryJoyUp;
        case GamepadDirection::SecondaryJoyDown:
            return GamepadButton::SecondaryJoyDown;
        case GamepadDirection::SecondaryJoyLeft:
            return GamepadButton::SecondaryJoyLeft;
        case GamepadDirection::SecondaryJoyRight:
            return GamepadButton::SecondaryJoyRight;
    }
}

Direction cl::utils::simplifyGamepadDirection(GamepadDirection direction) {
    switch (direction) {
        case GamepadDirection::None:
            return Direction::None;

        case GamepadDirection::Up:
        case GamepadDirection::JoyUp:
        case GamepadDirection::SecondaryJoyUp:
            return Direction::Up;

        case GamepadDirection::Down:
        case GamepadDirection::JoyDown:
        case GamepadDirection::SecondaryJoyDown:
            return Direction::Down;

        case GamepadDirection::Left:
        case GamepadDirection::JoyLeft:
        case GamepadDirection::SecondaryJoyLeft:
            return Direction::Left;

        case GamepadDirection::Right:
        case GamepadDirection::JoyRight:
        case GamepadDirection::SecondaryJoyRight:
            return Direction::Right;
    }
}

bool cl::utils::directionIsSecondaryJoystick(GamepadDirection direction) {
    switch (direction) {
        case GamepadDirection::None:    
        case GamepadDirection::Up:
        case GamepadDirection::Down:
        case GamepadDirection::Left:
        case GamepadDirection::Right:
        case GamepadDirection::JoyUp:
        case GamepadDirection::JoyDown:
        case GamepadDirection::JoyLeft:
        case GamepadDirection::JoyRight:
            return false;

        case GamepadDirection::SecondaryJoyUp:
        case GamepadDirection::SecondaryJoyDown:
        case GamepadDirection::SecondaryJoyLeft:
        case GamepadDirection::SecondaryJoyRight:
            return true;
    }
}

bool cl::utils::isPlayingLevel() {
    if (!GJBaseGameLayer::get()) return false; // no playlayer
    if (LevelEditorLayer::get()) return true; // leveleditorlayer

    // check if playlayer has a popup-like class
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(GJBaseGameLayer::get()->getChildren())) {
        if (cl::utils::shouldTreatParentAsImportant(child)) return false;
    }

    // playlayer and something else - see if any other children have >1 child
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(cocos2d::CCScene::get()->getChildren())) {
        if (child != GJBaseGameLayer::get() && !cl::utils::shouldNotTreatAsPopup(child)) return false;
    }

    return true; // playlayer and literally nothing else
}

bool cl::utils::isKeybindPopupOpen() {
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(cocos2d::CCScene::get()->getChildren())) {
        auto nodeName = cl::utils::getNodeClassName(child);

        if (nodeName == "EnterBindLayer") {
            auto layer = static_cast<cocos2d::CCLayer*>(child->getChildren()->objectAtIndex(0));
            auto label = layer->getChildByType<cocos2d::CCLabelBMFont*>(1);
            if (label && label->getOpacity() == 155) {
                return true; // it wants a keybind right now
            }

            return false; // it doesn't
        }
    }

    return false;
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

std::string cl::utils::getNodeClassName(cocos2d::CCNode* node) {
#ifdef GEODE_IS_WINDOWS
    std::string_view nodeName = typeid(*node).name();
    if (nodeName.starts_with("class ")) nodeName.remove_prefix(6);
    if (nodeName.starts_with("struct ")) nodeName.remove_prefix(7);
    return std::string(nodeName);
#else
    std::string nodeName;

    int status = 0;
    auto demangle = abi::__cxa_demangle(typeid(*node).name(), 0, 0, &status);
    if (status == 0) {
        ret = demangle;
    }
    free(demangle);

    return nodeName;
#endif
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

    for (auto button : buttons) {
        if (!geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(button)) continue;

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
                    return static_cast<cocos2d::CCMenuItem*>(button);
                } else {
                    // it wont be
                    return static_cast<cocos2d::CCMenuItem*>(button);
                }
            }
        }
    }

    return nullptr;
}

bool cl::utils::interactWithFocusableElement(cocos2d::CCNode* node, FocusInteractionType interaction) {
    // Unselect - moving off of the button (regardless of selected or not)
    // Selected - A down on the button (should NOT activate/focus)
    // Activate - A up on the button (should activate/focus)

    if (auto cast = geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(node)) {
        switch (interaction) {
            case FocusInteractionType::Unselect:
                cast->unselected();
                break;
            case FocusInteractionType::Select:
                cast->selected();
                break;
            case FocusInteractionType::Activate:
                cast->activate();
                break;
        }

        return true;
    }

    // note: cctextinputnode SELECT DOES FOCUS the text input because it makes
    // more sense to focus it on button down than button up
    if (auto cast = geode::cast::typeinfo_cast<CCTextInputNode*>(node)) {
        auto touch = new cocos2d::CCTouch;
        auto bb = cl::utils::getNodeBoundingBox(node);
        touch->autorelease();

        switch (interaction) {
            case FocusInteractionType::Unselect: {
                touch->setTouchInfo(cocos2d::CCTOUCHBEGAN, 99999.f, 99999.f);
                break;
            }
            
            case FocusInteractionType::Select: {
                // TODO: check if ck pr has been put into a release
                auto point = cocos2d::CCPoint{ bb.getMaxX(), bb.getMidY() };
                point = cocos2d::CCDirector::get()->convertToGL(point);
                touch->autorelease();
                touch->setTouchInfo(cocos2d::CCTOUCHBEGAN, point.x, point.y);
                break;
            }

            case FocusInteractionType::Activate:
                break;
        }
        
        cast->ccTouchBegan(touch, nullptr);

        return true;
    }

    // nothing happens here except on activate (button UP) idk pretty boring
    if (auto cast = geode::cast::typeinfo_cast<DialogLayer*>(node)) {
        switch (interaction) {
            case FocusInteractionType::Unselect:
            case FocusInteractionType::Select:
                break;
            case FocusInteractionType::Activate: {
                cast->handleDialogTap();
            }
        }

        return true;
    }

    // pretty sure this log will crash :fire:
    geode::log::warn("No interactions possible for node {} - attempting {}!", node, (int)interaction);
    return false;
}

FocusableNodeType cl::utils::getFocusableNodeType(cocos2d::CCNode* node) {
    if (!node) return FocusableNodeType::Unknown;
    if (node->getUserObject("is-button"_spr)) return FocusableNodeType::Button;
    if (node->getUserObject("is-text-input"_spr)) return FocusableNodeType::TextInput;
    if (node->getUserObject("is-dialog-layer"_spr)) return FocusableNodeType::DialogLayer;
    return FocusableNodeType::Unknown;
}

bool cl::utils::buttonIsActuallySliderThumb(cocos2d::CCNode* button) {
    return geode::cast::typeinfo_cast<SliderThumb*>(button);
}

// this is used for any time a child is a popup but is added to the current 
// layer instead of ccscene so it doesnt automatically get treated as the child
// of an important layer
bool cl::utils::shouldTreatParentAsImportant(cocos2d::CCNode* child) {
    if (geode::cast::typeinfo_cast<GJDropDownLayer*>(child)) return true;
    if (geode::cast::typeinfo_cast<FLAlertLayer*>(child)) return true;
    if (child->getUserObject("is-dialog-layer"_spr)) return true;
    return false;
}

// checked in gather buttons for children of important layers
bool cl::utils::shouldNotTreatAsPopup(cocos2d::CCNode* child) {
    if (child->getChildrenCount() == 0) return true;

    auto persistedNodes = geode::SceneManager::get()->getPersistedNodes();
    if (std::find(persistedNodes.begin(), persistedNodes.end(), child) != persistedNodes.end()) {
        return true;
    }

    // mods that have persistent layers with buttons but dont use persistent nodes
    static constexpr std::array<std::string_view, 3> ids = {
        "itzkiba.better_progression/tier-popup",
        "thesillydoggo.qolmod/QOLModButton",
        "dankmeme.globed2/notification-panel",
        // "hjfod.quick-volume-controls/overlay",
    };

    return std::find(ids.begin(), ids.end(), child->getID()) != ids.end();
}

bool cl::utils::isUsingController() {
    switch (cl::Manager::get().m_otherForceState) {
        case ControllerDetectionType::Automatic:
            return g_isUsingController;
        case ControllerDetectionType::ForceNonController:
            return false;
        case ControllerDetectionType::ForceController:
            return true;
    }
}

bool cl::utils::shouldForceIncludeShadow(cocos2d::CCNode* node) {
    if (!node) return false;

    // if we're already including shadow this shouldnt matter
    if (cl::Manager::get().m_selectionIncludeShadow) return false;

    // was thinking about seeing every ccscale9sprite child of this node and
    // checking if its transparent enough perhaps, but that is somewhat overkill
    // when this only applies to literally this ingame

    if (node->getID() == "level-button") return true;

    return false;
}

bool cl::utils::shouldForceUseLegacySelection(cocos2d::CCNode* node) {
    if (!node) return false;

    // if we're already using legacy selection this shouldnt matter
    if (cl::Manager::get().m_selectionOutlineType == SelectionOutlineType::Legacy) return false;

    // cctextinputnode's ccscale9sprite isnt connected to the actual input in
    // any way that i can easily check so i cant put this in the force include
    // shadow check above which would make it look nicer

    if (cl::utils::getFocusableNodeType(node) == FocusableNodeType::TextInput) return true;

    return false;
}
