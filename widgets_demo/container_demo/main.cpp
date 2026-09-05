/// @file main.cpp
/// @brief ENKI Container Widget Interactive Showcase.
/// Demonstrates classic styling alongside advanced SkSL background_shader and border_shader injection,
/// including a procedural "Daggers & Blood" border shader and a 1:1 Pixel Art Riveted Frame shader.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <memory>

using namespace enki;

class ContainerDemoState : public State {
public:
    WidgetPtr build(BuildContext&) override {
        // Header
        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .margin = StyleInsets::only(0, 0, 24.0f, 0),
            .children = {
                text("Container SkSL Shader Injection", {
                    .color = 0xFFFFFFFF,
                    .font_size = 28.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Procedural background_shader & border_shader with Real-time GPU Evaluation", {
                    .color = 0xFF94A3B8,
                    .font_size = 14.0f,
                })
            }
        });

        // 1. Classic Container (Backwards Compatibility test)
        auto card1 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF334155, 1.5f),
            .box_shadow = { BoxShadow::standard(0x40000000, 10.0f, 4.0f) },
            .align = Alignment::Center,
            .width = StyleValue::point(260.0f),
            .height = StyleValue::point(160.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("Classic Container", { .color = 0xFF38BDF8, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                    text("Solid Color & Border", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                    text("100% Backwards Compatible", { .color = 0xFF10B981, .font_size = 11.0f })
                }
            })
        });

        // 2. Animated Plasma Background Shader
        const std::string plasma_shader = R"(
            uniform float time;
            uniform vec2 resolution;

            vec4 main(vec2 fragCoord) {
                vec2 uv = fragCoord / resolution;
                float t = time * 0.8;
                float v = sin(uv.x * 6.0 + t) + sin(uv.y * 6.0 + t) + sin((uv.x + uv.y) * 6.0 + t);
                v = v * 0.5;
                vec3 col = 0.5 + 0.5 * cos(v + vec3(0.0, 2.0, 4.0));
                return vec4(col * 0.85, 1.0);
            }
        )";

        auto card2 = container({
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0x60FFFFFF, 1.5f),
            .box_shadow = { BoxShadow::glow(0x608B5CF6, 20.0f) },
            .background_shader = plasma_shader,
            .align = Alignment::Center,
            .width = StyleValue::point(260.0f),
            .height = StyleValue::point(160.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("background_shader", { .color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                    text("Live Animated SkSL Plasma", { .color = 0xFFF1F5F9, .font_size = 13.0f }),
                    text("Auto-ticking 60 FPS", { .color = 0xFFFDE047, .font_size = 11.0f })
                }
            })
        });

        // 3. Cyber Glowing Border Shader
        const std::string cyber_border_shader = R"(
            uniform float time;
            uniform vec2 resolution;

            vec4 main(vec2 fragCoord) {
                vec2 uv = fragCoord / resolution;
                float angle = atan(uv.y - 0.5, uv.x - 0.5);
                float glow = sin(angle * 3.0 + time * 3.0) * 0.5 + 0.5;
                vec3 cyan = vec3(0.06, 0.72, 0.95);
                vec3 pink = vec3(0.93, 0.28, 0.60);
                vec3 color = mix(cyan, pink, glow);
                return vec4(color, 1.0);
            }
        )";

        auto card3 = container({
            .color = 0xE60F172A,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(Colors::Transparent, 3.0f),
            .box_shadow = { BoxShadow::glow(0x4006B6D4, 25.0f) },
            .border_shader = cyber_border_shader,
            .align = Alignment::Center,
            .width = StyleValue::point(260.0f),
            .height = StyleValue::point(160.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("border_shader (Neon)", { .color = 0xFF38BDF8, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                    text("Rotating Dual Gradient", { .color = 0xFFCBD5E1, .font_size = 13.0f }),
                    text("3px Injected SkSL Stroke", { .color = 0xFFF472B6, .font_size = 11.0f })
                }
            })
        });

        // ═════════════════════════════════════════════════════════════
        // 4. THE PROCEDURAL "DAGGERS & BLOOD" BORDER SHADER (خناجر ودماء)
        // ═════════════════════════════════════════════════════════════
        const std::string dagger_blood_border_shader = R"(
            uniform float time;
            uniform vec2 resolution;

            vec4 main(vec2 fragCoord) {
                float w = resolution.x;
                float h = resolution.y;

                float dLeft   = fragCoord.x;
                float dRight  = w - fragCoord.x;
                float dTop    = fragCoord.y;
                float dBottom = h - fragCoord.y;
                float minEdge = min(min(dLeft, dRight), min(dTop, dBottom));

                float s = 0.0;
                float v = 0.0;
                float bWidth = 16.0;

                if (minEdge == dTop) {
                    s = fragCoord.x;
                    v = clamp(fragCoord.y / bWidth, 0.0, 1.0);
                } else if (minEdge == dRight) {
                    s = w + fragCoord.y;
                    v = clamp((w - fragCoord.x) / bWidth, 0.0, 1.0);
                } else if (minEdge == dBottom) {
                    s = w + h + (w - fragCoord.x);
                    v = clamp((h - fragCoord.y) / bWidth, 0.0, 1.0);
                } else {
                    s = 2.0 * w + h + (h - fragCoord.y);
                    v = clamp(fragCoord.x / bWidth, 0.0, 1.0);
                }

                float cycleLen  = 45.0;
                float bloodGap  = 5.0;
                float daggerLen = cycleLen - bloodGap;

                float u = mod(s + time * 12.0, cycleLen);

                // ── 1. THE 5-PIXEL BLOOD GAP ──
                if (u < bloodGap) {
                    float uBlood = u / bloodGap;
                    float pulse = sin(time * 6.0 + s * 0.4) * 0.5 + 0.5;
                    float droplet = sin(uBlood * 3.14159) * (1.0 - abs(v - 0.5) * 1.5);
                    
                    vec3 deepCoagulated = vec3(0.32, 0.01, 0.02);
                    vec3 arterialFresh  = vec3(0.95, 0.05, 0.08);
                    vec3 fluidGlint     = vec3(1.0, 0.5, 0.55);
                    
                    float spec = pow(max(0.0, sin(uBlood * 3.14159 * 2.0 - v * 2.0 + time * 4.0)), 12.0);
                    vec3 blood = mix(deepCoagulated, arterialFresh, pulse * 0.6 + droplet * 0.4);
                    blood += fluidGlint * spec * 0.8;
                    
                    return vec4(blood, 1.0);
                }

                // ── 2. THE DAGGER BLADE ──
                float xDagger = (u - bloodGap) / daggerLen;
                float yDistCenter = abs(v - 0.5) * 2.0;

                float maxHalfW = (xDagger < 0.72) ? 1.0 : (1.0 - (xDagger - 0.72) / 0.28);
                if (yDistCenter > maxHalfW) {
                    float bloodFade = sin(time * 3.0 + s * 0.3) * 0.5 + 0.5;
                    vec3 seepBlood = mix(vec3(0.18, 0.01, 0.02), vec3(0.65, 0.02, 0.04), bloodFade);
                    return vec4(seepBlood, 0.95);
                }

                float damascus = sin(xDagger * 40.0 + sin(yDistCenter * 16.0) * 2.5) * 0.5 + 0.5;
                vec3 darkSteel  = vec3(0.20, 0.22, 0.28);
                vec3 lightSteel = vec3(0.72, 0.78, 0.88);
                vec3 razorEdge  = vec3(0.98, 1.0, 1.0);

                vec3 metal = mix(darkSteel, lightSteel, damascus);
                float edgeBevel = smoothstep(0.60, 1.0, yDistCenter / maxHalfW);
                metal = mix(metal, razorEdge, edgeBevel * 0.85);

                // Central blood groove
                if (yDistCenter < 0.25 && xDagger > 0.12 && xDagger < 0.78) {
                    float grooveDepth = smoothstep(0.25, 0.04, yDistCenter);
                    float bloodPulse = sin(time * 5.0 - xDagger * 10.0) * 0.5 + 0.5;
                    vec3 grooveBlood = mix(vec3(0.35, 0.01, 0.02), vec3(0.92, 0.03, 0.05), bloodPulse);
                    metal = mix(metal, grooveBlood, grooveDepth * 0.92);
                }

                // Golden guard
                if (xDagger < 0.14) {
                    vec3 ornateGold = vec3(0.90, 0.70, 0.20);
                    vec3 darkBronze = vec3(0.42, 0.25, 0.08);
                    float guardRings = sin(yDistCenter * 14.0) * 0.5 + 0.5;
                    vec3 guard = mix(darkBronze, ornateGold, guardRings);
                    metal = mix(guard, metal, smoothstep(0.10, 0.14, xDagger));
                }

                float gleam = pow(max(0.0, cos(xDagger * 3.14159 - time * 2.2)), 18.0);
                metal += vec3(0.5, 0.6, 0.7) * gleam;

                return vec4(metal, 1.0);
            }
        )";

        // ═════════════════════════════════════════════════════════════
        // 5. PROCEDURAL 1:1 PIXEL ART RIVETED FRAME SHADER (بكسل آرت)
        // ═════════════════════════════════════════════════════════════
        const std::string pixel_art_border_shader = R"(
            uniform float time;
            uniform vec2 resolution;

            vec4 main(vec2 fragCoord) {
                float p = 3.0; // Retro pixel size
                vec2 g = floor(fragCoord / p);
                vec2 sz = floor(resolution / p);

                float dL = g.x;
                float dR = sz.x - 1.0 - g.x;
                float dT = g.y;
                float dB = sz.y - 1.0 - g.y;

                float dX = min(dL, dR);
                float dY = min(dT, dB);

                // Palette extracted directly from the user's PNG image
                vec4 cOutline = vec4(0.18, 0.16, 0.19, 1.0); // #2E2930 Charcoal dark border
                vec4 cDark    = vec4(0.32, 0.29, 0.34, 1.0); // #524A57 Deep iron shadow
                vec4 cMid     = vec4(0.48, 0.45, 0.50, 1.0); // #7A7380 Stone/iron body
                vec4 cLight   = vec4(0.68, 0.65, 0.71, 1.0); // #AEA6B5 Highlight stone

                // ── 1. Corner Rivet Blocks (5x5 pixels) ──
                if (dX < 5.0 && dY < 5.0) {
                    if (dX == 0.0 && dY == 0.0) return vec4(0.0); // Outer diagonal corner cutout
                    if (dX == 0.0 || dY == 0.0 || dX == 4.0 || dY == 4.0) {
                        return cOutline;
                    }
                    if (dX == 2.0 && dY == 2.0) {
                        return cDark; // Center rivet hole
                    }
                    if (dY <= 2.0 && dX <= 2.0) {
                        return cLight; // Specular highlight
                    }
                    return cMid;
                }

                // ── 2. Edge Rails with Staggered Cog Teeth ──
                float depth = min(dX, dY);
                if (depth > 4.0) {
                    return vec4(0.0); // Inner transparent area
                }

                float along = (dX < dY) ? g.y : g.x;
                float modStep = mod(along, 4.0);

                // Row 0: Outer protruding teeth
                if (depth == 0.0) {
                    if (modStep == 1.0 || modStep == 2.0) return cOutline;
                    return vec4(0.0);
                }

                // Row 1: Outer rail line
                if (depth == 1.0) {
                    if (modStep == 1.0 || modStep == 2.0) return cLight;
                    return cOutline;
                }

                // Row 2: Center rail track
                if (depth == 2.0) {
                    if (modStep == 0.0 || modStep == 3.0) return cDark;
                    return cMid;
                }

                // Row 3: Inner rail line (staggered)
                if (depth == 3.0) {
                    if (modStep == 0.0 || modStep == 3.0) return cOutline;
                    return cDark;
                }

                // Row 4: Inner protruding teeth
                if (depth == 4.0) {
                    if (modStep == 0.0 || modStep == 3.0) return cOutline;
                    return vec4(0.0);
                }

                return cOutline;
            }
        )";

        // Card 4: Hero Dagger & Blood Border Container
        auto cardDaggers = container({
            .color = 0xF50A0E17,
            .border_radius = BorderRadius::circular(20.0f),
            .border = Border(Colors::Transparent, 16.0f),
            .box_shadow = {
                BoxShadow::glow(0x60DC2626, 30.0f),
                BoxShadow::standard(0x80000000, 20.0f, 10.0f)
            },
            .border_shader = dagger_blood_border_shader,
            .align = Alignment::Center,
            .width = StyleValue::point(420.0f),
            .height = StyleValue::point(220.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("⚔️ Daggers & Blood ⚔️", {
                        .color = 0xFFF87171,
                        .font_size = 20.0f,
                        .font_weight = FontWeight::Bold
                    }),
                    text("خناجر دمشقية حادة مع 5 بكسل دماء نابضة", {
                        .color = 0xFFF1F5F9,
                        .font_size = 13.0f,
                        .font_weight = FontWeight::Medium
                    }),
                    text("16px Injected SkSL Border • Tapered Blades", {
                        .color = 0xFF94A3B8,
                        .font_size = 11.0f
                    })
                }
            })
        });

        // Card 5: Pixel Art Riveted Frame Container (The User's PNG matched)
        auto cardPixelArt = container({
            .color = 0xF5131117,
            .border = Border(Colors::Transparent, 15.0f),
            .box_shadow = {
                BoxShadow::glow(0x407A7380, 20.0f),
                BoxShadow::standard(0x80000000, 15.0f, 8.0f)
            },
            .border_shader = pixel_art_border_shader,
            .align = Alignment::Center,
            .width = StyleValue::point(360.0f),
            .height = StyleValue::point(220.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("🎮 Pixel Art Frame", {
                        .color = 0xFFE2E8F0,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold
                    }),
                    text("إطار بكسل آرت طبق الأصل من الصورة", {
                        .color = 0xFFA69FA9,
                        .font_size = 13.0f
                    }),
                    text("5x5 Corner Rivets • Staggered Cog Teeth", {
                        .color = 0xFF736D77,
                        .font_size = 11.0f
                    })
                }
            })
        });

        // Card 6: Circular Dual-Shader with Daggers
        auto cardCircle = container({
            .border = Border(Colors::Transparent, 16.0f),
            .box_shadow = { BoxShadow::glow(0x80DC2626, 25.0f) },
            .shape = BoxShape::Circle,
            .background_shader = plasma_shader,
            .border_shader = dagger_blood_border_shader,
            .align = Alignment::Center,
            .width = StyleValue::point(220.0f),
            .height = StyleValue::point(220.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = {
                    text("Blood & Steel", { .color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                    text("Shield", { .color = 0xFFFCA5A5, .font_size = 13.0f })
                }
            })
        });

        // ═════════════════════════════════════════════════════════════
        // 6. DIRECT SVG VECTOR INJECTION SHOWCASE (حقن الـ SVG المباشر)
        // ═════════════════════════════════════════════════════════════

        // Card 7: Futuristic Sci-Fi HUD Chamfered Border (SVG Border)
        const std::string scifi_hud_svg = R"(
            <svg viewBox="0 0 360 200">
                <path d="M 0 30 L 30 0 H 260 L 280 20 H 330 L 360 50 V 170 L 330 200 H 100 L 80 180 H 30 L 0 150 Z"
                      fill="#161B22" stroke="#00E5FF" stroke-width="2"/>
                <path d="M 35 12 H 140" stroke="#00E5FF" stroke-width="1.5"/>
                <path d="M 348 60 V 140" stroke="#00E5FF" stroke-width="2"/>
                <polygon points="12,30 30,12 36,18 18,36" fill="#00E5FF"/>
                <polygon points="348,170 330,188 324,182 342,164" fill="#00E5FF"/>
            </svg>
        )";

        auto cardSvgHud = container({
            .border = Border(Colors::Transparent, 2.0f),
            .box_shadow = { BoxShadow::glow(0x4000E5FF, 20.0f) },
            .border_svg = scifi_hud_svg,
            .align = Alignment::Center,
            .width = StyleValue::point(360.0f),
            .height = StyleValue::point(200.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("⚡ Sci-Fi Vector HUD", { .color = 0xFF00E5FF, .font_size = 18.0f, .font_weight = FontWeight::Bold }),
                    text("Direct SVG Border Injection", { .color = 0xFF94A3B8, .font_size = 13.0f }),
                    text("Complex Chamfers & Tech Notches", { .color = 0xFF38BDF8, .font_size = 11.0f })
                }
            })
        });

        // Card 8: 9-Slice Fantasy / Ornate Frame (SVG 9-Slice)
        const std::string ornate_9slice_svg = R"(
            <svg viewBox="0 0 100 100">
                <path d="M 0 24 V 8 C 0 2 2 0 8 0 H 24 C 14 4 10 10 10 18 C 10 22 14 24 24 24 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
                <path d="M 100 24 V 8 C 100 2 98 0 92 0 H 76 C 86 4 90 10 90 18 C 90 22 86 24 76 24 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
                <path d="M 0 76 V 92 C 0 98 2 100 8 100 H 24 C 14 96 10 90 10 82 C 10 78 14 76 24 76 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
                <path d="M 100 76 V 92 C 100 98 98 100 92 100 H 76 C 86 96 90 90 90 82 C 90 78 86 76 76 76 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
                <line x1="24" y1="2" x2="76" y2="2" stroke="#FBBF24" stroke-width="2"/>
                <line x1="24" y1="98" x2="76" y2="98" stroke="#FBBF24" stroke-width="2"/>
                <line x1="2" y1="24" x2="2" y2="76" stroke="#FBBF24" stroke-width="2"/>
                <line x1="98" y1="24" x2="98" y2="76" stroke="#FBBF24" stroke-width="2"/>
            </svg>
        )";

        auto cardSvg9Slice = container({
            .color = 0xEB1A1412,
            .border = Border(Colors::Transparent, 2.0f),
            .box_shadow = { BoxShadow::glow(0x60F59E0B, 20.0f) },
            .border_svg = ornate_9slice_svg,
            .svg_slice = SvgSlice::all(24.0f),
            .align = Alignment::Center,
            .width = StyleValue::point(360.0f),
            .height = StyleValue::point(200.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("👑 9-Slice Vector Frame", { .color = 0xFFFCD34D, .font_size = 18.0f, .font_weight = FontWeight::Bold }),
                    text("Ornate Gold Filigree Corners", { .color = 0xFFFDE68A, .font_size = 13.0f }),
                    text("Zero Corner Distortion at Any Scale", { .color = 0xFFD97706, .font_size = 11.0f })
                }
            })
        });

        // Card 9: Vector Shape + Live SkSL Shader (The Ultimate Combo)
        const std::string hexagon_svg_path = "M 40 0 L 180 0 L 220 75 L 180 150 L 40 150 L 0 75 Z";
        auto cardSvgShaderCombo = container({
            .color = 0xE60D1117,
            .border = Border(Colors::Transparent, 3.0f),
            .box_shadow = { BoxShadow::glow(0x60EC4899, 25.0f) },
            .border_shader = cyber_border_shader,
            .border_svg = hexagon_svg_path,
            .align = Alignment::Center,
            .width = StyleValue::point(220.0f),
            .height = StyleValue::point(200.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = {
                    text("Hexagon Vector", { .color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                    text("+ Neon Shader", { .color = 0xFFF472B6, .font_size = 13.0f })
                }
            })
        });

        auto rowCards = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .children = { card1, card2, card3 }
        });

        auto heroRow = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .margin = StyleInsets::only(24.0f, 0, 0, 0),
            .children = { cardDaggers, cardPixelArt, cardCircle }
        });

        auto svgRow = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .margin = StyleInsets::only(24.0f, 0, 0, 0),
            .children = { cardSvgHud, cardSvg9Slice, cardSvgShaderCombo }
        });

        auto contentCol = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .children = { header, rowCards, heroRow, svgRow }
        });

        auto app_body = container({
            .color = 0x4D000000,
            .align = Alignment::Center,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(20.0f, 20.0f),
            .child = scrollView(contentCol)
        });

        // Wrap the entire app in WindowFrame (CSD) with glowing SkSL border_shader
        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "ENKI — Container Shaders Showcase",
            .border_radius = 12.0f,
            .border_width = 15.0f,
            .background_color = 0x4D000000,
            .border_shader = pixel_art_border_shader,
            .titlebar_background_color = 0x4D000000,
            .titlebar_inactive_background_color = 0x4D000000,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
    }
};

class ContainerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ContainerDemoState>();
    }
    std::string_view typeName() const override { return "ContainerDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Container Shaders Showcase (CSD)\n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "ENKI — Container Shaders Showcase";
    config.width       = 1180;
    config.height      = 760;
    config.resizable   = true;
    config.enable_csd  = true;
    config.app_id      = "org.enki.container_demo";
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = false;
    config.clear_color = 0x0000004D;

    return runApp(std::make_shared<ContainerDemoApp>(), config);
}
