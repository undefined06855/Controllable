#include "CCScheduler.hpp"
#include "../globals.hpp"

void HookedCCScheduler::update(float dt) {
    CCScheduler::update(dt);

    if (g_scrollNextFrame != 0.f) {
        auto mouseDispatcher = cocos2d::CCDirector::get()->getMouseDispatcher();
        float scrollSpeed = 25.f * std::pow(g_scrollTime, 6.f) + 380.f;
        mouseDispatcher->dispatchScrollMSG(g_scrollNextFrame * dt * scrollSpeed, 0.f);
        g_scrollTime += dt;
    }

    // TODO: g_scrollTime does not get reset if you switch scrolling directions
    // mid-scroll
    if (g_scrollNextFrame == 0.f) {
        g_scrollTime = 0.f;
    }

    if (g_isEditingText) {
        g_editingTextRepeatTimer += dt;
    }

    if (g_isAdjustingSlider) {
        auto cast = geode::cast::typeinfo_cast<SliderThumb*>(g_button.data());
        if (!cast) {
            geode::log::warn("was editing slider but not focused on a sliderthumb!");
            g_isAdjustingSlider = false;
            return; // just in case
        }

        auto slider = static_cast<Slider*>(cast->getParent()->getParent());
        float newValue = cast->getValue() + g_sliderNextFrame * dt;
        newValue = std::max(0.f, std::min(newValue, 1.f));
        slider->setValue(newValue);
        slider->m_touchLogic->m_thumb->activate(); // update value
        g_sliderNextFrame = 0.f;
    }
}
