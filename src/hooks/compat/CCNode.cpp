#include "CCNode.hpp"

void HookedCCNode::setShaderProgram(cocos2d::CCGLProgram* program) {
    if (program != cocos2d::CCShaderCache::sharedShaderCache()->programForKey("SelectedButtonShader"_spr)) {
        setUserObject("orig-shader"_spr, program);
    }

    CCNode::setShaderProgram(program);
}

void HookedCCNode::setButtonShaderProgram() {
    auto shader = cocos2d::CCShaderCache::sharedShaderCache()->programForKey("SelectedButtonShader"_spr);
    setShaderProgram(shader);
    
    for (auto child : geode::cocos::CCArrayExt<HookedCCNode*>(getChildren())) {
        child->setButtonShaderProgram();
    }
}

void HookedCCNode::revertShaderProgram() {
    auto shader = static_cast<cocos2d::CCGLProgram*>(getUserObject("orig-shader"_spr));
    setShaderProgram(shader);

    for (auto child : geode::cocos::CCArrayExt<HookedCCNode*>(getChildren())) {
        child->revertShaderProgram();
    }
}
