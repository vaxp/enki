#pragma once
/// @file gallery_i18n.hpp
/// @brief Declarative Internationalization setup for ENKI Gallery application.

#include "enki/i18n/i18n.hpp"
#include "enki/i18n/locale.hpp"

namespace enki::gallery {

inline void initGalleryTranslations() {
    I18n::define(I18nConfig{
        .default_locale = Locale("en", "US"),
        .fallback_locale = Locale("en", "US"),
        .translations = {
            { "en", {
                { "gallery.title",           "Device Gallery" },
                { "gallery.window_title",    "ENKI Gallery • Real Photo Storage" },
                { "gallery.subtitle",        "Loaded: {count} files from storage" },
                { "gallery.rescan",          "Rescan Storage" },
                { "gallery.lang_switch",     "عربي" },
                { "folder.all",              "All" },
                { "folder.favorites",        "Favorites" },
                { "folder.pictures",         "Pictures" },
                { "folder.downloads",        "Downloads" },
                { "folder.images",           "Images" },
                { "folder.screenshots",      "Screenshots" },
                { "folder.camera",           "Camera" },
                { "folder.storage",          "Storage" },
                { "card.fav",                "FAV" },
                { "card.saved",              "FAV" },
                { "permission.granted",      "Storage Permission: GRANTED" },
                { "permission.denied",       "Storage Permission: PERMANENTLY DENIED" },
                { "permission.required",     "Storage Permission: REQUIRED" },
                { "permission.desc_granted", "Read access to local media library is active." },
                { "permission.desc_denied",  "Storage permission was permanently denied. Please enable it in system settings." },
                { "permission.desc_required","Enki Gallery requires storage access to display your local photos." },
                { "permission.open_settings", "Open App Settings" },
                { "permission.request_access", "Request Storage Access" },
                { "grid.locked_title",       "Storage Access Required" },
                { "grid.locked_desc",        "Tap 'Request Storage Access' above to scan photos on this device." },
                { "grid.empty_title",        "No photos found in this directory." },
                { "viewer.back",             "< Back" },
                { "viewer.prev",             "< Prev" },
                { "viewer.next",             "Next >" },
                { "viewer.favorite",         "FAVORITE" },
                { "viewer.saved",            "SAVED" },
            }},
            { "ar", {
                { "gallery.title",           "معرض الصور" },
                { "gallery.window_title",    "معرض صور إنكي • تخزين حقيقي للصور" },
                { "gallery.subtitle",        "تم تحميل {count} من الذاكرة" },
                { "gallery.rescan",          "إعادة فحص الذاكرة" },
                { "gallery.lang_switch",     "English" },
                { "folder.all",              "الكل" },
                { "folder.favorites",        "المفضلة" },
                { "folder.pictures",         "الصور" },
                { "folder.downloads",        "التنزيلات" },
                { "folder.images",           "صور" },
                { "folder.screenshots",      "لقطات الشاشة" },
                { "folder.camera",           "الكاميرا" },
                { "folder.storage",          "الذاكرة" },
                { "card.fav",                "مفضلة" },
                { "card.saved",              "مفضلة" },
                { "permission.granted",      "إذن التخزين: مفعّل" },
                { "permission.denied",       "إذن التخزين: مرفوض نهائياً" },
                { "permission.required",     "إذن التخزين: مطلوب" },
                { "permission.desc_granted", "الوصول إلى مكتبة الوسائط المحلية مفعّل وجاهز." },
                { "permission.desc_denied",  "تم رفض إذن التخزين بشكل دائم. يرجى تفعيله من إعدادات النظام." },
                { "permission.desc_required","يتطلب معرض صور إنكي الوصول للتخزين لعرض صورك المحلية." },
                { "permission.open_settings", "فتح إعدادات التطبيق" },
                { "permission.request_access", "طلب إذن الوصول للتخزين" },
                { "grid.locked_title",       "الوصول إلى الذاكرة مطلوب" },
                { "grid.locked_desc",        "اضغط على 'طلب إذن الوصول للتخزين' أعلاه لفحص الصور على هذا الجهاز." },
                { "grid.empty_title",        "لم يتم العثور على صور في هذا المجلد." },
                { "viewer.back",             "رجوع >" },
                { "viewer.prev",             "السابق" },
                { "viewer.next",             "التالي" },
                { "viewer.favorite",         "إعجاب" },
                { "viewer.saved",            "محفوظة" },
            }}
        },
        .plurals = {
            { "en", {
                { "photos_count", PluralForms{
                    .one   = "1 photo",
                    .other = "{count} photos"
                }}
            }},
            { "ar", {
                { "photos_count", PluralForms{
                    .zero  = "لا توجد صور",
                    .one   = "صورة واحدة",
                    .two   = "صورتان",
                    .few   = "{count} صور",
                    .many  = "{count} صورة",
                    .other = "{count} صورة"
                }}
            }}
        }
    });
}

inline std::string localizeFolderName(std::string_view folder) {
    if (folder == "All") return std::string(tr("folder.all"));
    if (folder == "Favorites") return std::string(tr("folder.favorites"));
    if (folder == "Pictures") return std::string(tr("folder.pictures"));
    if (folder == "Downloads") return std::string(tr("folder.downloads"));
    if (folder == "imges" || folder == "images" || folder == "Images") return std::string(tr("folder.images"));
    if (folder == "Screenshots") return std::string(tr("folder.screenshots"));
    if (folder == "Camera" || folder == "Camera Roll") return std::string(tr("folder.camera"));
    if (folder == "Storage") return std::string(tr("folder.storage"));
    return std::string(folder);
}

} // namespace enki::gallery
