#pragma once
#include "enums.hpp"

using namespace controllable;

namespace cl {

class Manager : public cocos2d::CCObject {
    Manager();
public:
    static Manager& get();
    void init();

    // selection settings
    float m_selectionThickness;
    cocos2d::ccColor4B m_selectionColor;
    bool m_selectionIncludeShadow;
    SelectionOutlineType m_selectionOutlineType;

    // navigation settings
    float m_navigationCaretRepeatInterval;
    bool m_navigationReverseScroll;

    // controller settings
    float m_controllerTriggerDeadzone;
    float m_controllerJoystickDeadzone;

    // other settings
    bool m_otherRemoveGDIcons;
    ControllerDetectionType m_otherForceState;
    bool m_otherDebug;

    bool m_settingsChangedThisFrame;

    bool m_androidLauncherOutdated;
    bool m_failedToLoadShader;

    cocos2d::CCGLProgram* m_outlineShaderProgram;

    bool m_forceSelectionIncludeShadow;

    float m_editingTextRepeatTimer;
    float m_scrollTime;

    // shader + settings stuff
    void updateSettings();
    void updateShaders();
    void createShaders();


    void update(float dt);

    // direction
    void pressDirection(GamepadDirection direction, bool allowFallback = true);
    void depressDirection(GamepadDirection direction);
    cocos2d::CCNode* attemptFindButton(Direction direction, cocos2d::CCRect rect, std::vector<cocos2d::CCNode*> buttons);

    // face buttons
    void pressButton(GamepadButton button, bool allowFallback = true);
    void depressButton(GamepadButton button, bool allowFallback = true);

    void fallbackToGD(GamepadButton button, GamepadDirection direction, bool down);

    void updateDrawNode();
};

}
