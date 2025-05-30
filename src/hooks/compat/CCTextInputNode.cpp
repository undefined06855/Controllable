#include "CCTextInputNode.hpp"
#include "../../utils.hpp"
#include "../../globals.hpp"

bool HookedCCTextInputNode::init(float p0, float p1, char const* p2, char const* p3, int p4, char const* p5) {
    if (!CCTextInputNode::init(p0, p1, p2, p3, p4, p5)) return false;

    setUserObject("is-focusable"_spr, cocos2d::CCBool::create(true));
    setUserObject("is-text-input"_spr, cocos2d::CCBool::create(true));
    setUserObject("fix-text-input", cocos2d::CCBool::create(true)); // geode

    return true;
}

void HookedCCTextInputNode::onClickTrackNode(bool selected) {
    CCTextInputNode::onClickTrackNode(selected);

    if (cl::utils::isPlayingLevel()) return;

    if (!selected && g_button == this) {
        g_isEditingText = false;
    }

    if (selected) {
        g_isEditingText = true;
        g_button = this;
    }
}
