#include "shaders.hpp"

// pull requests which improve this will be appreciated and most likely merged
// ideally make anti-aliasing better and perhaps more efficient?

const GLchar* g_outlineShaderFragment = R"(
#ifdef GL_ES
precision lowp float;
#endif

varying vec2 v_texCoord;
uniform sampler2D CC_Texture0;

uniform vec2 u_screenSize;
uniform vec4 u_outlineColor;
uniform float u_thickness;
uniform bool u_includeShadow;

void main() {
    vec4 here = texture2D(CC_Texture0, v_texCoord);

    // doing this allows any return; statements to make the outline, and any
    // discard; statements to let the background show through
    vec4 col = u_outlineColor;
    col += here * here.a;
    gl_FragColor = col;

    // discard any solid pixels, let them show through
    if (u_includeShadow) {
        if (here.a > 0.01) discard;
    } else {
        if (here.a > 0.9) discard;
    }

    vec2 texelSize = (1.0 / u_screenSize) * u_thickness;

    // go through surrounding pixels of this transparent pixel
    for (float x = -1; x <= 1; x++) {
        for (float y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue;

            vec2 offset = vec2(x, y) * texelSize;
            float alpha = texture2D(CC_Texture0, v_texCoord + offset).a;

            // if any surrounding pixel is solid enough then make this outline
            if (u_includeShadow) {
                if (alpha > 0.01) return;
            } else {
                if (alpha > 0.9) return;
            }
        }
    }

    // if there are no solid pixels, let background show through
    discard;
}
)";

const GLchar* g_outlineShaderVertex = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;

#ifdef GL_ES
varying mediump vec2 v_texCoord;
#else
varying vec2 v_texCoord;
#endif

void main() {
    gl_Position = CC_MVPMatrix * a_position;
    v_texCoord = a_texCoord;
}
)";
