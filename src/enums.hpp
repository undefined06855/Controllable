#pragma once

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
    Up, Down, Left, Right, // note: these are for the d-pad

    // these are for fallback
    JoyUp, JoyDown, JoyLeft, JoyRight,
    SecondaryJoyUp, SecondaryJoyDown, SecondaryJoyLeft, SecondaryJoyRight
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
    cocos2d::CCRect m_tryFocusRect;
    TryFocusRectType m_tryFocusRectType;
    cocos2d::CCRect m_from;
    cocos2d::CCRect m_to;
};
