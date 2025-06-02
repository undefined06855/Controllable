#pragma once
#include "enums.hpp"

namespace cl::utils {

void clearCurrentButton();
void setCurrentButton(cocos2d::CCNode* node);

std::vector<cocos2d::CCNode*> gatherAllButtons(cocos2d::CCNode* node, bool allowSkipOffscreenChecks = true);
std::vector<cocos2d::CCNode*> gatherAllButtons(cocos2d::CCNode* node, bool important, bool doOffscreenChecks);
cocos2d::CCRect getNodeBoundingBox(cocos2d::CCNode* node);
cocos2d::CCRect createTryFocusRect(cocos2d::CCRect initialButtonRect, TryFocusRectType type, Direction direction);
cocos2d::CCNode* findMostImportantButton(std::vector<cocos2d::CCNode*>& buttons);

GamepadButton directionToButton(GamepadDirection direction);
Direction simplifyGamepadDirection(GamepadDirection direction);
bool directionIsSecondaryJoystick(GamepadDirection direction);

bool canFocus(cocos2d::CCNode* node, bool doOffscreenChecks);

bool isPlayingLevel();
bool isKeybindPopupOpen();

bool isNodeOffscreen(cocos2d::CCNode* node);
bool isNodeClipped(cocos2d::CCNode* node);

template <class T>
T findParentOfType(cocos2d::CCNode* node);

geode::Result<std::string> getSpriteNodeFrameName(cocos2d::CCSprite* sprite);

cocos2d::CCNode* findNavArrow(NavigationArrowType type);

bool interactWithFocusableElement(cocos2d::CCNode* node, FocusInteractionType interaction);

FocusableNodeType getFocusableNodeType(cocos2d::CCNode* node);
bool buttonIsActuallySliderThumb(cocos2d::CCNode* button);

bool shouldTreatParentAsImportant(cocos2d::CCNode* child);
bool shouldNotTreatAsPopup(cocos2d::CCNode* child);

bool isUsingController();

bool shouldForceIncludeShadow(cocos2d::CCNode* node);
bool shouldForceUseLegacySelection(cocos2d::CCNode* node);

bool textInputIsFromGeode(cocos2d::CCNode* node);

}
