#pragma once
#include <cstdint>
#include <string>
#include "../math/color.hpp"

// テキスト装飾フラグ
// Siv3D の TextStyle + TextMeshPro の FontStyles に相当
enum class TextDecoration : uint32_t {
    None         = 0,
    Bold         = 1 << 0,   // 太字 (SDF threshold 調整で実現)
    Italic       = 1 << 1,   // 斜体 (頂点シアー変換で実現)
    Underline    = 1 << 2,   // 下線
    Strikethrough = 1 << 3,  // 取り消し線
    Superscript  = 1 << 4,   // 上付き文字
    Subscript    = 1 << 5    // 下付き文字
};

inline TextDecoration operator|(TextDecoration a, TextDecoration b) {
    return static_cast<TextDecoration>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline TextDecoration operator&(TextDecoration a, TextDecoration b) {
    return static_cast<TextDecoration>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline bool has_flag(TextDecoration flags, TextDecoration flag) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

// テキストスタイル: 1つのテキスト範囲に適用される視覚パラメータ
// Siv3D の TextStyle / TextMeshPro の TMP_Style に相当
//
// SDF/MSDFシェーダーでの描画パラメータを定義:
// - face: 文字本体の色と太さ
// - outline: アウトライン (SDF閾値範囲で実現)
// - shadow: ドロップシャドウ (SDFオフセットで実現)
// - glow: グロー効果 (SDF外側の範囲で実現)
struct TextStyle {
    // 文字本体
    Color face_color{255, 255, 255, 255};
    float face_softness = 0.0f;       // エッジのぼかし量 [0, 1]
    float face_dilate = 0.0f;         // 文字の太らせ量 [-1, 1] (Bold用)

    // アウトライン (Siv3D の outline / TextMeshPro の Outline)
    Color outline_color{0, 0, 0, 255};
    float outline_width = 0.0f;       // アウトラインの太さ [0, 1] (0=無し)
    float outline_softness = 0.0f;    // アウトラインのぼかし

    // ドロップシャドウ (TextMeshPro の Underlay)
    Color shadow_color{0, 0, 0, 128};
    float shadow_offset_x = 0.0f;     // 影のX方向オフセット
    float shadow_offset_y = 0.0f;     // 影のY方向オフセット
    float shadow_dilate = 0.0f;       // 影の拡張量
    float shadow_softness = 0.0f;     // 影のぼかし量

    // グロー (TextMeshPro の Glow)
    Color glow_color{255, 255, 255, 0};
    float glow_offset = 0.0f;         // グロー開始位置
    float glow_inner = 0.0f;          // グロー内側の強さ
    float glow_outer = 0.0f;          // グロー外側の強さ

    // テキスト装飾
    TextDecoration decoration = TextDecoration::None;
    float italic_slant = 0.2f;        // 斜体の傾き角度 (tan値)

    // プリセット
    static TextStyle default_style() { return {}; }

    static TextStyle with_outline(Color face, Color outline, float width) {
        TextStyle s;
        s.face_color = face;
        s.outline_color = outline;
        s.outline_width = width;
        return s;
    }

    static TextStyle with_shadow(Color face, Color shadow, float ox, float oy) {
        TextStyle s;
        s.face_color = face;
        s.shadow_color = shadow;
        s.shadow_offset_x = ox;
        s.shadow_offset_y = oy;
        s.shadow_dilate = 0.1f;
        s.shadow_softness = 0.2f;
        return s;
    }
};

// テキストマテリアル: シェーダー + スタイルの組み合わせ
// TextMeshPro の Material Preset に相当
//
// 複数のテキストコンポーネントが同じマテリアルを共有でき、
// 動的バッチングによる描画コスト削減が可能
struct TextMaterialHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

struct TextMaterial {
    uint64_t id = 0;
    std::string name;
    TextStyle style;
    // シェーダーバリアント選択 (将来の拡張用)
    uint32_t shader_variant = 0;
};
