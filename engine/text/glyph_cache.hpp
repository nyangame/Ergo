#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "glyph.hpp"
#include "font_atlas.hpp"

// グリフキャッシュ: 動的アトラスポピュレーションのためのランタイムキャッシュ
// Siv3D の実行時グリフキャッシュ / TextMeshPro の Dynamic Atlas に相当
//
// 使用文字が事前に確定できない場合 (チャット、ユーザー入力等) に、
// 必要なグリフをオンデマンドでアトラスに追加する仕組み
class GlyphCache {
public:
    // キャッシュエントリ: LRU用のアクセスカウンタ付き
    struct Entry {
        Glyph glyph;
        uint64_t last_used_frame = 0;
        uint32_t use_count = 0;
    };

private:
    // コードポイント → キャッシュエントリ
    std::unordered_map<uint32_t, Entry> entries_;

    // 現在のフレーム番号
    uint64_t current_frame_ = 0;

    // キャッシュ容量 (最大グリフ数)
    uint32_t capacity_ = 4096;

    // ダーティフラグ: アトラス更新が必要なグリフのコードポイント
    std::unordered_set<uint32_t> pending_upload_;

public:
    GlyphCache() = default;
    explicit GlyphCache(uint32_t capacity) : capacity_(capacity) {}

    // フレーム開始時に呼ぶ
    void begin_frame() { ++current_frame_; }

    // グリフがキャッシュに存在するか
    bool contains(uint32_t codepoint) const {
        return entries_.find(codepoint) != entries_.end();
    }

    // グリフを取得 (存在しない場合 nullptr)
    const Glyph* get(uint32_t codepoint) {
        auto it = entries_.find(codepoint);
        if (it == entries_.end()) return nullptr;
        it->second.last_used_frame = current_frame_;
        ++it->second.use_count;
        return &it->second.glyph;
    }

    // グリフをキャッシュに追加 (容量超過時はLRU削除)
    void insert(const Glyph& glyph) {
        if (entries_.size() >= capacity_) {
            evict_lru();
        }
        Entry entry;
        entry.glyph = glyph;
        entry.last_used_frame = current_frame_;
        entry.use_count = 1;
        entries_[glyph.codepoint] = entry;
        pending_upload_.insert(glyph.codepoint);
    }

    // GPU側アトラスへのアップロード待ちのコードポイント一覧
    const std::unordered_set<uint32_t>& pending_uploads() const {
        return pending_upload_;
    }

    // アップロード完了通知
    void clear_pending() { pending_upload_.clear(); }

    uint32_t size() const { return static_cast<uint32_t>(entries_.size()); }
    uint32_t capacity() const { return capacity_; }

private:
    // LRU削除: 最も古くアクセスされたエントリを削除
    void evict_lru() {
        if (entries_.empty()) return;

        uint32_t victim_cp = 0;
        uint64_t oldest_frame = UINT64_MAX;

        for (const auto& [cp, entry] : entries_) {
            if (entry.last_used_frame < oldest_frame) {
                oldest_frame = entry.last_used_frame;
                victim_cp = cp;
            }
        }
        entries_.erase(victim_cp);
    }
};
