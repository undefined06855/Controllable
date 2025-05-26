#pragma once

enum class Direction {
    None = 0, Up, Down, Left, Right
};

// up, down, left, right should only be used when passing through fallback
enum class GamepadButton {
    None = 0, A, B, X, Y, Start, Select, L, R, ZL, ZR, Up, Down, Left, Right
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

namespace cl::utils {

void clearCurrentButton();
void setCurrentButton(cocos2d::CCNode* node);

std::vector<cocos2d::CCNode*> gatherAllButtons(cocos2d::CCNode* node);
std::vector<cocos2d::CCNode*> gatherAllButtons(cocos2d::CCNode* node, bool important, bool doOffscreenChecks);
cocos2d::CCRect getNodeBoundingBox(cocos2d::CCNode* node);
cocos2d::CCRect createTryFocusRect(cocos2d::CCRect initialButtonRect, TryFocusRectType type, Direction direction);
cocos2d::CCNode* findMostImportantButton(std::vector<cocos2d::CCNode*>& buttons);

GamepadButton directionToButton(Direction direction);

bool isPlayingLevel();
bool isKeybindPopupOpen();

bool isNodeOffscreen(cocos2d::CCNode* node);
bool isNodeClipped(cocos2d::CCNode* node);

template <class T>
T findParentOfType(cocos2d::CCNode* node);

std::string getNodeClassName(cocos2d::CCNode* node);
geode::Result<std::string> getSpriteNodeFrameName(cocos2d::CCSprite* sprite);

cocos2d::CCMenuItem* findNavArrow(NavigationArrowType type);

bool interactWithFocusableElement(cocos2d::CCNode* node, FocusInteractionType interaction);

FocusableNodeType getFocusableNodeType(cocos2d::CCNode* node);
bool buttonIsActuallySliderThumb(cocos2d::CCNode* button);

bool shouldTreatParentAsImportant(cocos2d::CCNode* child);
bool shouldNotTreatAsPopup(cocos2d::CCNode* child);

}
