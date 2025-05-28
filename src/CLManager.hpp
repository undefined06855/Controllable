#pragma once
#include "enums.hpp"
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
    SelectionOutlineType m_selectionOutlineType;

    float m_navigationCaretRepeatInterval;
    bool m_navigationReverseScroll;
    
    float m_controllerTriggerDeadzone;
    float m_controllerJoystickDeadzone;

    ControllerDetectionType m_otherForceState;

    cocos2d::CCGLProgram* m_outlineShaderProgram;
    bool m_forceSelectionIncludeShadow;
    
    void updateSettings();
    void updateShaders();
    void createShaders();
};

}
