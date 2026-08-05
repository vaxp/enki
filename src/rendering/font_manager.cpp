#include "enki/rendering/font_manager.hpp"
#include <include/core/SkFontMgr.h>
#include <include/core/SkTypeface.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/TypefaceFontProvider.h>

namespace enki {

namespace {
    static sk_sp<skia::textlayout::TypefaceFontProvider> g_custom_font_provider = nullptr;
    static sk_sp<skia::textlayout::FontCollection> g_font_collection = nullptr;

    void initFontCollection() {
        if (!g_font_collection) {
            g_font_collection = sk_make_sp<skia::textlayout::FontCollection>();
            g_font_collection->setDefaultFontManager(SkFontMgr::RefDefault());
            
            g_custom_font_provider = sk_make_sp<skia::textlayout::TypefaceFontProvider>();
            g_font_collection->setAssetFontManager(g_custom_font_provider);
            
            g_font_collection->enableFontFallback();
        }
    }
}

bool FontManager::loadFont(const std::string& path, const std::string& alias) {
    initFontCollection();
    sk_sp<SkTypeface> typeface = SkTypeface::MakeFromFile(path.c_str());
    if (!typeface) {
        return false;
    }
    g_custom_font_provider->registerTypeface(std::move(typeface), SkString(alias.c_str()));
    return true;
}

sk_sp<skia::textlayout::FontCollection> FontManager::getFontCollection() {
    initFontCollection();
    return g_font_collection;
}

} // namespace enki
