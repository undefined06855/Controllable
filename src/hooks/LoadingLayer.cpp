#include "LoadingLayer.hpp"
#include "../globals.hpp"

const GLchar* g_newShaderFragment = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;

uniform vec2 u_screenSize;
uniform vec3 u_outlineColor;
uniform float u_thickness;
uniform bool u_includeShadow;

void main() {
    float here = texture2D(CC_Texture0, v_texCoord).a;

    // discard any solid pixels, let them show through
    if (u_includeShadow) {
        if (here > 0.1) discard;
    } else {
        if (here > 0.9) discard;
    }

    vec2 texelSize = (1.0 / u_screenSize) * u_thickness;

    // go through surrounding pixels of this transparent pixel
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue;

            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float alpha = texture2D(CC_Texture0, v_texCoord + offset).a;

            // if any surrounding pixel is solid enough then set this to red and
            // exit
            if (u_includeShadow) {
                if (alpha > 0.1) {
                    gl_FragColor = vec4(u_outlineColor, 1.0);
                    return;
                }
            } else {
                if (alpha > 0.9) {
                    gl_FragColor = vec4(u_outlineColor, 1.0);
                    return;
                } 
            }
        }
    }

    discard;
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
    bool ret = program->initWithVertexShaderByteArray(g_newShaderVertex, g_newShaderFragment);
    if (!ret) {
        geode::log::debug("{}", (cocos2d::CCObject*)(void*)0xb00b1e5);
    }

    program->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
    program->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);
    program->addAttribute(kCCAttributeNameColor, cocos2d::kCCVertexAttrib_Color);

    program->link();
    program->updateUniforms();
    
    // TODO: add settings for these
    auto size = cocos2d::CCDirector::get()->getWinSizeInPixels();
    auto sizeUniformLoc = program->getUniformLocationForName("u_screenSize");
    auto thicknessUniformLoc = program->getUniformLocationForName("u_thickness");
    auto outlineColorLoc = program->getUniformLocationForName("u_outlineColor");
    auto includeShadowLoc = program->getUniformLocationForName("u_includeShadow");
    program->setUniformLocationWith2f(sizeUniformLoc, size.width, size.height);
    program->setUniformLocationWith1f(thicknessUniformLoc, 3.5f);
    program->setUniformLocationWith3f(outlineColorLoc, 1.f, 0.f, 0.f);
    program->setUniformLocationWith1i(includeShadowLoc, (int)false);
    
    cocos2d::CCShaderCache::sharedShaderCache()->addProgram(program, "SelectedButtonShader"_spr);


    // controller
    // will most likely instantly be overridden by the cceglview hooks but eh
    g_isUsingController = PlatformToolbox::isControllerConnected();
    
    return true;
}
