#include "enki/widgets/file_drop_zone.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/dnd.hpp"
#include <layout_engine/Anu.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathEffect.h>
#include <include/effects/SkDashPathEffect.h>
#include <include/core/SkRRect.h>
#include <cctype>
#include <iostream>
#include <sstream>

namespace enki {

class RenderFileDropZone : public RenderBox {
public:
    FileDropZoneProps props_;
    bool is_drag_over_{false};
    float dash_phase_{0.0f};
    std::unique_ptr<Ticker> anim_ticker_;

    SlotId drag_enter_conn_ = 0;
    SlotId drag_motion_conn_ = 0;
    SlotId drag_leave_conn_ = 0;
    SlotId drop_conn_ = 0;

    bool isAllowedFile(const std::string& path) const {
        if (props_.allowed_extensions.empty()) return true;
        for (const auto& ext : props_.allowed_extensions) {
            if (path.length() >= ext.length()) {
                std::string suffix = path.substr(path.length() - ext.length());
                bool match = true;
                for (size_t i = 0; i < ext.length(); ++i) {
                    if (std::tolower(static_cast<unsigned char>(suffix[i])) !=
                        std::tolower(static_cast<unsigned char>(ext[i]))) {
                        match = false;
                        break;
                    }
                }
                if (match) return true;
            }
        }
        return false;
    }

    explicit RenderFileDropZone(FileDropZoneProps props)
        : props_(std::move(props)) {
        applyStyle();
        anim_ticker_ = createTicker([this]() {
            dash_phase_ += 1.0f;
            if (dash_phase_ >= 28.0f) dash_phase_ -= 28.0f;
            markNeedsPaint();
        });

        if (Platform::instance()) {
            drag_enter_conn_ = Platform::instance()->onDragEnter().connect([this](DragEnterEvent& ev) {
                ev.accept(DragAction::Copy);
                is_drag_over_ = true;
                if (anim_ticker_) anim_ticker_->start();
                markNeedsPaint();
            });

            drag_motion_conn_ = Platform::instance()->onDragMotion().connect([this](DragMotionEvent& ev) {
                ev.accept(DragAction::Copy);
                if (!is_drag_over_) {
                    is_drag_over_ = true;
                    if (anim_ticker_) anim_ticker_->start();
                    markNeedsPaint();
                }
            });

            drag_leave_conn_ = Platform::instance()->onDragLeave().connect([this](const DragLeaveEvent&) {
                is_drag_over_ = false;
                if (anim_ticker_) anim_ticker_->stop();
                markNeedsPaint();
            });

            drop_conn_ = Platform::instance()->onDrop().connect([this](DropEvent& ev) {
                is_drag_over_ = false;
                if (anim_ticker_) anim_ticker_->stop();
                markNeedsPaint();

                if (ev.data) {
                    std::vector<std::string> valid_files;
                    auto uris = ev.data->readUris();
                    for (auto& u : uris) {
                        std::string path = u;
                        if (path.rfind("file://", 0) == 0) {
                            path = path.substr(7);
                        }
                        if (isAllowedFile(path)) {
                            valid_files.push_back(path);
                        }
                    }
                    if (valid_files.empty()) {
                        std::string txt = ev.data->readText();
                        if (!txt.empty() && isAllowedFile(txt)) {
                            valid_files.push_back(txt);
                        }
                    }
                    if (!valid_files.empty()) {
                        std::cout << ">>> [FileDropZone] Native DnD Received " << valid_files.size() << " files:" << std::endl;
                        for (const auto& vf : valid_files) {
                            std::cout << "    • " << vf << std::endl;
                        }
                        if (props_.on_files_dropped) {
                            props_.on_files_dropped(valid_files);
                        }
                    }
                    ev.handled = true;
                }
            });
        }
    }

    ~RenderFileDropZone() override {
        if (anim_ticker_) {
            anim_ticker_->stop();
            anim_ticker_.reset();
        }
        if (Platform::instance()) {
            Platform::instance()->onDragEnter().disconnect(drag_enter_conn_);
            Platform::instance()->onDragMotion().disconnect(drag_motion_conn_);
            Platform::instance()->onDragLeave().disconnect(drag_leave_conn_);
            Platform::instance()->onDrop().disconnect(drop_conn_);
        }
    }

    void update(const FileDropZoneProps& new_props) {
        props_ = new_props;
        applyStyle();
        markNeedsPaint();
    }

    void applyStyle() {
        if (!anu_node_) return;
        ANUNodeStyleSetWidth(anu_node_, props_.width);
        ANUNodeStyleSetHeight(anu_node_, props_.height);
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0.0f && localPoint.x <= size_.width &&
               localPoint.y >= 0.0f && localPoint.y <= size_.height;
    }

    [[nodiscard]] SystemCursor cursor() const override {
        return SystemCursor::Pointer;
    }

    void handlePointerEnter(const PointerEvent&) override {
        is_drag_over_ = true;
        if (anim_ticker_) anim_ticker_->start();
        markNeedsPaint();
    }

    void handlePointerExit(const PointerEvent&) override {
        is_drag_over_ = false;
        if (anim_ticker_) anim_ticker_->stop();
        markNeedsPaint();
    }

    void handlePointerDown(const PointerEvent&) override {
        // Pure native DnD: Drag files from external file manager or desktop onto this zone.
    }

    void paint(PaintContext& ctx) override {
        auto& canvas = ctx.canvas;
        auto* sk = static_cast<SkCanvas*>(canvas.getNativeHandle());
        if (!sk) return;

        sk->save();
        sk->translate(ctx.offset.x, ctx.offset.y);

        Rect bounds{0.0f, 0.0f, size_.width, size_.height};

        // 1. Background Fill
        Paint bg_paint;
        bg_paint.setStyle(PaintStyle::Fill);
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(is_drag_over_ ? props_.hover_background : props_.idle_background);
        canvas.drawRRect(bounds, BorderRadius::circular(props_.border_radius), bg_paint);

        // 2. Dashed Border
        SkPaint border_paint;
        border_paint.setStyle(SkPaint::kStroke_Style);
        border_paint.setStrokeWidth(is_drag_over_ ? 2.0f : 1.5f);
        border_paint.setAntiAlias(true);
        border_paint.setColor(is_drag_over_ ? props_.hover_border_color : props_.idle_border_color);

        const SkScalar intervals[] = {8.0f, 6.0f};
        border_paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, dash_phase_));

        SkRect sk_r = SkRect::MakeXYWH(1.0f, 1.0f, size_.width - 2.0f, size_.height - 2.0f);
        SkRRect sk_rr = SkRRect::MakeRectXY(sk_r, props_.border_radius, props_.border_radius);
        sk->drawRRect(sk_rr, border_paint);

        // 3. Center Icon & Texts
        float cx = size_.width * 0.5f;
        float cy = size_.height * 0.5f - 18.0f;

        // Icon
        Paint icon_paint;
        icon_paint.setAntiAlias(true);
        icon_paint.setColor(is_drag_over_ ? 0xFF00E5FF : 0xFF38BDF8);
        canvas.drawText(props_.icon, Point(cx - 14.0f, cy), icon_paint, 28.0f, nullptr, false);

        // Prompt
        Paint prompt_paint;
        prompt_paint.setAntiAlias(true);
        prompt_paint.setColor(props_.text_color);
        float approx_pw = static_cast<float>(props_.prompt_text.length()) * 7.0f;
        canvas.drawText(props_.prompt_text, Point(cx - approx_pw * 0.5f, cy + 30.0f), prompt_paint, 13.0f, nullptr, false);

        // Subtext / Extensions
        std::string sub = props_.sub_text;
        if (!props_.allowed_extensions.empty()) {
            sub += " (";
            for (size_t i = 0; i < props_.allowed_extensions.size(); ++i) {
                if (i > 0) sub += ", ";
                sub += props_.allowed_extensions[i];
            }
            sub += ")";
        }

        Paint sub_paint;
        sub_paint.setAntiAlias(true);
        sub_paint.setColor(0xFF94A3B8);
        float approx_sw = static_cast<float>(sub.length()) * 5.8f;
        canvas.drawText(sub, Point(cx - approx_sw * 0.5f, cy + 48.0f), sub_paint, 11.0f, nullptr, false);

        sk->restore();
    }
};

FileDropZoneProps::operator WidgetPtr() const {
    return std::make_shared<FileDropZoneWidget>(*this);
}

std::unique_ptr<RenderObject> FileDropZoneWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderFileDropZone>(props);
}

void FileDropZoneWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderFileDropZone&>(renderObject);
    r.update(props);
}

} // namespace enki
