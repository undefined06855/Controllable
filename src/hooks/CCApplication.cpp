#include "CCApplication.hpp"
#include "../Controller.hpp"
#include "../globals.hpp"
#include "../utils.hpp"
#include <RenderTexture.hpp>

void HookedCCApplication::updateControllerKeys(CXBOXController* controller, int player) {
    if (!controller->m_controllerConnected) return;
    if (player != 1) return;

    g_controller.update();

    if (!cocos2d::CCScene::get()) return;

    // should be covered by the cclayer hooks but just in case
    if (cocos2d::CCDirector::get()->getIsTransitioning()) {
        cl::utils::clearCurrentButton();
        return;
    }

    // TODO: look at fine outline buttons being broken? https://discord.com/channels/911701438269386882/911702535373475870/1375889072081600603

    // force reset if the button goes offscreen
    if (cl::utils::isNodeOffscreen(g_button)) {
        cl::utils::clearCurrentButton();
    }

    // if playing level, skip no button checks we dont need the button since
    // everything will go through fallbacks
    if (!cl::utils::isPlayingLevel()) {
        if (!g_button && cocos2d::CCScene::get()) {
            auto buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());
            if (buttons.size() == 0) return geode::log::debug("Waiting for buttons...");
            cl::utils::setCurrentButton(cl::utils::findMostImportantButton(buttons));
        }
    }

    // do the shit
    auto directionPressed = g_controller.directionJustPressed();
    auto directionReleased = g_controller.directionJustReleased();
    auto directionPressing = g_controller.directionPressed();
    auto buttonPressed = g_controller.gamepadButtonJustPressed();
    auto buttonReleased = g_controller.gamepadButtonJustReleased();
    auto buttonPressing = g_controller.gamepadButtonPressed();

    // update g_isUsingController
    if (buttonPressed != GamepadButton::None || directionPressed != Direction::None) {
        g_isUsingController = true;
    }

    // slider shenanigans
    if (g_isAdjustingSlider) {
        g_sliderNextFrame = g_controller.getLeftJoystick().x;

        if (buttonPressing == GamepadButton::Left) {
            g_sliderNextFrame = -1.f;
        } else if (buttonPressing == GamepadButton::Right) {
            g_sliderNextFrame = 1.f;
        }
    }

    // text tshenanigans
    if (g_isEditingText) {
        auto cast = geode::cast::typeinfo_cast<CCTextInputNode*>(g_button.data());
        if (!cast) {
            geode::log::warn("was editing text but not focused on a text input!");
            g_isEditingText = false;
            return; // just in case
        }

        // use text repeat timer for pressing buttons, but if we've just pressed
        // a button, ignore text repeat timer
        // add setting for text navigation repeat speed
        if (g_editingTextRepeatTimer > .1f) {
            g_editingTextRepeatTimer = 0.f;

            // pressing - take timer into account
            if (directionPressing == Direction::Left || buttonPressing == GamepadButton::Left) {
                cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Left);
            } else if (directionPressing == Direction::Right || buttonPressing == GamepadButton::Right) {
                cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Right);
            }
        } else {
            // just pressed - ignore timer
            if (directionPressed == Direction::Left || buttonPressed == GamepadButton::Left) {
                cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Left);
                g_editingTextRepeatTimer = -.1f;
            } else if (directionPressed == Direction::Right || buttonPressed == GamepadButton::Right) {
                cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Right);
                g_editingTextRepeatTimer = -.1f;
            }
        }
    }

    if (directionPressed != Direction::None) {
        focusInDirection(directionPressed);
    }

    if (buttonPressed != GamepadButton::None) {
        pressButton(buttonPressed);
    }

    if (buttonReleased != GamepadButton::None) {
        depressButton(buttonReleased);
    }

    // scrolling
    // add setting for reverse scroll
    if (!cl::utils::isPlayingLevel()) {
        g_scrollNextFrame = -g_controller.getRightJoystick().y;
    }

    updateDrawNode();
}

void HookedCCApplication::focusInDirection(Direction direction) {
    if (!g_button) return;
    if (cl::utils::isPlayingLevel() || cl::utils::isKeybindPopupOpen()) {
        geode::log::debug("direction fallback");
        pressButton(cl::utils::directionToButton(direction));
        depressButton(cl::utils::directionToButton(direction));
        return;
    }

    if (g_isAdjustingSlider || g_isEditingText) return;

    // find buttons with shrunken, enlarged, further enlarged and extreme rect types
    static const std::array<TryFocusRectType, 4> rectTypes = {
        TryFocusRectType::Shrunken,
        TryFocusRectType::Enlarged,
        TryFocusRectType::FurtherEnlarged,
        TryFocusRectType::Extreme
    };

    auto buttonRect = cl::utils::getNodeBoundingBox(g_button);
    std::vector<cocos2d::CCNode*> buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());
    
    for (auto type : rectTypes) {
        cocos2d::CCRect tryFocusRect = cl::utils::createTryFocusRect(buttonRect, type, direction);
        auto actualDirection = type == TryFocusRectType::Extreme ? Direction::None : direction;
        if (auto button = attemptFindButton(actualDirection, tryFocusRect, buttons)) {
            geode::log::debug("Found with {}", (int)type);
            cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
            cl::utils::setCurrentButton(button);
            return;
        }
    }
}

cocos2d::CCNode* HookedCCApplication::attemptFindButton(Direction direction, cocos2d::CCRect rect, std::vector<cocos2d::CCNode*> buttons) {
    cocos2d::CCNode* closestButton = nullptr;
    geode::log::debug("Searching {} buttons", buttons.size());

    auto curButtonRect = cl::utils::getNodeBoundingBox(g_button);
    cocos2d::CCRect closestButtonRect = { 0.f, 0.f, 0.f, 0.f };
    for (auto button : buttons) {
        if (button == g_button) continue;

        auto buttonRect = cl::utils::getNodeBoundingBox(button);
        if (!buttonRect.intersectsRect(rect)) continue;
        
        // have we found any buttons yet?
        if (!closestButton) {
            closestButton = button;
            closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
            continue;
        }

        // compare different points depending on direction
        // for non-none directions, pretty sure i dont actually need to check
        // the difference between and i only need to check the positions
        switch (direction) {
            // use distance between centre of buttons - only used when extreme
            // tryFocusRect is used
            case Direction::None: {
                auto curButtonCenter = cocos2d::CCPoint{ curButtonRect.getMidX(), curButtonRect.getMidY() };
                float distance = cocos2d::CCPoint{ buttonRect.getMidX(), buttonRect.getMidY() }.getDistance(curButtonCenter);
                float closestDistance = cocos2d::CCPoint{ closestButtonRect.getMidX(), closestButtonRect.getMidY() }.getDistance(curButtonCenter);
                if (distance < closestDistance) {
                    closestButton = button;
                    closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                    continue;
                }
                break;
            }

            // use lowest points
            case Direction::Up: {
                float diffY = buttonRect.getMinY() - curButtonRect.getMinY();
                float closestDiffY = closestButtonRect.getMinY() - curButtonRect.getMinY();
                if (diffY < closestDiffY) {
                    closestButton = button;
                    closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                    continue;
                }
                if (diffY == closestDiffY) {
                    // check middles
                    float diffX = std::abs(buttonRect.getMidX() - curButtonRect.getMidX());
                    float closestDiffX = std::abs(closestButtonRect.getMidX() - curButtonRect.getMidX());
                    if (diffX < closestDiffX) {
                        closestButton = button;
                        closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                        continue;
                    }
                }
                break;
            }

            // use highest points
            case Direction::Down: {
                float diffY = curButtonRect.getMaxY() - buttonRect.getMaxY();
                float closestDiffY = curButtonRect.getMaxY() - closestButtonRect.getMaxY();
                if (diffY < closestDiffY) {
                    closestButton = button;
                    closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                    continue;
                }
                if (diffY == closestDiffY) {
                    // check middles
                    float diffX = std::abs(buttonRect.getMidX() - curButtonRect.getMidX());
                    float closestDiffX = std::abs(closestButtonRect.getMidX() - curButtonRect.getMidX());
                    if (diffX < closestDiffX) {
                        closestButton = button;
                        closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                        continue;
                    }
                }
                break;
            }

            // use rightmost points
            case Direction::Left: {
                float diffX = curButtonRect.getMaxX() - buttonRect.getMaxX();
                float closestDiffX = curButtonRect.getMaxX() - closestButtonRect.getMaxX();
                if (diffX < closestDiffX) {
                    closestButton = button;
                    closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                    continue;
                }
                if (diffX == closestDiffX) {
                    // check middles
                    float diffY = std::abs(buttonRect.getMidY() - curButtonRect.getMidY());
                    float closestDiffY = std::abs(closestButtonRect.getMidY() - curButtonRect.getMidY());
                    if (diffY < closestDiffY) {
                        closestButton = button;
                        closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                        continue;
                    }
                }
                break;
            }

            // use leftmost points
            case Direction::Right: {
                float diffX = buttonRect.getMinX() - curButtonRect.getMinX();
                float closestDiffX = closestButtonRect.getMinX() - curButtonRect.getMinX();
                if (diffX < closestDiffX) {
                    closestButton = button;
                    closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                    continue;
                }
                if (diffX == closestDiffX) {
                    // check middles
                    float diffY = std::abs(buttonRect.getMidY() - curButtonRect.getMidY());
                    float closestDiffY = std::abs(closestButtonRect.getMidY() - curButtonRect.getMidY());
                    if (diffY < closestDiffY) {
                        closestButton = button;
                        closestButtonRect = cl::utils::getNodeBoundingBox(closestButton);
                        continue;
                    }
                }
                break;
            }
        }
    }

    return closestButton;
}

#define CONTROLLER_CASE(gamepadBtn, cocosBtn, press) \
    case gamepadBtn: \
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocosBtn, press, false); \
        break;

void HookedCCApplication::pressButton(GamepadButton button) {
    if (LevelEditorLayer::get()) return;

    if (cl::utils::isPlayingLevel() || cl::utils::isKeybindPopupOpen()) {
        // forward to cckeyboarddispatcher for gd built in handling
        switch(button) {
            CONTROLLER_CASE(GamepadButton::A, cocos2d::enumKeyCodes::CONTROLLER_A, true)
            CONTROLLER_CASE(GamepadButton::B, cocos2d::enumKeyCodes::CONTROLLER_B, true)
            CONTROLLER_CASE(GamepadButton::X, cocos2d::enumKeyCodes::CONTROLLER_X, true)
            CONTROLLER_CASE(GamepadButton::Y, cocos2d::enumKeyCodes::CONTROLLER_Y, true)
            CONTROLLER_CASE(GamepadButton::Start, cocos2d::enumKeyCodes::CONTROLLER_Start, true)
            CONTROLLER_CASE(GamepadButton::Select, cocos2d::enumKeyCodes::CONTROLLER_Back, true)
            CONTROLLER_CASE(GamepadButton::L, cocos2d::enumKeyCodes::CONTROLLER_LB, true)
            CONTROLLER_CASE(GamepadButton::R, cocos2d::enumKeyCodes::CONTROLLER_RB, true)
            CONTROLLER_CASE(GamepadButton::ZL, cocos2d::enumKeyCodes::CONTROLLER_LT, true)
            CONTROLLER_CASE(GamepadButton::ZR, cocos2d::enumKeyCodes::CONTROLLER_RT, true)
            // these will also treat joystick inputs as d-pad inputs but whatever
            CONTROLLER_CASE(GamepadButton::Up, cocos2d::enumKeyCodes::CONTROLLER_Up, true)
            CONTROLLER_CASE(GamepadButton::Down, cocos2d::enumKeyCodes::CONTROLLER_Down, true)
            CONTROLLER_CASE(GamepadButton::Left, cocos2d::enumKeyCodes::CONTROLLER_Left, true)
            CONTROLLER_CASE(GamepadButton::Right, cocos2d::enumKeyCodes::CONTROLLER_Right, true)
            case GamepadButton::None: break;
        }

        return;
    }

    if (button == GamepadButton::A) {
        if (!g_button) return;
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Select);
    } else if (button == GamepadButton::B && !g_isAdjustingSlider && !g_isEditingText) {
        // B button simulates escape key
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocos2d::enumKeyCodes::KEY_Escape, true, false);
    }
}

void HookedCCApplication::depressButton(GamepadButton button) {
    if (LevelEditorLayer::get()) return;

    // there's a chance the fallback falls back to the pause button and presses
    // it (which is fine) - but if this had a similar condition the button would
    // never be released so the releasing code runs regardless of the playlayer
    // check
    switch(button) {
        CONTROLLER_CASE(GamepadButton::A, cocos2d::enumKeyCodes::CONTROLLER_A, false)
        CONTROLLER_CASE(GamepadButton::B, cocos2d::enumKeyCodes::CONTROLLER_B, false)
        CONTROLLER_CASE(GamepadButton::X, cocos2d::enumKeyCodes::CONTROLLER_X, false)
        CONTROLLER_CASE(GamepadButton::Y, cocos2d::enumKeyCodes::CONTROLLER_Y, false)
        CONTROLLER_CASE(GamepadButton::Start, cocos2d::enumKeyCodes::CONTROLLER_Start, false)
        CONTROLLER_CASE(GamepadButton::Select, cocos2d::enumKeyCodes::CONTROLLER_Back, false)
        CONTROLLER_CASE(GamepadButton::L, cocos2d::enumKeyCodes::CONTROLLER_LB, false)
        CONTROLLER_CASE(GamepadButton::R, cocos2d::enumKeyCodes::CONTROLLER_RB, false)
        CONTROLLER_CASE(GamepadButton::ZL, cocos2d::enumKeyCodes::CONTROLLER_LT, false)
        CONTROLLER_CASE(GamepadButton::ZR, cocos2d::enumKeyCodes::CONTROLLER_RT, false)
        // these will also treat joystick inputs as d-pad inputs but whatever
        CONTROLLER_CASE(GamepadButton::Up, cocos2d::enumKeyCodes::CONTROLLER_Up, false)
        CONTROLLER_CASE(GamepadButton::Down, cocos2d::enumKeyCodes::CONTROLLER_Down, false)
        CONTROLLER_CASE(GamepadButton::Left, cocos2d::enumKeyCodes::CONTROLLER_Left, false)
        CONTROLLER_CASE(GamepadButton::Right, cocos2d::enumKeyCodes::CONTROLLER_Right, false)
        case GamepadButton::None: break;
    }

    // only use fallback if we're playing level etc
    if (cl::utils::isPlayingLevel() || cl::utils::isKeybindPopupOpen()) {
        return;
    }

    if (button == GamepadButton::A) {
        // a button activates button
        if (!g_button) return;
        
        // deselect slider if we are
        if (g_isAdjustingSlider) {
            g_isAdjustingSlider = false;
            cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
            return;
        }

        // deselect text input if we are
        if (g_isEditingText) {
            g_isEditingText = false;
            cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
            return;
        }

        // select slider if we aren't and this is a slider
        if (cl::utils::buttonIsActuallySliderThumb(g_button)) {
            g_isAdjustingSlider = true;
            return;
        }

        // select text input if we aren't and this is a text input
        if (cl::utils::getFocusableNodeType(g_button) == FocusableNodeType::TextInput) {
            g_isEditingText = true;
            return;
        }

        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Activate);
    } else if (button == GamepadButton::B) {
        // B button simulates escape key unless on slider

        // deselect slider if we are
        if (g_isAdjustingSlider) {
            g_isAdjustingSlider = false;
            cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
            return;
        }

        // deselect text input if we are
        if (g_isEditingText) {
            g_isEditingText = false;
            cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
            return;
        }

        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocos2d::enumKeyCodes::KEY_Escape, false, false);
    } else if (button == GamepadButton::L) {
        // press any left buttons onscreen
        auto left = cl::utils::findNavArrow(NavigationArrowType::Left);
        if (left) left->activate();
    } else if (button == GamepadButton::R) {
        // press any right buttons onscreen
        auto right = cl::utils::findNavArrow(NavigationArrowType::Right);
        if (right) right->activate();
    }
}

#undef CONTROLLER_CASE

void HookedCCApplication::updateDrawNode() {
    // add setting to use legacy drawnode stuff
    if (false) {
        // lol nobody uses notification node, might as well steal it
        if (!g_overlay) {
            g_overlay = cocos2d::CCDrawNode::create();
            g_overlay->retain();
            cocos2d::CCDirector::get()->setNotificationNode(g_overlay);
        }
    
        g_overlay->clear();
    
        if (g_button && g_isUsingController && cl::utils::getFocusableNodeType(g_button) != FocusableNodeType::DialogLayer) {
            auto rect = cl::utils::getNodeBoundingBox(g_button);
            g_overlay->drawRect(rect, { 0, 0, 0, 0 }, 1.f, { 1, 0, 0, 1 });
        }
    
        // for (auto button : cl::utils::gatherAllButtons(cocos2d::CCScene::get())) {
        //     auto rect = cl::utils::getNodeBoundingBox(button);
        //     g_overlay->drawRect(rect, { 0, 0, 0, 0 }, .6f, { 0, 1, 0, 1 });
        // }
    } else {
        auto director = cocos2d::CCDirector::get();
        auto winSize = director->getWinSizeInPixels();

        auto bb = cl::utils::getNodeBoundingBox(g_button);
        int multiplier = director->getContentScaleFactor();

        cocos2d::CCDirector::get()->setNotificationNode(nullptr);

        g_buttonOverlay = RenderTexture(winSize.width, winSize.height, GL_RGBA, GL_RGBA, GL_LINEAR).intoManagedSprite();
        g_buttonOverlay->sprite->setShaderProgram(cocos2d::CCShaderCache::sharedShaderCache()->programForKey("SelectedButtonShader"_spr));
        g_buttonOverlay->sprite->setFlipY(true);
        g_buttonOverlay->sprite->ignoreAnchorPointForPosition(true);
        
        if (!g_button) return;

        auto origTransform = g_button->m_sTransform;
        g_button->m_sTransform = g_button->nodeToWorldTransform();
        g_buttonOverlay->render.capture(g_button);
        g_button->m_sTransform = origTransform;
        cocos2d::CCDirector::get()->setNotificationNode(g_buttonOverlay->sprite);
    }
}
