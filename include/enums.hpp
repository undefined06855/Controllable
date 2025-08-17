#pragma once

namespace controllable {

enum class Direction {
    None = 0,
    Up, Down, Left, Right,
};

enum class GamepadDirection {
    None = 0,
    Up, Down, Left, Right,
    JoyUp, JoyDown, JoyLeft, JoyRight,
    SecondaryJoyUp, SecondaryJoyDown, SecondaryJoyLeft, SecondaryJoyRight
};

// up, down, left, right should only be used when passing through fallback
enum class GamepadButton {
    None = 0,
    A, B, X, Y,
    Start, Select,
    L, R, ZL, ZR,
    Up, Down, Left, Right, // d-pad NOT joystick
    JoyLeft, JoyRight
};


// idk i gave up on names
enum class TryFocusRectType {
    Shrunken, Enlarged, FurtherEnlarged, Extreme
};

enum class NavigationArrowType {
    Left, Right
};

enum class FocusInteractionType {
    Unselect, Select, Activate
};

enum class FocusableNodeType {
    Unknown, Button, TextInput, DialogLayer
};


enum class ControllerDetectionType {
    Automatic, ForceNonController, ForceController
};

enum class SelectionOutlineType {
    Shader, Legacy, Hover
};


struct DebugInformation {
    TryFocusRectType m_tryFocusRectType;
    Direction m_tryFocusRectDirection;
    cocos2d::CCRect m_from;
    cocos2d::CCRect m_to;
};

}
