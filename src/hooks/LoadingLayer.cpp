#include "LoadingLayer.hpp"
#include "../globals.hpp"

const GLchar* g_newShaderFragment = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;

void main() {
    vec4 color = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
    if (color.a <= 0.1) {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        color = vec4(0.0, 0.0, 0.0, 0.0);
    }
    gl_FragColor = color;
}
)";

const GLchar* g_newShaderVertex = R"(
attribute vec4 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoord;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
#endif

void main() {
    gl_Position = CC_MVPMatrix * a_position;
    v_fragmentColor = a_color;
    v_texCoord = a_texCoord;
}
)";

bool HookedLoadingLayer::init(bool p0) {
    if (!LoadingLayer::init(p0)) return false;

    // shader stuff
    auto program = new cocos2d::CCGLProgram;
    program->autorelease();
    program->initWithVertexShaderByteArray(g_newShaderVertex, g_newShaderFragment);

    program->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
    program->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);
    program->addAttribute(kCCAttributeNameColor, cocos2d::kCCVertexAttrib_Color);

    program->link();
    program->updateUniforms();

    cocos2d::CCShaderCache::sharedShaderCache()->addProgram(program, "SelectedButtonShader"_spr);
    
    return true;
}
