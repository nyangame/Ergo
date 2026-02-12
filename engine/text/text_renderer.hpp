#pragma once
#include <vector>
#include <cstdint>
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/color.hpp"
#include "../resource/texture_handle.hpp"
#include "glyph.hpp"
#include "text_style.hpp"
#include "text_layout.hpp"
#include "font_asset.hpp"

// 前方宣言
struct SimpleText;
struct RichText;

// テキスト描画用の頂点フォーマット
// SDF/MSDFシェーダーへの入力データ
struct TextVertex {
    float pos_x, pos_y, pos_z;   // ワールド座標
    float uv_x, uv_y;            // アトラステクスチャUV
    uint8_t r, g, b, a;          // 頂点カラー
};

// バッチ描画単位: 同一アトラスページ + 同一マテリアルのグリフ群
struct TextDrawBatch {
    uint32_t atlas_page = 0;         // フォントアトラスページ番号
    FontHandle font{0};              // フォントアセット
    TextMaterialHandle material{0};  // テキストマテリアル
    std::vector<TextVertex> vertices;
    std::vector<uint32_t> indices;
};

// テキストレンダリングコマンド: render pipeline に投入する描画データ
// 既存の RenderCmd_DrawText を置換する高機能版
struct RenderCmd_DrawTextBatch {
    Vec3f origin;                    // テキストブロックの原点
    uint64_t font_id = 0;           // フォントアセットID
    uint64_t material_id = 0;       // テキストマテリアルID
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;
    // 実際の頂点・インデックスデータは別バッファで管理
    // (render pipeline のリソースマネージャ経由)
    uint64_t vertex_buffer_id = 0;
    uint64_t index_buffer_id = 0;
};

// テキストレンダラー: レイアウト結果からバッチ描画データを生成
// Siv3D の内部描画パイプライン / TextMeshPro の TMP_MeshInfo に相当
//
// レイアウト結果 (PlacedGlyph の配列) をテキスト描画用の頂点バッファに変換し、
// 同一アトラスページごとにバッチを構成する
class TextRenderer {
public:
    // レイアウト結果から描画バッチを生成
    static std::vector<TextDrawBatch> build_batches(
        const TextLayoutResult& layout,
        const FontAsset& font,
        const TextStyle& style,
        Vec3f origin
    );

    // SimpleText の描画バッチ生成 (ヘルパー)
    static std::vector<TextDrawBatch> build_simple(
        const SimpleText& text,
        const FontAsset& font
    );

    // RichText の描画バッチ生成 (ヘルパー)
    static std::vector<TextDrawBatch> build_rich(
        const RichText& text,
        const FontAsset& font
    );

private:
    // 1グリフ分の4頂点 + 6インデックスを追加
    static void emit_glyph_quad(
        TextDrawBatch& batch,
        const PlacedGlyph& pg,
        const FontAsset& font,
        const TextStyle& style,
        Vec3f origin
    );
};
