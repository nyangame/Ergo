#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "glyph.hpp"
#include "font_atlas.hpp"

// フォントアセットハンドル (エンジン全体で共有するID)
struct FontHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

// フォントフェイス情報: フォントファイルから取得されるメタデータ
struct FontFaceInfo {
    std::string family_name;          // フォントファミリー名 (例: "Noto Sans JP")
    std::string style_name;           // スタイル名 (例: "Regular", "Bold")
    float units_per_em = 0.0f;        // EMスクエアのユニット数
    float ascender = 0.0f;            // アセンダーライン (ベースラインから上端)
    float descender = 0.0f;           // ディセンダーライン (ベースラインから下端、負値)
    float line_height = 0.0f;         // 行の高さ
    float underline_offset = 0.0f;    // 下線の位置
    float underline_thickness = 0.0f; // 下線の太さ
    float strikethrough_offset = 0.0f;
    float strikethrough_thickness = 0.0f;
};

// フォントアセット: 1つのフォントに対応するデータの集合
// Siv3D の Font / TextMeshPro の TMP_FontAsset に相当
//
// 設計方針:
// - フォントアセットはフォントデータ+アトラス+グリフテーブルの所有者
// - TextコンポーネントはFontHandleを通じてフォントアセットを参照
// - フォールバック機能により、文字が見つからない場合は代替フォントを検索
struct FontAsset {
    uint64_t id = 0;
    std::string name;                 // アセット名 (ユーザー定義)
    std::string source_path;          // フォントファイルパス (.ttf, .otf)
    float base_size = 32.0f;          // MSDF基本サイズ (ピクセル)

    FontFaceInfo face;
    FontAtlas atlas;

    // グリフテーブル: glyph_index → Glyph
    // TextMeshPro の glyphTable に相当
    std::unordered_map<uint32_t, Glyph> glyph_table;

    // 文字テーブル: codepoint → glyph_index
    // TextMeshPro の characterTable に相当
    std::unordered_map<uint32_t, uint32_t> character_table;

    // カーニングテーブル: (first << 32 | second) → KerningPair
    std::unordered_map<uint64_t, KerningPair> kerning_table;

    // フォールバックフォント: この順序で文字を検索
    // TextMeshPro の fallbackFontAssetTable に相当
    std::vector<FontHandle> fallback_fonts;

    // ユーティリティ
    static uint64_t kerning_key(uint32_t first, uint32_t second) {
        return (static_cast<uint64_t>(first) << 32) | second;
    }

    // コードポイントに対応するグリフを検索
    const Glyph* find_glyph(uint32_t codepoint) const {
        auto char_it = character_table.find(codepoint);
        if (char_it == character_table.end()) return nullptr;
        auto glyph_it = glyph_table.find(char_it->second);
        if (glyph_it == glyph_table.end()) return nullptr;
        return &glyph_it->second;
    }

    // カーニング値を取得
    float get_kerning(uint32_t first, uint32_t second) const {
        auto it = kerning_table.find(kerning_key(first, second));
        if (it == kerning_table.end()) return 0.0f;
        return it->second.x_advance;
    }
};
