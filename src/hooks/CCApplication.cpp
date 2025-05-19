#include "CCApplication.hpp"
#include "../Controller.hpp"
#include "../globals.hpp"
#include "../utils.hpp"

void HookedCCApplication::updateControllerKeys(CXBOXController* controller, int player) {
    if (!controller->m_controllerConnected) return;
    if (player != 1) return;

    g_controller.update();

    // should be covered by the cclayer hooks but just in case
    if (cocos2d::CCDirector::get()->getIsTransitioning()) {
        g_button = nullptr;
        return;
    }

    // force reset if the button goes offscreen
    if (cl::utils::isNodeOffscreen(g_button)) {
        g_button = nullptr;
    }

    // if playing level, skip no button checks we dont need the button since
    // everything will go through fallbacks
    if (!cl::utils::isPlayingLevel()) {
        if (!g_button && cocos2d::CCScene::get()) {
            auto buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());
            if (buttons.size() == 0) return geode::log::debug("Waiting for buttons...");
            g_button = cl::utils::findMostImportantButton(buttons);
        }
    }

    // do the shit
    auto direction = g_controller.directionJustPressed();
    auto buttonPressed = g_controller.gamepadButtonJustPressed();
    auto buttonReleased = g_controller.gamepadButtonJustReleased();
    auto buttonPressing = g_controller.gamepadButtonPressed();

    // slider shenanigans
    if (g_isAdjustingSlider) {
        g_sliderNextFrame = g_controller.getLeftJoystick().x;

        if (buttonPressing == GamepadButton::Left) {
            g_sliderNextFrame = -1.f;
        } else if (buttonPressing == GamepadButton::Right) {
            g_sliderNextFrame = 1.f;
        }
    }


    if (direction != Direction::None) {
        focusInDirection(direction);
    }

    if (buttonPressed != GamepadButton::None) {
        pressButton(buttonPressed);
    }

    if (buttonReleased != GamepadButton::None) {
        depressButton(buttonReleased);
    }

    // scrolling
    g_scrollNextFrame = -g_controller.getRightJoystick().y;

    updateDrawNode();
}

void HookedCCApplication::focusInDirection(Direction direction) {
    if (!g_button) return;
    if (cl::utils::isPlayingLevel()) {
        geode::log::debug("direction fallback");
        pressButton(cl::utils::directionToButton(direction));
        depressButton(cl::utils::directionToButton(direction));
        return;
    }

    if (g_isAdjustingSlider) return;
    
    // find buttons with shrunken, enlarged, further enlarged and extreme rect types
    static const std::array<TryFocusRectType, 4> rectTypes = {
        TryFocusRectType::Shrunken,
        TryFocusRectType::Enlarged,
        TryFocusRectType::FurtherEnlarged,
        TryFocusRectType::Extreme
    };
    
    auto buttonRect = cl::utils::getNodeBoundingBox(g_button);
    std::vector<cocos2d::CCMenuItem*> buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());
    
    for (auto type : rectTypes) {
        cocos2d::CCRect tryFocusRect = cl::utils::createTryFocusRect(buttonRect, type, direction);
        if (auto button = attemptFindButton(direction, tryFocusRect, buttons)) {
            geode::log::debug("Found with {}", (int)type);
            g_button->unselected();
            g_button = button;
            if (g_controller.gamepadButtonPressed() == GamepadButton::A) g_button->selected();
            return;
        }
    }
}

cocos2d::CCMenuItem* HookedCCApplication::attemptFindButton(Direction direction, cocos2d::CCRect rect, std::vector<cocos2d::CCMenuItem*> buttons) {
    cocos2d::CCMenuItem* closestButton = nullptr;
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

    if (cl::utils::isPlayingLevel()) {
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
            // only used when this is the fallback from direction
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
        g_button->selected();
    } else if (button == GamepadButton::B && !g_isAdjustingSlider) {
        // B button simulates escape key
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocos2d::enumKeyCodes::KEY_Escape, true, false);
    }
}

void HookedCCApplication::depressButton(GamepadButton button) {
    if (LevelEditorLayer::get()) return;

    if (cl::utils::isPlayingLevel()) {
        // forward to cckeyboarddispatcher for gd built in handling
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
            // only used when this is the fallback from direction
            CONTROLLER_CASE(GamepadButton::Up, cocos2d::enumKeyCodes::CONTROLLER_Up, false)
            CONTROLLER_CASE(GamepadButton::Down, cocos2d::enumKeyCodes::CONTROLLER_Down, false)
            CONTROLLER_CASE(GamepadButton::Left, cocos2d::enumKeyCodes::CONTROLLER_Left, false)
            CONTROLLER_CASE(GamepadButton::Right, cocos2d::enumKeyCodes::CONTROLLER_Right, false)
            case GamepadButton::None: break;
        }

        return;
    }

    if (button == GamepadButton::A) {
        // a button activates button unless its a slider thumb
        if (!g_button) return;
        
        // deselect slider if we are
        if (g_isAdjustingSlider) {
            g_isAdjustingSlider = false;
            g_button->unselected();
            return;
        }

        // select slider if we aren't
        if (geode::cast::typeinfo_cast<SliderThumb*>(g_button)) {
            g_isAdjustingSlider = true;
            return;
        }

        g_button->activate();
    } else if (button == GamepadButton::B) {
        // B button simulates escape key unless on slider

        // deselect slider if we are
        if (g_isAdjustingSlider) {
            g_isAdjustingSlider = false;
            g_button->unselected();
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

#undef SEND_CONTROLLER_BTN

void HookedCCApplication::updateDrawNode() {
    // lol nobody uses notification node, might as well steal it
    if (!g_overlay) {
        g_overlay = cocos2d::CCDrawNode::create();
        g_overlay->retain();
        cocos2d::CCDirector::get()->setNotificationNode(g_overlay);
    }

    g_overlay->clear();

    if (g_button && g_isUsingController) {
        auto rect = cl::utils::getNodeBoundingBox(g_button);
        g_overlay->drawRect(rect, { 0, 0, 0, 0 }, 1.f, { 1, 0, 0, 1 });
    }
}
