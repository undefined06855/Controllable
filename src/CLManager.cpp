#include "CLManager.hpp"
#include "Geode/loader/Setting.hpp"
#include "globals.hpp"
#include "shaders.hpp"

// rest are settings and can stay uninitialised
cl::Manager::Manager()
    : m_outlineShaderProgram(nullptr)
    , m_forceSelectionIncludeShadow(false) {}

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

    cocos2d::CCScheduler::get()->scheduleUpdateForTarget(this, 0, false);
}

#define GET_SETTING(type, name) mod->getSettingValue<type>(name)

void cl::Manager::updateSettings() {
    auto mod = geode::Mod::get();

    m_selectionThickness = GET_SETTING(double, "selection-outline-thickness");
    m_selectionColor = GET_SETTING(cocos2d::ccColor4B, "selection-outline-color");
    m_selectionIncludeShadow = GET_SETTING(bool, "selection-outline-include-shadow");

    static const std::unordered_map<std::string_view, SelectionOutlineType> outlineMap = {
        { "Shader", SelectionOutlineType::Shader },
        { "Legacy", SelectionOutlineType::Legacy },
        { "Hover", SelectionOutlineType::Hover },
    };

    m_selectionOutlineType = outlineMap.at(GET_SETTING(std::string, "selection-outline-type"));

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
    m_otherDebug = GET_SETTING(bool, "other-debug");

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
    m_outlineShaderProgram->setUniformLocationWith1i(includeShadowLoc, m_selectionIncludeShadow || m_forceSelectionIncludeShadow);
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

    m_outlineShaderProgram->link();
    m_outlineShaderProgram->updateUniforms();
    
    // set uniforms that wont change here
    m_outlineShaderProgram->use();
    auto size = cocos2d::CCDirector::get()->getWinSizeInPixels();
    auto sizeUniformLoc = m_outlineShaderProgram->getUniformLocationForName("u_screenSize");
    m_outlineShaderProgram->setUniformLocationWith2f(sizeUniformLoc, size.width, size.height);

    // here would traditionally be adding the shader to shader cache but it
    // doesnt really matter for us since we're only using it in our mod
}

// update globals and act on them
void cl::Manager::update(float dt) {
    if (g_scrollNextFrame != 0.f) {
        auto mouseDispatcher = cocos2d::CCDirector::get()->getMouseDispatcher();
        float scrollSpeed = 25.f * std::pow(g_scrollTime, 6.f) + 380.f;
        mouseDispatcher->dispatchScrollMSG(g_scrollNextFrame * dt * scrollSpeed, 0.f);
        g_scrollTime += dt * g_scrollNextFrame;
    }

    // TODO: g_scrollTime does not get reset if you switch scrolling directions
    // mid-scroll (edit: has been minimised with deadzone but still exists)
    if (g_scrollNextFrame == 0.f) {
        g_scrollTime = 0.f;
    }

    if (g_isEditingText) {
        g_editingTextRepeatTimer += dt;
    }

    if (g_isAdjustingSlider) {
        auto cast = geode::cast::typeinfo_cast<SliderThumb*>(g_button.data());
        if (!cast) {
            geode::log::warn("Was editing slider but not focused on a SliderThumb!");
            g_isAdjustingSlider = false;
            return;
        }

        auto slider = static_cast<Slider*>(cast->getParent()->getParent());
        float newValue = cast->getValue() + g_sliderNextFrame * dt;
        newValue = std::max(0.f, std::min(newValue, 1.f));
        slider->setValue(newValue);
        slider->m_touchLogic->m_thumb->activate(); // update value
        g_sliderNextFrame = 0.f;
    }

    if (cocos2d::CCDirector::get()->getIsTransitioning()) {
        g_transitionPercentage += dt;
    } else {
        g_transitionPercentage = 0.f;
    }
}
