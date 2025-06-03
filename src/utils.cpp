#include "utils.hpp"
#include "ControllableManager.hpp"
#include "enums.hpp"
#include "globals.hpp"
#include "Controller.hpp"
#include <alphalaneous.alphas_geode_utils/include/Utils.h>

void cl::utils::clearCurrentButton() {
    if (!g_button) return;

    cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);

    g_button = nullptr;
}

void cl::utils::setCurrentButton(cocos2d::CCNode* node) {
    if (!node) return;

    cl::utils::clearCurrentButton();

    g_button = node;

    // select the button if we're in hover selection type
    if (cl::Manager::get().m_selectionOutlineType == SelectionOutlineType::Hover) {
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Select);
    }

    // start hovering button if we're pressing A
    if (g_controller.gamepadButtonPressed() == GamepadButton::A) {
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Select);
    }

    // if this is a link in an mdtextarea, call select and unselect to prevent
    // some uninitialised member bs
    // https://discord.com/channels/911701438269386882/911702535373475870/1378456881760305203
    if (g_button->getUserObject("requires-selected-before-unselected"_spr)) {
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Select);
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
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
    if (cl::utils::getFocusableNodeType(node) == FocusableNodeType::DialogLayer) {
        return { node };
    }

    // this node we should skip offscreen checks regardless of if theyve been
    // set or not, since this layer will most likely start offscreen
    if (node->getUserObject("skip-offscreen-checks"_spr)) {
        doOffscreenChecks = false;
    }

    std::vector<cocos2d::CCNode*> ret = {};

    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (cl::utils::canFocus(child, doOffscreenChecks)) {
            ret.push_back(child);
        }

        for (auto button : cl::utils::gatherAllButtons(child, false, doOffscreenChecks)) {
            ret.push_back(button);
        }
    }

    return ret;
}

bool cl::utils::canFocus(cocos2d::CCNode* node, bool doOffscreenChecks) {
    if (!node) return false;
    if (!node->isVisible()) return false;
    if (!node->getUserObject("is-focusable"_spr)) return false;
    if (node->getUserObject("should-not-focus"_spr)) return false;
    if (doOffscreenChecks && cl::utils::isNodeOffscreen(node)) return false;

    // node specific checks
    auto asButton = geode::cast::typeinfo_cast<cocos2d::CCMenuItem*>(node);
    auto asInput = geode::cast::typeinfo_cast<CCTextInputNode*>(node);
    auto asDialogLayer = geode::cast::typeinfo_cast<DialogLayer*>(node);
    if (
           (asButton && asButton->isEnabled())
        || (asInput)
        || (asDialogLayer)
    ) {
        return true;
    }

    return false;
}

cocos2d::CCRect cl::utils::getNodeBoundingBox(cocos2d::CCNode* node) {
    if (!node || !node->getParent()) return { 0.f, 0.f, 0.f, 0.f };

    auto ret = node->boundingBox();

    auto bl = node->getParent()->convertToWorldSpace({ ret.getMinX(), ret.getMinY() });
    auto tr = node->getParent()->convertToWorldSpace({ ret.getMaxX(), ret.getMaxY() });

    auto rect = cocos2d::CCRect{ bl.x, bl.y, tr.x - bl.x, tr.y - bl.y };

    // textinputs are so silly
    if (cl::utils::getFocusableNodeType(node) == FocusableNodeType::TextInput) {
        rect.origin.y -= rect.size.height / 2.f;

        // adjust for middle aligned text inputs
        if (auto cast = geode::cast::typeinfo_cast<CCTextInputNode*>(node)) {
            rect.origin.x -= rect.size.width * cast->m_textField->getAnchorPoint().x;
        } else {
            geode::log::warn("Getting bounding box of text input that is not a CCTextInputNode!");
        }

        // non geode textinputs are so CHUNKY
        if (!cl::utils::textInputIsFromGeode(node)) {
            auto fixedHeight = rect.size.height * .7f;
            rect.origin.y += (rect.size.height - fixedHeight) / 2.f;
            rect.size.height = fixedHeight;
        }
    }

    if (std::string_view(node->getID()) == "copy-username-button") {
        auto newWidth = rect.size.width * 6.5f;
        rect.origin.x -= newWidth / 2.f;
        rect.size.width = newWidth;
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

    float maximumJump;
    float jumpOffset;
    switch (type) {
        case TryFocusRectType::Shrunken:
            maximumJump = 5.f;
            jumpOffset = 7.5f;
            break;
        case TryFocusRectType::Enlarged:
        case TryFocusRectType::FurtherEnlarged:
            maximumJump = 20.f;
            jumpOffset = -7.5f;
            break;
        case TryFocusRectType::Extreme:
            maximumJump = 40.f;
            jumpOffset = -10.f;
            break;
    }

    // adjust initial pos to ensure the rect is to one side of the button
    // x1.5 to ensure buttons that are on the same secondary axis or overlapping
    // dont get selected
    // also here adjusts the rect if it gets scaled - e.g. Shrunken scales down
    // the rect but around the centre, meaning the jump distance will be larger
    // than it should be
    switch (direction) {
        case Direction::Up:
            tryFocusRect.origin.y += tryFocusRect.size.height;
            tryFocusRect.origin.y += std::min(maximumJump, tryFocusRect.size.height * 0.5f);
            tryFocusRect.origin.y -= jumpOffset;
            break;
        case Direction::Down:
            tryFocusRect.origin.y -= tryFocusRect.size.height;
            tryFocusRect.origin.y -= std::min(maximumJump, tryFocusRect.size.height * 0.5f);
            tryFocusRect.origin.y += jumpOffset;
            break;
        case Direction::Left:
            tryFocusRect.origin.x -= tryFocusRect.size.width;
            tryFocusRect.origin.x -= std::min(maximumJump, tryFocusRect.size.width * 0.5f);
            tryFocusRect.origin.x += jumpOffset;
            break;
        case Direction::Right:
            tryFocusRect.origin.x += tryFocusRect.size.width;
            tryFocusRect.origin.x += std::min(maximumJump, tryFocusRect.size.width * 0.5f);
            tryFocusRect.origin.x -= jumpOffset;
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
            tryFocusRect.origin -= cocos2d::CCPoint{ 7.5f, 7.5f };
            tryFocusRect.size += cocos2d::CCPoint{ 15.f, 15.f };
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
        { "more", 5 }, // moderrorpopup in mind

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
        { "refresh login", 4 },
        { "log in", 4 },
        { "login", 4 },
        { "next", 4 },

        // most popups - negative button
        { "no", -5 },
        { "cancel", -5 },
        { "exit", -6 }
    };

    static const std::unordered_map<std::string_view, int> buttonIDImportantness = {
        // levelselectlayer
        { "level-button", 5 },
        { "tower-button", 5 },
        { "secret-door-button", 5 },

        { "vaultkeeper-button", 5 }, // vaults
        { "enter-btn", 5 }, // the tower
        { "level-5001-button", 5 }, // the tower
        { "gauntlet-button-1", 5 }, // gauntlets

        // chests
        { "chest2", 5 }, // page 1 middle
        { "chest5", 5 }, // page 2 middle
        { "chest8", 7 }, // page 3 middle
        { "chest7", 6 }, // page 3 left
        { "chest9", 5 }, // page 3 right
        { "chest10", 5 }, // gold chest
        { "scratch-shop", 5 },

        // betterinfo page
        { "cvolton.betterinfo/featured-button", 5 }
    };

    for (auto button : buttons) {
        // dont do this for non-buttons
        if (cl::utils::getFocusableNodeType(button) != FocusableNodeType::Button) continue;
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
            auto caption = std::string(text->m_caption);
            std::transform(caption.begin(), caption.end(), caption.begin(), [](auto c){ return std::tolower(c); });

            if (buttonSpriteImportantness.contains(caption)) {
                importantness += buttonSpriteImportantness.at(caption);
            }
        }

        // check if this has an id
        auto id = std::string_view(button->getID());
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
        auto nodeName = AlphaUtils::Cocos::getClassName(child);

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

cocos2d::CCNode* cl::utils::findNavArrow(NavigationArrowType type) {
    auto buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());

    static constexpr std::array<std::string_view, 6> arrowButtonNames = {
        "controllerBtn_DPad_Left_001.png",
        "controllerBtn_DPad_Right_001.png",
        "GJ_arrow_01_001.png",
        "GJ_arrow_02_001.png",
        "GJ_arrow_03_001.png",
        "navArrowBtn_001.png"
    };

    for (auto button : buttons) {
        if (cl::utils::getFocusableNodeType(button) != FocusableNodeType::Button) continue;

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
                if (frameName == "navArrowBtn_001.png") isButtonRight = !isButtonRight;
                if (isFindingRight != isButtonRight) continue;

                // correct button now, is this potentially a back button?
                if (type == NavigationArrowType::Left) {
                    // it may be
                    auto bb = cl::utils::getNodeBoundingBox(button);
                    auto top = cocos2d::CCDirector::get()->getWinSize().height;
                    if (bb.getMaxX() < 30.f || bb.getMaxY() > top - 30.f) {
                        // probably is a back button :(
                        continue;
                    }

                    // probably isnt a back button I hope
                    return button;
                } else {
                    // it wont be a back button, this is certainly it
                    return button;
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

    switch (cl::utils::getFocusableNodeType(node)) {
        case FocusableNodeType::Unknown:
            geode::log::warn("Attempted interaction on unknown node type!");
            break;
        
        case FocusableNodeType::Button: {
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
            } else {
                geode::log::warn("Was interacting with button that is not a CCMenuItem!");
                return false;
            }
        }

        case FocusableNodeType::TextInput: {
            // this used to select the text input on button down instead of
            // button up like it does now (makes more sense imo) but complicated
            // other parts of the code so its just all done on button up now
            if (auto cast = geode::cast::typeinfo_cast<CCTextInputNode*>(node)) {
                auto touch = new cocos2d::CCTouch;
                auto bb = cl::utils::getNodeBoundingBox(node);
                touch->autorelease();

                switch (interaction) {
                    case FocusInteractionType::Unselect: {
                        touch->setTouchInfo(cocos2d::CCTOUCHBEGAN, 99999.f, 99999.f);
                        break;
                    }
                    
                    case FocusInteractionType::Select:
                        return true;
                        
                    case FocusInteractionType::Activate: {
                        // TODO: check if ck pr has been put into a release
                        auto point = cocos2d::CCPoint{ bb.getMaxX(), bb.getMidY() };
                        point = cocos2d::CCDirector::get()->convertToGL(point);
                        touch->autorelease();
                        touch->setTouchInfo(cocos2d::CCTOUCHBEGAN, point.x, point.y);
                        break;
                    }
                }
                
                cast->ccTouchBegan(touch, nullptr);

                return true;
            } else {
                geode::log::warn("Was interacting with text input that is not a CCTextInputNode!");
                return false;
            }
        }

        case FocusableNodeType::DialogLayer: {
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
            } else {
                geode::log::warn("Was interacting with dialog that is not a DialogLayer!");
                return false;
            }
        }
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
// note we cant use id to find flalertlayer like we can with gjdropdownlayer
// because we also need to return true for alerts that perhaps only inherit
bool cl::utils::shouldTreatParentAsImportant(cocos2d::CCNode* child) {
    // if (geode::cast::typeinfo_cast<GJDropDownLayer*>(child)) return true;
    auto id = std::string_view(child->getID());
    if (id == "GJDropDownLayer") return true;
    if (id == "EndLevelLayer") return true;
    if (geode::cast::typeinfo_cast<FLAlertLayer*>(child)) return true;
    if (cl::utils::getFocusableNodeType(child) == FocusableNodeType::DialogLayer) return true;
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
    static constexpr std::array<std::string_view, 4> ids = {
        "itzkiba.better_progression/tier-popup",
        "thesillydoggo.qolmod/QOLModButton",
        "dankmeme.globed2/notification-panel",
        "hjfod.quick-volume-controls/overlay"
    };

    return std::find(ids.begin(), ids.end(), std::string_view(child->getID())) != ids.end();
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
    // when this only applies to two cases in vanilla and a bit of geode

    auto id = std::string_view(node->getID());
    if (id == "level-button") return true;
    if (id == "hide-button") return true;

    // sliderthumbs look a bit ass
    // TODO: fix the half transparency compressed mess it has going on
    if (cl::utils::buttonIsActuallySliderThumb(node)) return true;

    // check user object
    if (node->getUserObject("force-shadowed-selection"_spr)) return true;
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (child->getUserObject("force-shadowed-selection"_spr)) {
            return true;
        }
    }

    return false;
}

bool cl::utils::shouldForceUseLegacySelection(cocos2d::CCNode* node) {
    if (!node) return false;

    auto& manager = cl::Manager::get();

    // if we have debug info force legacy
    if (manager.m_otherDebug) return true;

    // if we're not using shader this shouldnt matter
    if (manager.m_selectionOutlineType != SelectionOutlineType::Shader) return false;

    // if it failed to load the shader we should always use legacy
    if (manager.m_failedToLoadShader) return true;

    // cctextinputnode's ccscale9sprite isnt connected to the actual input in
    // any way that i can easily check so i cant put this in the force include
    // shadow check above which would make it look nicer

    if (cl::utils::getFocusableNodeType(node) == FocusableNodeType::TextInput) return true;

    auto id = std::string_view(node->getID());

    // secret door but invisible - need to show it
    if (id == "secret-door-button") {
        if (auto cast = geode::cast::typeinfo_cast<cocos2d::CCNodeRGBA*>(node->getChildByID("secret-door-sprite"))) {
            if (cast->getOpacity() == 0) return true;
        }
    }

    // the tower
    if (id == "enter-btn") return true;
    
    // and check user object
    if (node->getUserObject("force-legacy-selection"_spr)) return true;
    for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*>(node->getChildren())) {
        if (child->getUserObject("force-legacy-selection"_spr)) {
            return true;
        }
    }

    return false;
}

bool cl::utils::textInputIsFromGeode(cocos2d::CCNode* node) {
    if (!node->getParent()) return false;
    if (geode::cast::typeinfo_cast<geode::TextInput*>(node->getParent())) return true;
    return false;
}
