#include "rich_text.hpp"
#include <algorithm>
#include <charconv>
#include <cstdlib>

// ---------------------------------------------------------------
// カラーコードパーサー
// ---------------------------------------------------------------

Color RichText::parse_hex_color(std::string_view hex) {
    // '#' を除去
    if (!hex.empty() && hex[0] == '#') hex = hex.substr(1);

    auto hex_digit = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    };

    auto hex_byte = [&](std::string_view s, size_t offset) -> uint8_t {
        if (offset + 1 >= s.size()) return 0;
        return (hex_digit(s[offset]) << 4) | hex_digit(s[offset + 1]);
    };

    Color color{255, 255, 255, 255};
    if (hex.size() >= 6) {
        color.r = hex_byte(hex, 0);
        color.g = hex_byte(hex, 2);
        color.b = hex_byte(hex, 4);
    }
    if (hex.size() >= 8) {
        color.a = hex_byte(hex, 6);
    }
    return color;
}

float RichText::parse_float(std::string_view str) {
    float val = 0.0f;
    std::from_chars(str.data(), str.data() + str.size(), val);
    return val;
}

// ---------------------------------------------------------------
// スタイルスタック (パーサー内部用)
// ---------------------------------------------------------------

struct StyleState {
    Color color{255, 255, 255, 255};
    float font_size = 0.0f;
    FontHandle font{0};
    TextDecoration decoration = TextDecoration::None;
    float italic_slant = 0.2f;
};

// ---------------------------------------------------------------
// マークアップパーサー
// ---------------------------------------------------------------

// タグ名と属性を抽出するヘルパー
struct ParsedTag {
    std::string_view name;
    bool is_closing = false;
    // 簡易属性マップ (最大4属性)
    struct Attr {
        std::string_view key;
        std::string_view value;
    };
    Attr attrs[4];
    uint32_t attr_count = 0;

    std::string_view get_attr(std::string_view key) const {
        for (uint32_t i = 0; i < attr_count; ++i) {
            if (attrs[i].key == key) return attrs[i].value;
        }
        return {};
    }

    // 無名属性 (例: <color=#FF0000> の "#FF0000" 部分)
    std::string_view default_value() const {
        if (attr_count > 0 && attrs[0].key.empty()) return attrs[0].value;
        return {};
    }
};

static ParsedTag parse_tag(std::string_view tag_content) {
    ParsedTag result;

    // 先頭・末尾のスペースを削除
    while (!tag_content.empty() && tag_content.front() == ' ') tag_content = tag_content.substr(1);
    while (!tag_content.empty() && tag_content.back() == ' ') tag_content = tag_content.substr(0, tag_content.size() - 1);

    // 閉じタグ判定
    if (!tag_content.empty() && tag_content[0] == '/') {
        result.is_closing = true;
        tag_content = tag_content.substr(1);
    }

    // タグ名抽出
    size_t name_end = 0;
    while (name_end < tag_content.size() && tag_content[name_end] != ' ' && tag_content[name_end] != '=') {
        ++name_end;
    }
    result.name = tag_content.substr(0, name_end);

    if (result.is_closing) return result;

    // 属性パース
    size_t pos = name_end;
    while (pos < tag_content.size() && result.attr_count < 4) {
        // スペースをスキップ
        while (pos < tag_content.size() && tag_content[pos] == ' ') ++pos;
        if (pos >= tag_content.size()) break;

        if (tag_content[pos] == '=') {
            // 無名属性 (tag_name=value)
            ++pos;
            size_t val_start = pos;
            // 引用符で囲まれた値
            if (pos < tag_content.size() && (tag_content[pos] == '"' || tag_content[pos] == '\'')) {
                char quote = tag_content[pos];
                ++pos;
                val_start = pos;
                while (pos < tag_content.size() && tag_content[pos] != quote) ++pos;
                result.attrs[result.attr_count].value = tag_content.substr(val_start, pos - val_start);
                if (pos < tag_content.size()) ++pos;
            } else {
                while (pos < tag_content.size() && tag_content[pos] != ' ') ++pos;
                result.attrs[result.attr_count].value = tag_content.substr(val_start, pos - val_start);
            }
            result.attr_count++;
        } else {
            // key=value 形式
            size_t key_start = pos;
            while (pos < tag_content.size() && tag_content[pos] != '=' && tag_content[pos] != ' ') ++pos;
            auto key = tag_content.substr(key_start, pos - key_start);

            if (pos < tag_content.size() && tag_content[pos] == '=') {
                ++pos;
                size_t val_start = pos;
                if (pos < tag_content.size() && (tag_content[pos] == '"' || tag_content[pos] == '\'')) {
                    char quote = tag_content[pos];
                    ++pos;
                    val_start = pos;
                    while (pos < tag_content.size() && tag_content[pos] != quote) ++pos;
                    result.attrs[result.attr_count].key = key;
                    result.attrs[result.attr_count].value = tag_content.substr(val_start, pos - val_start);
                    if (pos < tag_content.size()) ++pos;
                } else {
                    while (pos < tag_content.size() && tag_content[pos] != ' ') ++pos;
                    result.attrs[result.attr_count].key = key;
                    result.attrs[result.attr_count].value = tag_content.substr(val_start, pos - val_start);
                }
                result.attr_count++;
            }
        }
    }

    return result;
}

std::vector<RichTextSegment> RichText::parse_markup(
    std::string_view markup,
    Color default_color,
    float default_size)
{
    std::vector<RichTextSegment> result;

    // スタイルスタック (ネスト対応)
    struct StackEntry {
        StyleState state;
        RichTagType tag_type;
    };
    std::vector<StackEntry> style_stack;

    StyleState current;
    current.color = default_color;
    current.font_size = default_size;

    size_t pos = 0;
    size_t text_start = 0;

    while (pos < markup.size()) {
        if (markup[pos] == '<') {
            // タグ開始: ここまでのテキストをセグメントとして追加
            if (pos > text_start) {
                RichTextSegment seg;
                seg.text = std::string(markup.substr(text_start, pos - text_start));
                seg.color = current.color;
                seg.font_size = current.font_size;
                seg.font = current.font;
                seg.decoration = current.decoration;
                seg.italic_slant = current.italic_slant;
                result.push_back(std::move(seg));
            }

            // タグ終了位置を検索
            size_t tag_end = markup.find('>', pos);
            if (tag_end == std::string_view::npos) {
                // 閉じ '>' がない場合はリテラルとして扱う
                text_start = pos;
                ++pos;
                continue;
            }

            auto tag_content = markup.substr(pos + 1, tag_end - pos - 1);
            auto tag = parse_tag(tag_content);

            if (tag.is_closing) {
                // 閉じタグ: スタックからポップ
                if (!style_stack.empty()) {
                    current = style_stack.back().state;
                    style_stack.pop_back();
                }
            } else {
                // 開きタグ: 現在のスタイルをプッシュしてから変更
                StackEntry entry;
                entry.state = current;

                if (tag.name == "color" || tag.name == "c") {
                    entry.tag_type = RichTagType::Color;
                    auto val = tag.default_value();
                    if (!val.empty()) {
                        current.color = parse_hex_color(val);
                    }
                } else if (tag.name == "size") {
                    entry.tag_type = RichTagType::Size;
                    auto val = tag.default_value();
                    if (!val.empty()) {
                        current.font_size = parse_float(val);
                    }
                } else if (tag.name == "b") {
                    entry.tag_type = RichTagType::Bold;
                    current.decoration = current.decoration | TextDecoration::Bold;
                } else if (tag.name == "i") {
                    entry.tag_type = RichTagType::Italic;
                    current.decoration = current.decoration | TextDecoration::Italic;
                } else if (tag.name == "u") {
                    entry.tag_type = RichTagType::Underline;
                    current.decoration = current.decoration | TextDecoration::Underline;
                } else if (tag.name == "s") {
                    entry.tag_type = RichTagType::Strikethrough;
                    current.decoration = current.decoration | TextDecoration::Strikethrough;
                } else if (tag.name == "sup") {
                    entry.tag_type = RichTagType::Superscript;
                    current.decoration = current.decoration | TextDecoration::Superscript;
                    current.font_size = current.font_size * 0.65f;
                } else if (tag.name == "sub") {
                    entry.tag_type = RichTagType::Subscript;
                    current.decoration = current.decoration | TextDecoration::Subscript;
                    current.font_size = current.font_size * 0.65f;
                } else if (tag.name == "font") {
                    entry.tag_type = RichTagType::Font;
                    // フォント名はフォントレジストリで解決 (ここではハンドルに変換しない)
                    // 実行時に解決するためフォント名を保持する必要があるが、
                    // 現段階ではFontHandle=0のまま (レジストリ連携は利用側で行う)
                }

                style_stack.push_back(entry);
            }

            pos = tag_end + 1;
            text_start = pos;
        } else {
            ++pos;
        }
    }

    // 残りのテキスト
    if (pos > text_start) {
        RichTextSegment seg;
        seg.text = std::string(markup.substr(text_start, pos - text_start));
        seg.color = current.color;
        seg.font_size = current.font_size;
        seg.font = current.font;
        seg.decoration = current.decoration;
        seg.italic_slant = current.italic_slant;
        result.push_back(std::move(seg));
    }

    return result;
}

// ---------------------------------------------------------------
// レイアウト構築
// ---------------------------------------------------------------

TextLayoutResult RichText::build_layout(const FontAsset& font_asset) const {
    // セグメントを TextLayoutEngine::StyledSegment に変換
    std::vector<TextLayoutEngine::StyledSegment> styled;
    styled.reserve(segments.size());

    for (const auto& seg : segments) {
        TextLayoutEngine::StyledSegment ss;
        ss.text = seg.text;
        ss.font = seg.font.valid() ? seg.font : default_font;
        ss.font_size = (seg.font_size > 0.0f) ? seg.font_size : layout_config.font_size;
        ss.color = seg.color;
        ss.decoration = seg.decoration;
        ss.italic_slant = seg.italic_slant;
        styled.push_back(ss);
    }

    return TextLayoutEngine::layout_rich(styled, font_asset, layout_config);
}
