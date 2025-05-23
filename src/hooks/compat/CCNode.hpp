#include <Geode/modify/CCNode.hpp>

class $modify(HookedCCNode, cocos2d::CCNode) {
    void setShaderProgram(cocos2d::CCGLProgram* program);

    void setButtonShaderProgram();
    void revertShaderProgram();
};
