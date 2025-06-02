#include "ControllableManager.hpp"
#include "globals.hpp"
#include "shaders.hpp"
#include "Controller.hpp"
#include "utils.hpp"
#include <RenderTexture.hpp>

// rest of the members are settings and can stay uninitialised
cl::Manager::Manager()
    : m_settingsChangedThisFrame(false)

    , m_failedToLoadShader(false)
    , m_outlineShaderProgram(nullptr)
    , m_forceSelectionIncludeShadow(false)

    , m_editingTextRepeatTimer(0.f)
    , m_scrollTime(0.f) {}

cl::Manager& cl::Manager::get() {
    static cl::Manager& instance = *new cl::Manager;
    return instance;
}

void cl::Manager::init() {
    // note: this will be called multiple times if multiple settings are changed
    geode::listenForAllSettingChanges([this](std::shared_ptr<geode::SettingV3>){
        updateSettings();
        m_settingsChangedThisFrame = true;
    });

    updateSettings();

    cocos2d::CCScheduler::get()->scheduleUpdateForTarget(this, 0, false);
    g_controller.update();
}

#define GET_SETTING(type, name) mod->getSettingValue<type>(name)

void cl::Manager::updateSettings() {
    auto mod = geode::Mod::get();

    m_selectionThickness = GET_SETTING(double, "selection-outline-thickness");
    m_selectionColor = GET_SETTING(cocos2d::ccColor4B, "selection-outline-color");
    m_selectionIncludeShadow = GET_SETTING(bool, "selection-outline-include-shadow");

    static const std::unordered_map<std::string_view, SelectionOutlineType> outlineMap = {
        { "Shader", SelectionOutlineType::Shader },
        { "Legacy", SelectionOutlineType::Legacy },
        { "Hover", SelectionOutlineType::Hover },
    };

    m_selectionOutlineType = outlineMap.at(GET_SETTING(std::string, "selection-outline-type"));

    m_navigationCaretRepeatInterval = GET_SETTING(double, "navigation-caret-repeat-interval");
    m_navigationReverseScroll = GET_SETTING(bool, "navigation-reverse-scroll");

    m_controllerTriggerDeadzone = GET_SETTING(int64_t, "controller-trigger-deadzone") / 100.f;
    m_controllerJoystickDeadzone = GET_SETTING(int64_t, "controller-joystick-deadzone") / 100.f;

    static const std::unordered_map<std::string_view, ControllerDetectionType> detectionMap = {
        { "Automatic", ControllerDetectionType::Automatic },
        { "Force Not Using Controller", ControllerDetectionType::ForceNonController },
        { "Force Using Controller", ControllerDetectionType::ForceController },
    };

    m_otherRemoveGDIcons = GET_SETTING(bool, "other-remove-gd-icons");
    m_otherForceState = detectionMap.at(GET_SETTING(std::string, "other-force-state"));
    m_otherDebug = GET_SETTING(bool, "other-debug");

    if (m_settingsChangedThisFrame) return;

    updateShaders();
}

#undef GET_SETTING

void cl::Manager::updateShaders() {
    if (!m_outlineShaderProgram) {
        createShaders();
    }

    // set uniforms that will change here
    m_outlineShaderProgram->use();
    auto thicknessUniformLoc = m_outlineShaderProgram->getUniformLocationForName("u_thickness");
    auto outlineColorLoc = m_outlineShaderProgram->getUniformLocationForName("u_outlineColor");
    auto includeShadowLoc = m_outlineShaderProgram->getUniformLocationForName("u_includeShadow");
    m_outlineShaderProgram->setUniformLocationWith1f(thicknessUniformLoc, m_selectionThickness);
    m_outlineShaderProgram->setUniformLocationWith4f(
        outlineColorLoc,
        m_selectionColor.r / 255.f,
        m_selectionColor.g / 255.f,
        m_selectionColor.b / 255.f,
        m_selectionColor.a / 255.f
    );
    m_outlineShaderProgram->setUniformLocationWith1i(includeShadowLoc, m_selectionIncludeShadow || m_forceSelectionIncludeShadow);
}

void cl::Manager::createShaders() {
    // please dont release this thanks
    m_outlineShaderProgram = new cocos2d::CCGLProgram;
    bool ret = m_outlineShaderProgram->initWithVertexShaderByteArray(g_outlineShaderVertex, g_outlineShaderFragment);
    if (!ret) {
        // geode::log::debug("{}", (cocos2d::CCObject*)(void*)0xb00b1e5);
        m_failedToLoadShader = true;
        return;
    }

    m_outlineShaderProgram->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
    m_outlineShaderProgram->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);

    m_outlineShaderProgram->link();
    m_outlineShaderProgram->updateUniforms();

    // set uniforms that wont change here
    m_outlineShaderProgram->use();
    auto size = cocos2d::CCDirector::get()->getWinSizeInPixels();
    auto sizeUniformLoc = m_outlineShaderProgram->getUniformLocationForName("u_screenSize");
    m_outlineShaderProgram->setUniformLocationWith2f(sizeUniformLoc, size.width, size.height);

    // here would traditionally be adding the shader to shader cache but it
    // doesnt really matter for us since we're only using it in our mod
}

void cl::Manager::update(float dt) {
    g_controller.update();
    m_settingsChangedThisFrame = false;
    
    auto application = cocos2d::CCApplication::get();
    application->m_pControllerHandler->m_controllerConnected = g_controller.m_connected;
    application->m_pController2Handler->m_controllerConnected = false;
    application->m_bControllerConnected = g_controller.m_connected;
    
    // call ck callback if needed
    if (g_ckCallback && cocos2d::CCScene::get()) {
        (g_ckTarget->*g_ckCallback)(dt);
    }

    // tell gd no controller connected to prevent ui showing up
    if (m_otherRemoveGDIcons) {
        application->m_pControllerHandler->m_controllerConnected = false;
        application->m_pController2Handler->m_controllerConnected = false;
        application->m_bControllerConnected = false;
    }

    if (!cocos2d::CCScene::get()) return;

    // TODO: look at fine outline buttons being broken? https://discord.com/channels/911701438269386882/911702535373475870/1375889072081600603
    // does this also happen for checkboxes / any switcher with two buttons?
    // TODO: android support???

    // force reset if the button goes offscreen or we are transitioning
    if (!cl::utils::canFocus(g_button, true) || cocos2d::CCDirector::get()->getIsTransitioning()) {
        cl::utils::clearCurrentButton();
    }

    // if playing level, skip no button checks we dont need the button since
    // everything will go through fallbacks
    if (!cl::utils::isPlayingLevel()) {
        if (!g_button && cocos2d::CCScene::get()) {
            auto buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get());
            if (buttons.size() == 0) {
                updateDrawNode(); // let it clear
                return;
            }
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
    if (buttonPressed != GamepadButton::None || directionPressed != GamepadDirection::None) {
        g_isUsingController = true;
    }

    // slider shenanigans
    if (g_isAdjustingSlider) {
        float slide = g_controller.getLeftJoystick().x;

        if (buttonPressing == GamepadButton::Left) {
            slide = -1.f;
        } else if (buttonPressing == GamepadButton::Right) {
            slide = 1.f;
        }

        auto cast = geode::cast::typeinfo_cast<SliderThumb*>(g_button.data());
        if (!cast) {
            geode::log::warn("Was editing slider but not focused on a SliderThumb!");
            g_isAdjustingSlider = false;
        } else {
            auto slider = static_cast<Slider*>(cast->getParent()->getParent());
            float newValue = cast->getValue() + slide * dt;
            newValue = std::max(0.f, std::min(newValue, 1.f));
            slider->setValue(newValue);
            slider->m_touchLogic->m_thumb->activate(); // update value
        }
    }

    // text tshenanigans
    if (g_isEditingText) {
        auto cast = geode::cast::typeinfo_cast<CCTextInputNode*>(g_button.data());
        if (!cast) {
            geode::log::warn("Was editing text but not focused on a CCTextInputNode!");
            g_isEditingText = false;
        } else {
            // use text repeat timer for pressing buttons, but if we've just pressed
            // a button, ignore text repeat timer
            m_editingTextRepeatTimer += dt;
            if (m_editingTextRepeatTimer > m_navigationCaretRepeatInterval) {
                m_editingTextRepeatTimer = 0.f;
                
                // pressing - take timer into account
                auto simpleDirection = cl::utils::simplifyGamepadDirection(directionPressing);

                if (simpleDirection == Direction::Left) {
                    cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Left);
                } else if (simpleDirection == Direction::Right) {
                    cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Right);
                }
            } else {
                // just pressed - ignore timer
                auto simpleDirection = cl::utils::simplifyGamepadDirection(directionPressed);

                if (simpleDirection == Direction::Left) {
                    cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Left);
                    m_editingTextRepeatTimer = -.1f;
                } else if (simpleDirection == Direction::Right) {
                    cast->onTextFieldInsertText(nullptr, "", 0, cocos2d::enumKeyCodes::KEY_Right);
                    m_editingTextRepeatTimer = -.1f;
                }
            }
        }
    }

    // scrolling
    if (!cl::utils::isPlayingLevel()) {
        auto y = g_controller.getRightJoystick().y;
        if (y > m_controllerJoystickDeadzone || y < -m_controllerJoystickDeadzone) {
            float scroll = -y;
            if (m_navigationReverseScroll) {
                scroll = -scroll;
            }

            auto mouseDispatcher = cocos2d::CCDirector::get()->getMouseDispatcher();
            float scrollSpeed = 25.f * std::pow(m_scrollTime, 6.f) + 380.f;
            mouseDispatcher->dispatchScrollMSG(scroll * dt * scrollSpeed, 0.f);
            m_scrollTime += dt * scroll;
        } else {
            m_scrollTime = 0.f;
        }
    }

    // general direction and button inputs
    // these should be called while in a level to allow them to fallthrough
    if (directionPressed != GamepadDirection::None) {
        pressDirection(directionPressed);
    }

    if (directionReleased != GamepadDirection::None) {
        depressDirection(directionReleased);
    }

    if (buttonPressed != GamepadButton::None) {
        pressButton(buttonPressed);
    }

    if (buttonReleased != GamepadButton::None) {
        depressButton(buttonReleased);
    }

    updateDrawNode();
}

void cl::Manager::pressDirection(GamepadDirection direction) {
    if (
        cl::utils::isPlayingLevel()
        || cl::utils::isKeybindPopupOpen()
        || cl::utils::directionIsSecondaryJoystick(direction)
    ) {
        fallbackToGD(GamepadButton::None, direction, true);
        return;
    }

    if (!g_button) return;
    if (g_isAdjustingSlider || g_isEditingText) return;

    // find buttons with shrunken, enlarged, further enlarged and extreme rect types
    static const std::array<TryFocusRectType, 4> rectTypes = {
        TryFocusRectType::Shrunken,
        TryFocusRectType::Enlarged,
        TryFocusRectType::FurtherEnlarged,
        TryFocusRectType::Extreme
    };

    auto buttonRect = cl::utils::getNodeBoundingBox(g_button);
    // false here means dont allow offscreen buttons to be focused
    std::vector<cocos2d::CCNode*> buttons = cl::utils::gatherAllButtons(cocos2d::CCScene::get(), false);

    auto simpleDirection = cl::utils::simplifyGamepadDirection(direction);

    for (auto type : rectTypes) {
        cocos2d::CCRect tryFocusRect = cl::utils::createTryFocusRect(buttonRect, type, simpleDirection);
        auto findDirection = type == TryFocusRectType::Extreme ? Direction::None : simpleDirection;
        if (auto button = attemptFindButton(findDirection, tryFocusRect, buttons)) {
            g_debugInformation = DebugInformation{
                .m_tryFocusRectType = type,
                .m_tryFocusRectDirection = simpleDirection,
                .m_from = cl::utils::getNodeBoundingBox(g_button),
                .m_to = cl::utils::getNodeBoundingBox(button)
            };
            cl::utils::setCurrentButton(button);
            return;
        }
    }
}

// this only exists for the fallback
void cl::Manager::depressDirection(GamepadDirection direction) {
    if (
        cl::utils::isPlayingLevel()
        || cl::utils::isKeybindPopupOpen()
        || cl::utils::directionIsSecondaryJoystick(direction)
    ) {
        fallbackToGD(GamepadButton::None, direction, false);
    }
}

cocos2d::CCNode* cl::Manager::attemptFindButton(Direction direction, cocos2d::CCRect rect, std::vector<cocos2d::CCNode*> buttons) {
    cocos2d::CCNode* closestButton = nullptr;

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
                    closestButtonRect = buttonRect;
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
                    closestButtonRect = buttonRect;
                    continue;
                }
                if (diffY == closestDiffY) {
                    // check middles
                    float diffX = std::abs(buttonRect.getMidX() - curButtonRect.getMidX());
                    float closestDiffX = std::abs(closestButtonRect.getMidX() - curButtonRect.getMidX());
                    if (diffX < closestDiffX) {
                        closestButton = button;
                        closestButtonRect = buttonRect;
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
                    closestButtonRect = buttonRect;
                    continue;
                }
                if (diffY == closestDiffY) {
                    // check middles
                    float diffX = std::abs(buttonRect.getMidX() - curButtonRect.getMidX());
                    float closestDiffX = std::abs(closestButtonRect.getMidX() - curButtonRect.getMidX());
                    if (diffX < closestDiffX) {
                        closestButton = button;
                        closestButtonRect = buttonRect;
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
                    closestButtonRect = buttonRect;
                    continue;
                }
                if (diffX == closestDiffX) {
                    // check middles
                    float diffY = std::abs(buttonRect.getMidY() - curButtonRect.getMidY());
                    float closestDiffY = std::abs(closestButtonRect.getMidY() - curButtonRect.getMidY());
                    if (diffY < closestDiffY) {
                        closestButton = button;
                        closestButtonRect = buttonRect;
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
                    closestButtonRect = buttonRect;
                    continue;
                }
                if (diffX == closestDiffX) {
                    // check middles
                    float diffY = std::abs(buttonRect.getMidY() - curButtonRect.getMidY());
                    float closestDiffY = std::abs(closestButtonRect.getMidY() - curButtonRect.getMidY());
                    if (diffY < closestDiffY) {
                        closestButton = button;
                        closestButtonRect = buttonRect;
                        continue;
                    }
                }
                break;
            }
        }
    }

    return closestButton;
}

void cl::Manager::pressButton(GamepadButton button) {
    if (LevelEditorLayer::get()) return;

    if (cl::utils::isPlayingLevel() || cl::utils::isKeybindPopupOpen()) {
        // forward to cckeyboarddispatcher for gd built in handling
        fallbackToGD(button, GamepadDirection::None, true);
        return;
    }

    if (button == GamepadButton::A) {
        if (!g_button) return;
        // this will only select most elements
        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Select);
    } else if (button == GamepadButton::B && !g_isAdjustingSlider && !g_isEditingText) {
        // B button simulates escape key
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocos2d::enumKeyCodes::KEY_Escape, true, false);
    }
}

void cl::Manager::depressButton(GamepadButton button) {
    if (LevelEditorLayer::get()) return;

    // there's a chance the fallback falls back to the pause button and presses
    // it (which is fine) - but if this had a similar condition the button would
    // never be released so the releasing code runs regardless of the playlayer
    // check
    // robtop does all the controller keybind stuff either on key down or in
    // that ccapplication function i removed idk i cant be bothered to check
    fallbackToGD(button, GamepadDirection::None, false);

    // only use fallback if we're playing level etc
    if (cl::utils::isPlayingLevel() || cl::utils::isKeybindPopupOpen()) {
        return;
    }

    if (button == GamepadButton::A) {
        // a button activates button
        if (!g_button) return;

        // select slider if we aren't and this is a slider
        // deselect slider if we are
        if (cl::utils::buttonIsActuallySliderThumb(g_button)) {
            g_isAdjustingSlider = !g_isAdjustingSlider;
            if (!g_isAdjustingSlider) {
                cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
                return;
            }
        }

        // select text input if we aren't and this is a text input
        // deselect text input if we are
        if (cl::utils::getFocusableNodeType(g_button) == FocusableNodeType::TextInput) {
            g_isEditingText = !g_isEditingText;
            if (!g_isEditingText) {
                cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Unselect);
                return;
            }
        }

        cl::utils::interactWithFocusableElement(g_button, FocusInteractionType::Activate);
    } else if (button == GamepadButton::B) {
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

        // else simulate escape key
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocos2d::enumKeyCodes::KEY_Escape, false, false);
    } else if (button == GamepadButton::L) {
        // press any left buttons onscreen
        auto left = cl::utils::findNavArrow(NavigationArrowType::Left);
        cl::utils::interactWithFocusableElement(left, FocusInteractionType::Activate);
    } else if (button == GamepadButton::R) {
        // press any right buttons onscreen
        auto right = cl::utils::findNavArrow(NavigationArrowType::Right);
        cl::utils::interactWithFocusableElement(right, FocusInteractionType::Activate);
    }
}

#define CONTROLLER_CASE(gamepadBtn, cocosBtn) \
    case gamepadBtn: \
        cocos2d::CCKeyboardDispatcher::get()->dispatchKeyboardMSG(cocos2d::enumKeyCodes::cocosBtn, down, false); \
        break;

void cl::Manager::fallbackToGD(GamepadButton button, GamepadDirection direction, bool down) {
    switch (button) {
        CONTROLLER_CASE(GamepadButton::A, CONTROLLER_A)
        CONTROLLER_CASE(GamepadButton::B, CONTROLLER_B)
        CONTROLLER_CASE(GamepadButton::X, CONTROLLER_X)
        CONTROLLER_CASE(GamepadButton::Y, CONTROLLER_Y)
        CONTROLLER_CASE(GamepadButton::Start, CONTROLLER_Start)
        CONTROLLER_CASE(GamepadButton::Select, CONTROLLER_Back)
        CONTROLLER_CASE(GamepadButton::L, CONTROLLER_LB)
        CONTROLLER_CASE(GamepadButton::R, CONTROLLER_RB)
        CONTROLLER_CASE(GamepadButton::ZL, CONTROLLER_LT)
        CONTROLLER_CASE(GamepadButton::ZR, CONTROLLER_RT)
        CONTROLLER_CASE(GamepadButton::Up, CONTROLLER_Up)
        CONTROLLER_CASE(GamepadButton::Down, CONTROLLER_Down)
        CONTROLLER_CASE(GamepadButton::Left, CONTROLLER_Left)
        CONTROLLER_CASE(GamepadButton::Right, CONTROLLER_Right)
        case GamepadButton::None: break;
    }

    switch (direction) {
        CONTROLLER_CASE(GamepadDirection::Up, CONTROLLER_Up)
        CONTROLLER_CASE(GamepadDirection::Down, CONTROLLER_Down)
        CONTROLLER_CASE(GamepadDirection::Left, CONTROLLER_Left)
        CONTROLLER_CASE(GamepadDirection::Right, CONTROLLER_Right)
        CONTROLLER_CASE(GamepadDirection::JoyUp, CONTROLLER_LTHUMBSTICK_UP)
        CONTROLLER_CASE(GamepadDirection::JoyDown, CONTROLLER_LTHUMBSTICK_DOWN)
        CONTROLLER_CASE(GamepadDirection::JoyLeft, CONTROLLER_LTHUMBSTICK_LEFT)
        CONTROLLER_CASE(GamepadDirection::JoyRight, CONTROLLER_LTHUMBSTICK_RIGHT)
        CONTROLLER_CASE(GamepadDirection::SecondaryJoyUp, CONTROLLER_RTHUMBSTICK_UP)
        CONTROLLER_CASE(GamepadDirection::SecondaryJoyDown, CONTROLLER_RTHUMBSTICK_DOWN)
        CONTROLLER_CASE(GamepadDirection::SecondaryJoyLeft, CONTROLLER_RTHUMBSTICK_LEFT)
        CONTROLLER_CASE(GamepadDirection::SecondaryJoyRight, CONTROLLER_RTHUMBSTICK_RIGHT)
        case GamepadDirection::None: break;
    }
}

#undef CONTROLLER_CASE

void cl::Manager::updateDrawNode() {
    auto director = cocos2d::CCDirector::get();
    director->setNotificationNode(nullptr);

    // dont draw outline if we have no bloody button to begin with
    if (!g_button) return;

    // dont draw the outline for dialoglayers
    if (cl::utils::getFocusableNodeType(g_button) == FocusableNodeType::DialogLayer) return;

    // dont draw outline if we're not using controller
    if (!cl::utils::isUsingController()) return;

    // dont draw outline if we're using hover selection type
    if (m_selectionOutlineType == SelectionOutlineType::Hover) return;

    if (m_selectionOutlineType == SelectionOutlineType::Legacy
     || cl::utils::shouldForceUseLegacySelection(g_button)) {
        auto overlay = cocos2d::CCDrawNode::create();
        overlay->clear();

        auto rect = cl::utils::getNodeBoundingBox(g_button);
        auto col = m_selectionColor;
        auto thickness = m_selectionThickness / 4.f;
        overlay->drawRect(
            rect,
            { 0.f, 0.f, 0.f, 0.f },
            thickness,
            {
                col.r / 255.f,
                col.g / 255.f,
                col.b / 255.f,
                col.a / 255.f
            }
        );

        // debug stuff
        static const std::unordered_map<TryFocusRectType, cocos2d::ccColor4F> rectColorMap = {
            { TryFocusRectType::Shrunken, { 0.f, 1.f, 0.f, 1.f } },
            { TryFocusRectType::Enlarged, { 0.f, 0.f, 1.f, 1.f } },
            { TryFocusRectType::FurtherEnlarged, { 1.f, .5f, 0.f, 1.f } },
            { TryFocusRectType::Extreme, { 1.f, 0.f, 0.f, 1.f } }
        };

        if (m_otherDebug) {
            auto fillCol = rectColorMap.at(g_debugInformation.m_tryFocusRectType);
            fillCol.a = .01f;
            overlay->drawRect(
                cl::utils::createTryFocusRect(
                    g_debugInformation.m_from,
                    g_debugInformation.m_tryFocusRectType,
                    g_debugInformation.m_tryFocusRectDirection
                ),
                fillCol,
                .1f,
                rectColorMap.at(g_debugInformation.m_tryFocusRectType)
            );

            overlay->drawRect(
                g_debugInformation.m_from,
                { 0.f, 0.f, 0.f, 0.f },
                .3f,
                { 1.f, 0.f, 1.f, 1.f }
            );

            overlay->drawRect(
                g_debugInformation.m_to,
                { 0.f, 0.f, 0.f, 0.f },
                .3f,
                { .5f, .0f, .5f, 1.f }
            );

            for (int i = (int)TryFocusRectType::Shrunken; i <= (int)TryFocusRectType::Extreme; i++) {
                auto rect = cl::utils::createTryFocusRect(
                    g_debugInformation.m_from,
                    (TryFocusRectType)i,
                    g_debugInformation.m_tryFocusRectDirection
                );
                overlay->drawRect(
                    rect,
                    { 0.f, 0.f, 0.f, 0.f },
                    .4f,
                    rectColorMap.at((TryFocusRectType)i)
                );
            }
        }

        overlay->setUserObject("is-special-and-important-notification-node"_spr, cocos2d::CCBool::create(true));
        director->setNotificationNode(overlay);
    } else {
        auto winSize = director->getWinSizeInPixels();

        auto bb = cl::utils::getNodeBoundingBox(g_button);
        int multiplier = director->getContentScaleFactor();

        auto overlay = RenderTexture(winSize.width, winSize.height, GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE).intoManagedSprite();
        overlay->sprite->setShaderProgram(m_outlineShaderProgram);
        overlay->sprite->setFlipY(true);
        overlay->sprite->ignoreAnchorPointForPosition(true);

        m_forceSelectionIncludeShadow = cl::utils::shouldForceIncludeShadow(g_button);
        updateShaders();

        auto origTransform = g_button->m_sTransform;
        g_button->m_sTransform = g_button->nodeToWorldTransform();
        overlay->render.capture(g_button);
        g_button->m_sTransform = origTransform;

        overlay->sprite->setUserObject("is-special-and-important-notification-node"_spr, cocos2d::CCBool::create(true));
        director->setNotificationNode(overlay->sprite);
    }
}
