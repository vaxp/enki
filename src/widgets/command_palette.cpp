/// @file command_palette.cpp
/// @brief Implementation of Advanced Keyboard-Driven Command Palette & Fuzzy Search Overlay Widget.

#include "enki/widgets/command_palette.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/utility.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderCommandPaletteScrim — Dark dismiss click-catcher overlay
// ════════════════════════════════════════════════════════════════

class RenderCommandPaletteScrim : public RenderBox {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;

    RenderCommandPaletteScrim(float a, Color c, std::function<void()> tap)
        : alpha(a), base_color(c), on_tap(std::move(tap)) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext& ctx) override {
        if (alpha <= 0.0f) return;
        float x = ctx.offset.x;
        float y = ctx.offset.y;
        float w = size_.width;
        float h = size_.height;

        uint8_t base_a = (base_color >> 24) & 0xFF;
        uint8_t eff_a  = static_cast<uint8_t>(base_a * std::clamp(alpha, 0.0f, 1.0f));
        Color col = (static_cast<uint32_t>(eff_a) << 24) | (base_color & 0x00FFFFFF);

        Paint p;
        p.setColor(col);
        ctx.canvas.drawRect(Rect{x, y, w, h}, p);
    }

    bool hitTestSelf(Point p) const override {
        return alpha > 0.0f && p.x >= 0 && p.x <= size_.width &&
               p.y >= 0 && p.y <= size_.height;
    }

    void setAlpha(float a) {
        if (std::abs(alpha - a) > 0.001f) {
            alpha = a;
            markNeedsPaint();
        }
    }

    void handlePointerDown(const PointerEvent&) override {
        if (alpha > 0.0f && on_tap) {
            on_tap();
        }
    }
};

class CommandPaletteScrimWidget : public SingleChildRenderObjectWidget {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;
    std::shared_ptr<RenderCommandPaletteScrim*> holder;

    CommandPaletteScrimWidget(float a, Color c, std::function<void()> tap,
                              std::shared_ptr<RenderCommandPaletteScrim*> h = nullptr)
        : SingleChildRenderObjectWidget(Key::none()),
          alpha(a), base_color(c), on_tap(std::move(tap)), holder(std::move(h)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderCommandPaletteScrim>(alpha, base_color, on_tap);
        if (holder) *holder = r.get();
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& s = static_cast<RenderCommandPaletteScrim&>(ro);
        if (holder) *holder = &s;
        s.alpha = alpha;
        s.base_color = base_color;
        s.on_tap = on_tap;
        s.markNeedsPaint();
    }

    void didUnmountRenderObject(RenderObject&) override {
        if (holder && *holder) *holder = nullptr;
    }

    [[nodiscard]] std::string_view typeName() const override {
        return "CommandPaletteScrim";
    }
};

// ════════════════════════════════════════════════════════════════
// RenderCommandItemRow — Zero-rebuild interactive row with GPU hover
// ════════════════════════════════════════════════════════════════

class RenderCommandItemRow : public RenderBox {
public:
    Color normal_bg = 0x00000000;
    Color hover_bg = 0x3338BDF8;
    Color selected_bg = 0xFF0284C7;
    float border_radius = 8.0f;
    bool is_selected = false;
    bool is_hovered = false;
    bool is_disabled = false;
    std::function<void()> on_tap;

    RenderCommandItemRow(Color norm, Color hov, Color sel, float rad,
                         bool sel_state, bool dis, std::function<void()> tap)
        : normal_bg(norm), hover_bg(hov), selected_bg(sel), border_radius(rad),
          is_selected(sel_state), is_disabled(dis), on_tap(std::move(tap)) {
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
    }

    void update(Color norm, Color hov, Color sel, float rad,
                bool sel_state, bool dis, std::function<void()> tap) {
        normal_bg = norm;
        hover_bg = hov;
        selected_bg = sel;
        border_radius = rad;
        is_disabled = dis;
        on_tap = std::move(tap);
        if (is_selected != sel_state) {
            is_selected = sel_state;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        Color bg = is_selected ? selected_bg : (is_hovered ? hover_bg : normal_bg);
        if ((bg >> 24) != 0) {
            Paint p;
            p.setColor(bg);
            p.setAntiAlias(true);
            Rect r{ctx.offset.x, ctx.offset.y, size_.width, size_.height};
            ctx.canvas.drawRRect(r, BorderRadius::circular(border_radius), p);
        }

        if (!children().empty() && children()[0]) {
            auto* child_box = static_cast<RenderBox*>(children()[0]);
            PaintContext child_ctx = ctx.withOffset(child_box->offset());
            child_box->paint(child_ctx);
        }
    }

    bool hitTestSelf(Point localPoint) const override {
        return !is_disabled && localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    [[nodiscard]] SystemCursor cursor() const override {
        return is_disabled ? SystemCursor::Default : SystemCursor::Pointer;
    }

    void handlePointerEnter(const PointerEvent&) override {
        if (!is_disabled && !is_hovered) {
            is_hovered = true;
            markNeedsPaint();
        }
    }

    void handlePointerExit(const PointerEvent&) override {
        if (is_hovered) {
            is_hovered = false;
            markNeedsPaint();
        }
    }

    void handlePointerDown(const PointerEvent&) override {
        if (!is_disabled && on_tap) {
            on_tap();
        }
    }
};

class CommandItemRowWidget : public SingleChildRenderObjectWidget {
public:
    Color normal_bg;
    Color hover_bg;
    Color selected_bg;
    float border_radius;
    bool is_selected;
    bool is_disabled;
    std::function<void()> on_tap;

    CommandItemRowWidget(WidgetPtr child, Color norm, Color hov, Color sel,
                         float rad, bool sel_state, bool dis, std::function<void()> tap)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          normal_bg(norm), hover_bg(hov), selected_bg(sel), border_radius(rad),
          is_selected(sel_state), is_disabled(dis), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderCommandItemRow>(
            normal_bg, hover_bg, selected_bg, border_radius, is_selected, is_disabled, on_tap
        );
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderCommandItemRow&>(ro);
        r.update(normal_bg, hover_bg, selected_bg, border_radius, is_selected, is_disabled, on_tap);
    }

    [[nodiscard]] std::string_view typeName() const override {
        return "CommandItemRow";
    }
};

// ════════════════════════════════════════════════════════════════
// High-Performance Fuzzy Search & Scoring Algorithm
// ════════════════════════════════════════════════════════════════

struct FuzzyMatchResult {
    bool matched = false;
    int score = 0;
    std::vector<size_t> matched_indices; // Character positions in title
};

static std::string toLowerString(std::string_view s) {
    std::string res;
    res.reserve(s.size());
    for (char c : s) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return res;
}

static FuzzyMatchResult fuzzyMatchItem(std::string_view pattern, std::string_view text,
                                       std::string_view subtitle, const std::vector<std::string>& keywords) {
    if (pattern.empty()) {
        return {true, 0, {}};
    }

    std::string pat_lower = toLowerString(pattern);
    std::string text_lower = toLowerString(text);
    std::string sub_lower = toLowerString(subtitle);

    // 1. Exact string match
    if (text_lower == pat_lower) {
        std::vector<size_t> indices(text.size());
        for (size_t i = 0; i < text.size(); ++i) indices[i] = i;
        return {true, 10000, std::move(indices)};
    }

    // 2. Prefix match
    if (text_lower.starts_with(pat_lower)) {
        std::vector<size_t> indices(pattern.size());
        for (size_t i = 0; i < pattern.size(); ++i) indices[i] = i;
        int score = 5000 - static_cast<int>(text.size() * 2);
        return {true, score, std::move(indices)};
    }

    // 3. Substring match
    size_t sub_pos = text_lower.find(pat_lower);
    if (sub_pos != std::string::npos) {
        std::vector<size_t> indices(pattern.size());
        for (size_t i = 0; i < pattern.size(); ++i) indices[i] = sub_pos + i;
        bool word_start = (sub_pos == 0 || text[sub_pos - 1] == ' ' || text[sub_pos - 1] == '_' || text[sub_pos - 1] == '-');
        int bonus = word_start ? 4000 : 2000;
        int score = bonus - static_cast<int>(sub_pos * 10 + text.size());
        return {true, score, std::move(indices)};
    }

    // 4. Subsequence fuzzy match
    size_t p_idx = 0;
    size_t last_match_idx = 0;
    int score = 0;
    std::vector<size_t> indices;
    indices.reserve(pattern.size());

    for (size_t t_idx = 0; t_idx < text.size() && p_idx < pattern.size(); ++t_idx) {
        if (text_lower[t_idx] == pat_lower[p_idx]) {
            indices.push_back(t_idx);
            int char_score = 100;

            // Word boundary or camelCase match bonus
            bool is_word_boundary = (t_idx == 0) ||
                                    (text[t_idx - 1] == ' ' || text[t_idx - 1] == '-' || text[t_idx - 1] == '_' || text[t_idx - 1] == '/') ||
                                    (std::isupper(static_cast<unsigned char>(text[t_idx])) && !std::isupper(static_cast<unsigned char>(text[t_idx - 1])));
            if (is_word_boundary) {
                char_score += 250;
            }

            // Consecutive match bonus
            if (p_idx > 0 && t_idx == last_match_idx + 1) {
                char_score += 150;
            } else if (p_idx > 0) {
                // Distance penalty
                int distance = static_cast<int>(t_idx - last_match_idx);
                char_score -= std::min(distance * 5, 80);
            }

            // Exact case bonus
            if (text[t_idx] == pattern[p_idx]) {
                char_score += 20;
            }

            score += char_score;
            last_match_idx = t_idx;
            p_idx++;
        }
    }

    if (p_idx == pattern.size()) {
        score -= static_cast<int>(text.size() * 2);
        return {true, score, std::move(indices)};
    }

    // 5. Check secondary match in subtitle or keywords
    bool secondary_matched = false;
    int secondary_score = 0;
    if (!sub_lower.empty()) {
        size_t spos = sub_lower.find(pat_lower);
        if (spos != std::string::npos) {
            secondary_score = 600 - static_cast<int>(spos * 5);
            secondary_matched = true;
        }
    }
    for (const auto& kw : keywords) {
        std::string kw_lower = toLowerString(kw);
        if (kw_lower.starts_with(pat_lower)) {
            secondary_score = std::max(secondary_score, 700);
            secondary_matched = true;
        } else if (kw_lower.find(pat_lower) != std::string::npos) {
            secondary_score = std::max(secondary_score, 400);
            secondary_matched = true;
        }
    }

    if (secondary_matched) {
        return {true, secondary_score, {}};
    }

    return {false, 0, {}};
}

// Internal match entry for sorting and rendering
struct MatchedCommandEntry {
    CommandItem item;
    int score = 0;
    std::vector<size_t> matched_indices;
    size_t selectable_index = 0;
    bool is_recent = false;
};

// ════════════════════════════════════════════════════════════════
// CommandPaletteState — High-Performance Zero-Rebuild Overlay
// ════════════════════════════════════════════════════════════════

class CommandPaletteState : public State {
private:
    bool is_open_ = false;
    std::string query_ = "";
    int active_index_ = 0;

    std::vector<CommandItem> library_items_;
    std::deque<std::string> recent_ids_;

    std::vector<MatchedCommandEntry> displayed_entries_;
    size_t total_selectable_ = 0;

    SlotId key_down_conn_ = 0;
    SlotId text_input_conn_ = 0;

    // Layer Caching: body layer is cached as a stable pointer to prevent any rebuild of main app
    WidgetPtr cached_raw_body_ = nullptr;
    WidgetPtr cached_body_widget_ = nullptr;

    // Overlay Card Caching: cached to prevent Skia/Flexbox reflows
    WidgetPtr cached_card_ = nullptr;
    WidgetPtr cached_pos_card_col_ = nullptr;
    WidgetPtr cached_footer_bar_ = nullptr;

    void invalidateCard() {
        cached_card_ = nullptr;
        cached_pos_card_col_ = nullptr;
    }

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const CommandPaletteWidget*>(widget());

        is_open_ = w->initial_open;
        library_items_ = w->items;

        wireController();
        updateFilteredItems();

        if (Platform::instance()) {
            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                handleKeyDown(key, mods);
            });

            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                handleTextInput(std::string(text));
            });
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        library_items_ = w->items;
        wireController();
        updateFilteredItems();
        invalidateCard();
    }

    void dispose() override {
        if (Platform::instance()) {
            if (key_down_conn_) Platform::instance()->onKeyDown().disconnect(key_down_conn_);
            if (text_input_conn_) Platform::instance()->onTextInput().disconnect(text_input_conn_);
        }
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        if (w->controller) {
            w->controller->open_fn = [this] { openPalette(); };
            w->controller->close_fn = [this] { closePalette(); };
            w->controller->toggle_fn = [this] { togglePalette(); };
            w->controller->is_open_fn = [this] { return is_open_; };
            w->controller->set_query_fn = [this](std::string q) {
                query_ = std::move(q);
                active_index_ = 0;
                updateFilteredItems();
                invalidateCard();
                setState([] {});
            };
            w->controller->get_query_fn = [this] { return query_; };
            w->controller->set_items_fn = [this](std::vector<CommandItem> items) {
                library_items_ = std::move(items);
                updateFilteredItems();
                invalidateCard();
                setState([] {});
            };
            w->controller->select_next_fn = [this] { selectNext(); };
            w->controller->select_prev_fn = [this] { selectPrevious(); };
            w->controller->execute_active_fn = [this] { executeActive(); };
            w->controller->clear_recent_fn = [this] {
                recent_ids_.clear();
                updateFilteredItems();
                invalidateCard();
                setState([] {});
            };
        }
    }

    void openPalette() {
        if (is_open_) return;
        is_open_ = true;
        active_index_ = 0;
        updateFilteredItems();
        invalidateCard();

        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        if (w->options.on_open) w->options.on_open();
        setState([] {});
    }

    void closePalette() {
        if (!is_open_) return;
        is_open_ = false;
        invalidateCard();

        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        if (w->options.on_close) w->options.on_close();
        setState([] {});
    }

    void togglePalette() {
        if (is_open_) closePalette();
        else openPalette();
    }

    void updateFilteredItems() {
        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        const auto& opts = w->options;

        displayed_entries_.clear();
        total_selectable_ = 0;

        if (query_.empty()) {
            // Mode 1: Empty Query — Display Recent commands first, then library items
            std::unordered_set<std::string> seen_recent_ids;
            if (opts.show_recent && !recent_ids_.empty()) {
                for (const auto& rec_id : recent_ids_) {
                    auto it = std::find_if(library_items_.begin(), library_items_.end(),
                                           [&](const CommandItem& item) { return item.id == rec_id; });
                    if (it != library_items_.end() && !it->disabled) {
                        MatchedCommandEntry entry;
                        entry.item = *it;
                        entry.item.category = "Recent";
                        entry.score = 1000;
                        entry.is_recent = true;
                        entry.selectable_index = total_selectable_++;
                        displayed_entries_.push_back(entry);
                        seen_recent_ids.insert(rec_id);
                        if (displayed_entries_.size() >= 5) break; // Cap recent section
                    }
                }
            }

            for (const auto& item : library_items_) {
                if (displayed_entries_.size() >= opts.max_results) break;
                if (seen_recent_ids.count(item.id)) continue;

                MatchedCommandEntry entry;
                entry.item = item;
                entry.score = 0;
                entry.is_recent = false;
                if (!item.disabled) {
                    entry.selectable_index = total_selectable_++;
                }
                displayed_entries_.push_back(entry);
            }
        } else {
            // Mode 2: Fuzzy matching query against library items
            std::vector<MatchedCommandEntry> matches;
            for (const auto& item : library_items_) {
                auto res = fuzzyMatchItem(query_, item.title, item.subtitle, item.keywords);
                if (res.matched) {
                    MatchedCommandEntry entry;
                    entry.item = item;
                    entry.score = res.score;
                    entry.matched_indices = std::move(res.matched_indices);
                    matches.push_back(std::move(entry));
                }
            }

            // Sort by score descending
            std::sort(matches.begin(), matches.end(), [](const MatchedCommandEntry& a, const MatchedCommandEntry& b) {
                return a.score > b.score;
            });

            size_t count = std::min(matches.size(), opts.max_results);
            for (size_t i = 0; i < count; ++i) {
                if (!matches[i].item.disabled) {
                    matches[i].selectable_index = total_selectable_++;
                }
                displayed_entries_.push_back(std::move(matches[i]));
            }
        }

        // Clamp active index
        if (total_selectable_ == 0) {
            active_index_ = 0;
        } else if (active_index_ >= static_cast<int>(total_selectable_)) {
            active_index_ = static_cast<int>(total_selectable_) - 1;
        } else if (active_index_ < 0) {
            active_index_ = 0;
        }
    }

    void selectNext() {
        if (total_selectable_ == 0) return;
        active_index_ = (active_index_ + 1) % static_cast<int>(total_selectable_);
        invalidateCard();
        setState([] {});
    }

    void selectPrevious() {
        if (total_selectable_ == 0) return;
        active_index_ = (active_index_ <= 0) ? static_cast<int>(total_selectable_) - 1 : active_index_ - 1;
        invalidateCard();
        setState([] {});
    }

    void selectFirst() {
        if (total_selectable_ == 0) return;
        active_index_ = 0;
        invalidateCard();
        setState([] {});
    }

    void selectLast() {
        if (total_selectable_ == 0) return;
        active_index_ = static_cast<int>(total_selectable_) - 1;
        invalidateCard();
        setState([] {});
    }

    void executeActive() {
        for (const auto& entry : displayed_entries_) {
            if (!entry.item.disabled && static_cast<int>(entry.selectable_index) == active_index_) {
                executeItem(entry.item);
                return;
            }
        }
    }

    void executeItem(const CommandItem& item) {
        if (item.disabled) return;

        // Track in recent LRU
        if (!item.id.empty()) {
            auto it = std::find(recent_ids_.begin(), recent_ids_.end(), item.id);
            if (it != recent_ids_.end()) recent_ids_.erase(it);
            recent_ids_.push_front(item.id);
            if (recent_ids_.size() > 10) recent_ids_.pop_back();
        }

        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        if (w->options.on_item_selected) {
            w->options.on_item_selected(item);
        }
        if (item.on_execute) {
            item.on_execute();
        }

        if (w->options.auto_close_on_select) {
            closePalette();
        }
    }

    void handleKeyDown(int key, int mods) {
        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        bool ctrl = (mods & 2) != 0 || (mods & 4) != 0; // Ctrl or Meta/Super

        // Global Shortcut: Ctrl+K / Cmd+K toggle
        if (w->options.enable_global_shortcut && ctrl &&
            (key == 'k' || key == 'K' || key == 0x6b || key == 0x4b || key == 45)) {
            togglePalette();
            return;
        }

        if (!is_open_) return;

        const int KEY_BACKSPACE = 0xff08;
        const int KEY_DELETE    = 0xffff;
        const int KEY_UP        = 0xff52;
        const int KEY_DOWN      = 0xff54;
        const int KEY_HOME      = 0xff50;
        const int KEY_END       = 0xff57;
        const int KEY_RETURN    = 0xff0d;
        const int KEY_ESCAPE    = 0xff1b;

        if (key == KEY_ESCAPE) {
            closePalette();
        } else if (key == KEY_DOWN) {
            selectNext();
        } else if (key == KEY_UP) {
            selectPrevious();
        } else if (key == KEY_HOME) {
            selectFirst();
        } else if (key == KEY_END) {
            selectLast();
        } else if (key == KEY_RETURN || key == 0xff8d) {
            executeActive();
        } else if (key == KEY_BACKSPACE || key == 0x08 || key == 0x7f) {
            if (!query_.empty()) {
                // Delete one UTF-8 character backwards
                while (!query_.empty() && (static_cast<unsigned char>(query_.back()) & 0xC0) == 0x80) {
                    query_.pop_back();
                }
                if (!query_.empty()) query_.pop_back();

                active_index_ = 0;
                updateFilteredItems();
                invalidateCard();
                if (w->options.on_query_change) w->options.on_query_change(query_);
                setState([] {});
            }
        } else if (key == KEY_DELETE) {
            query_.clear();
            active_index_ = 0;
            updateFilteredItems();
            invalidateCard();
            if (w->options.on_query_change) w->options.on_query_change(query_);
            setState([] {});
        }
    }

    void handleTextInput(const std::string& text) {
        if (!is_open_ || text.empty()) return;
        // Ignore control characters
        if (static_cast<unsigned char>(text[0]) < 32 && text[0] != '\t') return;

        query_ += text;
        active_index_ = 0;
        updateFilteredItems();
        invalidateCard();

        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        if (w->options.on_query_change) w->options.on_query_change(query_);
        setState([] {});
    }

    // ── UI Builders ───────────────────────────────────────────

    WidgetPtr buildHighlightedTitle(const std::string& title, const std::vector<size_t>& matched_indices,
                                    Color normal_color, Color highlight_color) {
        if (matched_indices.empty()) {
            return text(title, {
                .color = normal_color,
                .font_size = 13.5f,
                .font_weight = FontWeight::Medium
            });
        }

        std::vector<bool> is_matched(title.size(), false);
        for (size_t idx : matched_indices) {
            if (idx < title.size()) is_matched[idx] = true;
        }

        std::vector<std::shared_ptr<InlineSpan>> spans;
        size_t start = 0;
        while (start < title.size()) {
            bool state = is_matched[start];
            size_t end = start + 1;
            while (end < title.size() && is_matched[end] == state) {
                end++;
            }
            TextStyle st{
                .color = state ? highlight_color : normal_color,
                .font_size = 13.5f,
                .font_weight = state ? FontWeight::Bold : FontWeight::Medium
            };
            spans.push_back(span(title.substr(start, end - start), st));
            start = end;
        }

        return richText(span("", std::nullopt, spans));
    }

    WidgetPtr buildItemRow(const MatchedCommandEntry& entry, const CommandPaletteOptions& opts) {
        const auto& item = entry.item;
        bool is_selected = !item.disabled && (static_cast<int>(entry.selectable_index) == active_index_);

        Color title_color = item.disabled ? 0xFF64748B
                          : (item.is_danger ? 0xFFEF4444 : (is_selected ? 0xFFFFFFFF : opts.item_title_color));
        Color subtitle_color = item.disabled ? 0xFF475569
                             : (is_selected ? 0xFFE2E8F0 : opts.item_subtitle_color);

        // 1. Left Icon Container
        WidgetPtr icon_box = container({
            .color = is_selected ? 0x33FFFFFF : 0x221E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .padding = StyleInsets::all(6.0f),
            .child = text(item.icon.empty() ? "⚡" : item.icon, {
                .color = item.is_danger ? 0xFFEF4444 : (is_selected ? 0xFFFFFFFF : 0xFF38BDF8),
                .font_size = 14.0f
            })
        });

        // 2. Title & Subtitle
        std::vector<WidgetPtr> text_children;
        text_children.push_back(buildHighlightedTitle(
            item.title, entry.matched_indices, title_color, is_selected ? 0xFFFFFFFF : opts.highlight_match_color
        ));

        if (!item.subtitle.empty()) {
            text_children.push_back(text(item.subtitle, {
                .color = subtitle_color,
                .font_size = 11.5f
            }));
        }

        auto text_col = column({
            .flex = 1.0f,
            .gap = StyleValue::point(2.0f),
            .children = text_children
        });

        // 3. Right Badges & Shortcut
        std::vector<WidgetPtr> right_elements;

        if (item.is_danger) {
            right_elements.push_back(container({
                .color = 0x33EF4444,
                .border_radius = BorderRadius::circular(4.0f),
                .border = Border(0x66EF4444, 1.0f),
                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                .child = text("Destructive", { .color = 0xFFEF4444, .font_size = 10.5f, .font_weight = FontWeight::SemiBold })
            }));
        } else if (!item.badge.empty()) {
            right_elements.push_back(container({
                .color = 0x2238BDF8,
                .border_radius = BorderRadius::circular(4.0f),
                .border = Border(0x5538BDF8, 1.0f),
                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                .child = text(item.badge, { .color = 0xFF38BDF8, .font_size = 10.5f, .font_weight = FontWeight::SemiBold })
            }));
        }

        if (!item.shortcut.empty()) {
            right_elements.push_back(container({
                .color = is_selected ? 0x22000000 : opts.shortcut_badge_bg,
                .border_radius = BorderRadius::circular(4.0f),
                .border = Border(is_selected ? 0x44FFFFFF : 0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                .child = text(item.shortcut, {
                    .color = is_selected ? 0xFFFFFFFF : opts.shortcut_text_color,
                    .font_size = 11.0f,
                    .font_weight = FontWeight::Medium
                })
            }));
        }

        auto row_content = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                icon_box,
                text_col,
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(6.0f),
                    .children = right_elements
                })
            }
        });

        auto row_padded = container({
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(7.0f, 10.0f),
            .child = row_content
        });

        if (item.disabled) {
            return row_padded;
        }

        return std::make_shared<CommandItemRowWidget>(
            row_padded, 0x00000000, opts.item_hover_bg, opts.item_selected_bg,
            8.0f, is_selected, item.disabled, [this, item] { executeItem(item); }
        );
    }

    WidgetPtr buildResultsList(const CommandPaletteOptions& opts) {
        if (displayed_entries_.empty()) {
            return container({
                .padding = StyleInsets::all(40.0f),
                .child = column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .width = StyleValue::percent(100.0f),
                    .children = {
                        text("🔍", { .font_size = 28.0f }),
                        text(opts.empty_text, {
                            .color = 0xFFF1F5F9,
                            .font_size = 14.5f,
                            .font_weight = FontWeight::SemiBold
                        }),
                        text("Try searching with different keywords or clear query", {
                            .color = 0xFF64748B,
                            .font_size = 12.0f
                        })
                    }
                })
            });
        }

        // Group entries by category
        std::vector<WidgetPtr> list_children;
        std::string current_cat = "";

        for (const auto& entry : displayed_entries_) {
            std::string cat = entry.item.category.empty() ? "Commands" : entry.item.category;
            if (cat != current_cat) {
                current_cat = cat;
                list_children.push_back(container({
                    .padding = StyleInsets::only(10.0f, 4.0f, 4.0f, 8.0f),
                    .child = text(toLowerString(current_cat) == "recent" ? "🕒 RECENT COMMANDS" : current_cat, {
                        .color = opts.section_header_color,
                        .font_size = 10.5f,
                        .font_weight = FontWeight::Bold
                    })
                }));
            }
            list_children.push_back(buildItemRow(entry, opts));
        }

        auto inner_col = column({
            .gap = StyleValue::point(2.0f),
            .width = StyleValue::percent(100.0f),
            .children = list_children
        });

        return container({
            .max_height = StyleValue::point(opts.max_list_height),
            .padding = StyleInsets::symmetric(6.0f, 10.0f),
            .child = ScrollView {
                .child = inner_col,
                .direction = Axis::Vertical,
                .show_scrollbar = true
            }
        });
    }

    WidgetPtr buildPaletteCard(const CommandPaletteOptions& opts) {
        // 1. Header Search Input Bar
        std::vector<WidgetPtr> search_row_items;
        search_row_items.push_back(text("🔍", {
            .color = opts.highlight_match_color,
            .font_size = 16.0f
        }));

        if (query_.empty()) {
            search_row_items.push_back(text(opts.placeholder, {
                .color = opts.placeholder_color,
                .font_size = 14.5f
            }));
        } else {
            search_row_items.push_back(row({
                .align_items = Align::Center,
                .children = {
                    text(query_, {
                        .color = opts.input_text_color,
                        .font_size = 14.5f,
                        .font_weight = FontWeight::Medium
                    }),
                    text("│", {
                        .color = opts.highlight_match_color,
                        .font_size = 14.5f
                    })
                }
            }));
        }

        auto search_left = row({
            .align_items = Align::Center,
            .flex = 1.0f,
            .gap = StyleValue::point(10.0f),
            .children = search_row_items
        });

        auto esc_pill = container({
            .color = 0x221E293B,
            .border_radius = BorderRadius::circular(4.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(2.0f, 6.0f),
            .child = text("ESC", {
                .color = 0xFF94A3B8,
                .font_size = 10.5f,
                .font_weight = FontWeight::Bold
            })
        });

        auto search_bar = container({
            .color = opts.input_bg_color,
            .border = Border(opts.border_color, 1.0f),
            .padding = StyleInsets::symmetric(14.0f, 16.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    search_left,
                    esc_pill
                }
            })
        });

        // 2. Results List
        auto results_list = buildResultsList(opts);

        // 3. Footer Bar with Shortcuts Cheatsheet
        if (!cached_footer_bar_) {
            cached_footer_bar_ = container({
                .color = 0xFF0B0F19,
                .border = Border(opts.border_color, 1.0f),
                .padding = StyleInsets::symmetric(8.0f, 14.0f),
                .child = row({
                    .justify_content = Justify::SpaceBetween,
                    .align_items = Align::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {
                        row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(12.0f),
                            .children = {
                                row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(4.0f),
                                    .children = {
                                        text("↑↓", { .color = 0xFF38BDF8, .font_size = 11.0f, .font_weight = FontWeight::Bold }),
                                        text("navigate", { .color = 0xFF64748B, .font_size = 11.0f })
                                    }
                                }),
                                row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(4.0f),
                                    .children = {
                                        text("↵", { .color = 0xFF38BDF8, .font_size = 11.0f, .font_weight = FontWeight::Bold }),
                                        text("select", { .color = 0xFF64748B, .font_size = 11.0f })
                                    }
                                }),
                                row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(4.0f),
                                    .children = {
                                        text("esc", { .color = 0xFF38BDF8, .font_size = 11.0f, .font_weight = FontWeight::Bold }),
                                        text("dismiss", { .color = 0xFF64748B, .font_size = 11.0f })
                                    }
                                })
                            }
                        }),
                        text("Enki Command Engine", {
                            .color = 0xFF475569,
                            .font_size = 10.5f,
                            .font_weight = FontWeight::Medium
                        })
                    }
                })
            });
        }

        // Outer Card Container with Elevation & Border
        return container({
            .color = opts.card_bg_color,
            .border_radius = BorderRadius::circular(opts.card_border_radius),
            .border = Border(opts.border_color, 1.0f),
            .box_shadow = {BoxShadow(0xCC000000, {0.0f, 16.0f}, 40.0f)},
            .width = StyleValue::point(opts.card_width),
            .child = column({
                .children = {
                    search_bar,
                    results_list,
                    cached_footer_bar_
                }
            })
        });
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const CommandPaletteWidget*>(widget());
        const auto& opts = w->options;

        // ── 1. Underlying Page Body (Invariant full-viewport layer) ──
        if (!cached_body_widget_ || cached_raw_body_ != w->body) {
            cached_raw_body_ = w->body;
            cached_body_widget_ = w->body ? Positioned::fill(w->body) : Positioned::fill(container({}));
        }

        // Palette completely closed: render only invariant body
        if (!is_open_) {
            return stack({
                .children = {cached_body_widget_},
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f)
            });
        }

        // ── 2. Scrim Backdrop Overlay (Dismiss on tap) ────────────────
        std::function<void()> tap_cb = nullptr;
        if (opts.barrier_dismissible) {
            tap_cb = [this] { closePalette(); };
        }
        auto scrim = std::make_shared<CommandPaletteScrimWidget>(
            1.0f, opts.overlay_color, tap_cb
        );

        // ── 3. Centered Command Palette Card ──────────────────────────
        if (!cached_card_) {
            cached_card_ = buildPaletteCard(opts);
            cached_pos_card_col_ = Positioned::fill(column({
                .align_items = Align::Center,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = {
                    container({
                        .margin = StyleInsets::only(opts.top_margin, 0.0f, 0.0f, 0.0f),
                        .child = cached_card_
                    })
                }
            }));
        }

        // ── 4. Stack Composition: Body + Scrim + Command Card ─────────
        return stack({
            .children = {
                cached_body_widget_,
                scrim,
                cached_pos_card_col_
            },
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f)
        });
    }
};

std::unique_ptr<State> CommandPaletteWidget::createState() {
    return std::make_unique<CommandPaletteState>();
}

} // namespace enki
