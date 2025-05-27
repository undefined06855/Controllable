#pragma once
#include "utils.hpp"

namespace cl {

// manages shaders and settings
class Manager {
    Manager();
public:
    static Manager& get();
    void init();
    
    float m_selectionThickness;
    cocos2d::ccColor4B m_selectionColor;
    bool m_selectionIncludeShadow;
    bool m_selectionLegacy;

    float m_navigationCaretRepeatInterval;
    bool m_navigationReverseScroll;
    
    float m_controllerTriggerDeadzone;
    float m_controllerJoystickDeadzone;

    ControllerDetectionType m_otherForceState;

    cocos2d::CCGLProgram* m_outlineShaderProgram;
    
    void updateSettings();
    void updateShaders();
    void createShaders();
};

}
