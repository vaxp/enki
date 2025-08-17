/// @file file_picker.cpp
/// @brief Advanced Native FilePicker implementation built on NativePopup.

#include "enki/widgets/file_picker.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <chrono>

namespace fs = std::filesystem;

namespace enki {

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for FilePicker Dialog Frame
// ════════════════════════════════════════════════════════════════

class RenderFilePickerBackground : public RenderBox {
public:
    FilePickerProps props;

    explicit RenderFilePickerBackground(FilePickerProps opt)
        : props(std::move(opt)) {}

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        SkRect rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(rect, props.border_radius, props.border_radius);

        // 1. Draw Drop Shadow
        if (props.elevation > 0.0f) {
            SkPaint shadow_paint;
            shadow_paint.setAntiAlias(true);
            shadow_paint.setColor(props.shadow_color);
            shadow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, props.elevation * 0.5f));

            canvas->save();
            canvas->translate(0, props.elevation * 0.3f);
            canvas->drawRRect(rrect, shadow_paint);
            canvas->restore();
        }

        // 2. Draw Main Dialog Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(props.background_color);
        canvas->drawRRect(rrect, bg_paint);

        // 3. Draw Outer Border Stroke
        SkPaint border_paint;
        border_paint.setAntiAlias(true);
        border_paint.setStyle(SkPaint::kStroke_Style);
        border_paint.setStrokeWidth(1.0f);
        border_paint.setColor(props.border_color);
        canvas->drawRRect(rrect, border_paint);

        // 4. Paint Child Content
        if (!children().empty()) {
            RenderBox* child = static_cast<RenderBox*>(children()[0]);
            PaintContext child_ctx = ctx.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }
};

class FilePickerBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    FilePickerProps props;

    FilePickerBackgroundWidget(FilePickerProps opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          props(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderFilePickerBackground>(props);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderFilePickerBackground*>(&renderObject)) {
            rb->props = props;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "FilePickerBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// FilePicker Content View & Filesystem Logic
// ════════════════════════════════════════════════════════════════

struct FileEntryItem {
    std::string name;
    std::string path;
    bool is_directory;
    uint64_t size_bytes;
};

class FilePickerContentView : public StatefulWidget {
public:
    FilePickerProps props;
    std::function<void(const FilePickerResult&)> on_result;
    std::shared_ptr<NativePopup> popup_handle;

    FilePickerContentView(FilePickerProps props_,
                          std::function<void(const FilePickerResult&)> on_result,
                          std::shared_ptr<NativePopup> popup_handle)
        : props(std::move(props_)), on_result(std::move(on_result)), popup_handle(std::move(popup_handle)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FilePickerContentView"; }
};

class FilePickerContentViewState : public State {
private:
    fs::path current_path_;
    std::string selected_path_;
    std::vector<FileEntryItem> entries_;

public:
    void initState() override {
        State::initState();

        auto* view = static_cast<const FilePickerContentView*>(widget());
        if (view && !view->props.initial_directory.empty() && fs::exists(view->props.initial_directory)) {
            current_path_ = view->props.initial_directory;
        } else {
            const char* home = std::getenv("HOME");
            current_path_ = home ? fs::path(home) : fs::current_path();
        }

        refreshDirectory();
    }

    void refreshDirectory() {
        entries_.clear();
        selected_path_.clear();

        try {
            if (current_path_.has_parent_path() && current_path_ != current_path_.root_path()) {
                entries_.push_back({"..", current_path_.parent_path().string(), true, 0});
            }

            for (const auto& entry : fs::directory_iterator(current_path_, fs::directory_options::skip_permission_denied)) {
                std::string filename = entry.path().filename().string();
                if (filename.empty() || filename[0] == '.') continue; // Skip hidden files

                bool is_dir = entry.is_directory();
                uint64_t fsz = is_dir ? 0 : entry.file_size();

                entries_.push_back({filename, entry.path().string(), is_dir, fsz});
            }

            // Sort: Directories first, then files alphabetically
            std::sort(entries_.begin(), entries_.end(), [](const FileEntryItem& a, const FileEntryItem& b) {
                if (a.name == "..") return true;
                if (b.name == "..") return false;
                if (a.is_directory != b.is_directory) return a.is_directory > b.is_directory;
                return a.name < b.name;
            });
        } catch (const std::exception& ex) {
            std::cerr << "[ENKI FilePicker] Error reading directory: " << ex.what() << "\n";
        }
    }

    void navigateTo(const std::string& path_str) {
        fs::path target(path_str);
        if (fs::exists(target) && fs::is_directory(target)) {
            current_path_ = fs::canonical(target);
            setState([this]() {
                refreshDirectory();
            });
        }
    }

    WidgetPtr buildQuickLocationButton(const std::string& label, const std::string& target_path) {
        auto txt = text(label);
        txt->fontSize(13.0f).color(0xFFCBD5E1);

        auto btn_w = container(txt);
        btn_w->paddingSymmetric(6.0f, 10.0f)
             .color(0x00000000)
             .borderRadius(6.0f);

        auto gesture = gestureDetector(btn_w);
        gesture->hit_test_behavior = HitTestBehavior::Translucent;
        gesture->onTapUp([this, target_path](const TapUpDetails&) {
            navigateTo(target_path);
        });

        return gesture;
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* view = static_cast<const FilePickerContentView*>(widget());
        if (!view) return nullptr;

        const auto& opt = view->props;
        const char* home_env = std::getenv("HOME");
        std::string home_dir = home_env ? home_env : "/";

        // 1. Quick Access Sidebar
        std::vector<WidgetPtr> sidebar_items = {
            buildQuickLocationButton("🏠 Home", home_dir),
            buildQuickLocationButton("📁 Documents", home_dir + "/Documents"),
            buildQuickLocationButton("📥 Downloads", home_dir + "/Downloads"),
            buildQuickLocationButton("🖥️ Desktop", home_dir + "/Desktop"),
            buildQuickLocationButton("🖼️ Pictures", home_dir + "/Pictures"),
            buildQuickLocationButton("💽 Root (/)", "/")
        };

        auto sidebar_col = column(sidebar_items);
        sidebar_col->gap(StyleValue::point(4.0f));

        auto sidebar = container(sidebar_col);
        sidebar->color(opt.sidebar_color)
               .paddingAll(12.0f)
               .width(160.0f);

        // 2. Header Bar (Path Breadcrumb & Mode title)
        std::string mode_title = "Open File";
        if (opt.mode == FilePickerMode::OpenMultipleFiles) mode_title = "Open Files";
        else if (opt.mode == FilePickerMode::SelectFolder) mode_title = "Select Folder";
        else if (opt.mode == FilePickerMode::SaveFile) mode_title = "Save File";

        auto header_title = text(mode_title);
        header_title->fontSize(16.0f).bold().color(opt.text_color);

        auto current_path_txt = text(current_path_.string());
        current_path_txt->fontSize(12.0f).color(opt.subtext_color);

        auto header_col = column({header_title, current_path_txt});
        header_col->gap(StyleValue::point(2.0f));

        auto header_box = container(header_col);
        header_box->paddingSymmetric(8.0f, 12.0f)
                  .margin(StyleInsets::only(0, 0, 8.0f, 0));

        // 3. File Entries List View
        std::vector<WidgetPtr> entry_widgets;
        entry_widgets.reserve(entries_.size());

        for (const auto& entry : entries_) {
            std::string icon_str = entry.is_directory ? "📁 " : "📄 ";
            if (entry.name == "..") icon_str = "⬆️ ";

            auto entry_lbl = text(icon_str + entry.name);
            entry_lbl->fontSize(13.0f).color(opt.text_color);

            Color bg_col = (selected_path_ == entry.path) ? 0xFF1E3A8A : 0x00000000;

            auto entry_box = container(entry_lbl);
            entry_box->paddingSymmetric(6.0f, 10.0f)
                     .color(bg_col)
                     .borderRadius(4.0f);

            auto gesture = gestureDetector(entry_box);
            gesture->hit_test_behavior = HitTestBehavior::Translucent;

            gesture->onTapUp([this, entry](const TapUpDetails&) {
                if (entry.is_directory) {
                    navigateTo(entry.path);
                } else {
                    setState([this, entry]() {
                        selected_path_ = entry.path;
                    });
                }
            });

            entry_widgets.push_back(gesture);
        }

        auto files_col = column(std::vector<WidgetPtr>(entry_widgets.begin(), entry_widgets.end()));
        files_col->gap(StyleValue::point(2.0f));

        ScrollOptions s_opt;
        s_opt.show_scrollbar = true;

        float content_h = opt.window_size.height - 110.0f;
        if (content_h < 200.0f) content_h = 200.0f;

        auto scroll_view = scrollView(s_opt, files_col);
        auto files_scroll = container(scroll_view);
        files_scroll->height(content_h)
                    .paddingAll(4.0f);

        // 4. Bottom Action Bar (Cancel & Confirm buttons)
        auto cancel_lbl = text("Cancel");
        cancel_lbl->fontSize(13.0f).color(opt.text_color);
        auto cancel_btn = button(cancel_lbl, [view]() {
            if (view->on_result) {
                view->on_result({true, {}});
            }
            if (view->popup_handle) {
                view->popup_handle->close();
            }
        });

        std::string confirm_label = "Open";
        if (opt.mode == FilePickerMode::SaveFile) confirm_label = "Save";
        else if (opt.mode == FilePickerMode::SelectFolder) confirm_label = "Select Folder";

        ButtonProps confirm_opt;
        confirm_opt.normal_color = opt.accent_color;

        auto confirm_lbl = text(confirm_label);
        confirm_lbl->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto confirm_btn = button(confirm_lbl, [this, view]() {
            std::string final_path = selected_path_;
            if (final_path.empty() && (view->props.mode == FilePickerMode::SelectFolder || view->props.mode == FilePickerMode::SaveFile)) {
                final_path = current_path_.string();
            }

            if (view->on_result) {
                view->on_result({false, {final_path}});
            }
            if (view->popup_handle) {
                view->popup_handle->close();
            }
        }, confirm_opt);

        std::vector<WidgetPtr> actions_children;
        actions_children.push_back(cancel_btn);
        actions_children.push_back(confirm_btn);
        auto actions_row = row(actions_children);
        actions_row->justifyContent(Justify::End).gap(StyleValue::point(10.0f));

        auto actions_box = container(actions_row);
        actions_box->paddingAll(8.0f);

        // Right Main Content Assembly
        std::vector<WidgetPtr> right_col_children;
        right_col_children.push_back(header_box);
        right_col_children.push_back(files_scroll);
        right_col_children.push_back(actions_box);
        auto right_col = column(right_col_children);
        right_col->flexGrow(1.0f);

        std::vector<WidgetPtr> main_row_children;
        main_row_children.push_back(sidebar);
        main_row_children.push_back(right_col);
        auto main_row = row(main_row_children);
        main_row->flexGrow(1.0f);

        return main_row;
    }
};

std::unique_ptr<State> FilePickerContentView::createState() {
    return std::make_unique<FilePickerContentViewState>();
}

// ════════════════════════════════════════════════════════════════
// FilePicker State & Static Launcher
// ════════════════════════════════════════════════════════════════

std::shared_ptr<NativePopup> FilePicker::show(
    BuildContext& context,
    FilePickerProps props) {

    Element* elem = context.element();
    if (!elem) return nullptr;

    Size screen_sz = context.mediaSize();

    int32_t pop_w = static_cast<int32_t>(props.window_size.width);
    int32_t pop_h = static_cast<int32_t>(props.window_size.height);

    float pop_x = (screen_sz.width - pop_w) / 2.0f;
    float pop_y = (screen_sz.height - pop_h) / 2.0f;

    if (pop_x < 10.0f) pop_x = 10.0f;
    if (pop_y < 10.0f) pop_y = 10.0f;

    PopupOptions pop_opts;
    pop_opts.position = {pop_x, pop_y};
    pop_opts.width = pop_w;
    pop_opts.height = pop_h;
    pop_opts.auto_dismiss = false; // Modal dialog

    return NativePopup::show(context, pop_opts, [props](BuildContext&, std::shared_ptr<NativePopup> popup) {
        auto content = std::make_shared<FilePickerContentView>(props, props.on_result, popup);
        return std::make_shared<FilePickerBackgroundWidget>(props, content);
    });
}

class FilePickerState : public State {
private:
    std::shared_ptr<NativePopup> active_popup_ = nullptr;

public:
    void dispose() override {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* picker_widget = static_cast<const FilePicker*>(widget());

        auto gesture = gestureDetector(picker_widget->props.child);
        gesture->hit_test_behavior = HitTestBehavior::Translucent;

        gesture->onTapUp([this, picker_widget](const TapUpDetails&) {
            Element* elem = element();
            if (!elem) return;
            BuildContext context(elem);

            active_popup_ = FilePicker::show(context, picker_widget->props);
        });

        return gesture;
    }
};

std::unique_ptr<State> FilePicker::createState() {
    return std::make_unique<FilePickerState>();
}

} // namespace enki
