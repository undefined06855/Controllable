#include "CLManager.hpp"
#include "Geode/loader/Setting.hpp"
#include "shaders.hpp"

// rest can stay uninitialised
cl::Manager::Manager()
    : m_outlineShaderProgram(nullptr) {}

cl::Manager& cl::Manager::get() {
    static cl::Manager instance;
    return instance;
}

void cl::Manager::init() {
    // TODO: this runs for every setting changed, so may run multiple times
    geode::listenForAllSettingChanges([this](std::shared_ptr<geode::SettingV3>){
        updateSettings();
    });

    updateSettings();
}

#define GET_SETTING(type, name) mod->getSettingValue<type>(name)

void cl::Manager::updateSettings() {
    auto mod = geode::Mod::get();

    m_selectionThickness = GET_SETTING(double, "selection-outline-thickness");
    m_selectionColor = GET_SETTING(cocos2d::ccColor4B, "selection-outline-color");
    m_selectionIncludeShadow = GET_SETTING(bool, "selection-outline-include-shadow");
    m_selectionLegacy = GET_SETTING(bool, "selection-outline-legacy");

    m_navigationCaretRepeatInterval = GET_SETTING(double, "navigation-caret-repeat-interval");
    m_navigationReverseScroll = GET_SETTING(bool, "navigation-reverse-scroll");

    m_controllerTriggerDeadzone = GET_SETTING(int64_t, "controller-trigger-deadzone") / 100.f;
    m_controllerJoystickDeadzone = GET_SETTING(int64_t, "controller-joystick-deadzone") / 100.f;
    
    static const std::unordered_map<std::string_view, ControllerDetectionType> detectionMap = {
        { "Automatic", ControllerDetectionType::Automatic },
        { "Force Not Using Controller", ControllerDetectionType::ForceNonController },
        { "Force Using Controller", ControllerDetectionType::ForceController },
    };

    m_otherForceState = detectionMap.at(GET_SETTING(std::string, "other-force-state"));

    updateShaders();
}

#undef GET_SETTING

void cl::Manager::updateShaders() {
    if (!m_outlineShaderProgram) {
        // TODO: shader stuff breaks devtools!!
        createShaders();
    }

    // set uniforms that will change here
    m_outlineShaderProgram->use();
    auto thicknessUniformLoc = m_outlineShaderProgram->getUniformLocationForName("u_thickness");
    auto outlineColorLoc = m_outlineShaderProgram->getUniformLocationForName("u_outlineColor");
    auto includeShadowLoc = m_outlineShaderProgram->getUniformLocationForName("u_includeShadow");
    m_outlineShaderProgram->setUniformLocationWith1f(thicknessUniformLoc, m_selectionThickness);
    m_outlineShaderProgram->setUniformLocationWith4f(
        outlineColorLoc,
        m_selectionColor.r / 255.f,
        m_selectionColor.g / 255.f,
        m_selectionColor.b / 255.f,
        m_selectionColor.a / 255.f
    );
    m_outlineShaderProgram->setUniformLocationWith1i(includeShadowLoc, m_selectionIncludeShadow); // TODO: force this on for main levels
}

void cl::Manager::createShaders() {
    // please dont release this thanks
    m_outlineShaderProgram = new cocos2d::CCGLProgram;
    bool ret = m_outlineShaderProgram->initWithVertexShaderByteArray(g_outlineShaderVertex, g_outlineShaderFragment);
    if (!ret) {
        // TODO: better debugging for failed shader compilation
        geode::log::debug("{}", (cocos2d::CCObject*)(void*)0xb00b1e5);
    }

    m_outlineShaderProgram->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
    m_outlineShaderProgram->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);
    m_outlineShaderProgram->addAttribute(kCCAttributeNameColor, cocos2d::kCCVertexAttrib_Color);

    m_outlineShaderProgram->link();
    m_outlineShaderProgram->updateUniforms();
    
    // set uniforms that wont change here
    m_outlineShaderProgram->use();
    auto size = cocos2d::CCDirector::get()->getWinSizeInPixels();
    auto sizeUniformLoc = m_outlineShaderProgram->getUniformLocationForName("u_screenSize");
    m_outlineShaderProgram->setUniformLocationWith2f(sizeUniformLoc, size.width, size.height);
}
