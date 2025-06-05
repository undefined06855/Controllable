$on_mod(Loaded) {
    auto path = geode::Mod::get()->getResourcesDir();
    cocos2d::CCFileUtils::get()->addTexturePack({
        .m_id = "sliderfix"_spr,
        .m_paths = { path.string() }
    });
}
