# モジュール テスト設計書: [モジュール名]

> **対象仕様書**: `docs/specs/[module_name]_spec.md`
> **テストファイル**: `tests/test_[domain].cpp`
> **作成日**: YYYY-MM-DD
> **最終更新**: YYYY-MM-DD

---

## 1. テスト概要

### 1.1 テスト対象

| 項目 | 値 |
|------|-----|
| モジュール名 | |
| ヘッダ | `system/[domain]/[module_name].hpp` |
| 満たす concept | `RendererBackend` / `InputBackend` / `WindowBackend` / その他 |
| テストスイート名 | `"System/[ModuleName]"` |

### 1.2 テスト方針

- GUI 非依存 (テストフレームワーク `ergo::test` を使用)
- ハードウェア依存部分はスタブ/モックで代替
- concept 準拠を静的 or 動的に検証
- プラットフォーム固有テストは条件コンパイルで制御

### 1.3 テスト実行環境

| 項目 | 要件 |
|------|------|
| ハードウェア依存 | `あり` / `なし` |
| モック方針 | <!-- 例: ダミーデバイスで initialize() をテスト --> |
| CI 実行可否 | `可` / `不可 (ハードウェア必須)` / `条件付き可` |

---

## 2. concept 準拠テスト

| テスト ID | テスト名 | 検証内容 | 期待結果 |
|----------|---------|---------|---------|
| MC-01 | `satisfies_concept` | concept 要件をすべて満たすか | コンパイル成功 / static_assert 通過 |
| MC-02 | `required_methods_exist` | 必須メソッドの呼び出し可能性 | コンパイル成功 |

---

## 3. 単体テスト

### 3.1 初期化 / シャットダウン

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| IN-01 | `default_construction` | なし | デフォルトコンストラクタ | 内部状態が未初期化 |
| IN-02 | `initialize_succeeds` | 未初期化 | `initialize()` | `true` を返す |
| IN-03 | `double_initialize` | 初期化済み | 再度 `initialize()` | 安全に処理 (冪等 or false) |
| IN-04 | `shutdown_after_init` | 初期化済み | `shutdown()` | リソース解放、クラッシュなし |
| IN-05 | `shutdown_without_init` | 未初期化 | `shutdown()` | クラッシュなし |
| IN-06 | `reinitialize_after_shutdown` | shutdown 済み | `initialize()` | 再初期化成功 |

### 3.2 フレーム処理

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| FR-01 | `begin_end_frame_cycle` | 初期化済み | `begin_frame()` → `end_frame()` | 正常完了 |
| FR-02 | `multiple_frame_cycles` | 初期化済み | 複数回 begin/end 繰り返し | 安定動作 |
| FR-03 | `begin_frame_without_init` | 未初期化 | `begin_frame()` | クラッシュしない |

### 3.3 公開 API

<!-- concept 要件以外のモジュール固有メソッドのテストを設計する -->

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| API-01 | | | | |
| API-02 | | | | |

### 3.4 エラー / 異常系

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| ERR-01 | `invalid_parameter` | 初期化済み | 不正パラメータでの呼び出し | エラー処理、クラッシュなし |
| ERR-02 | `resource_unavailable` | 初期化済み | リソース不足シミュレーション | 適切なエラー返却 |

---

## 4. プラットフォーム固有テスト

<!-- プラットフォーム別に必要な追加テストを記述する -->

### 4.1 Desktop (Windows / Linux)

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| PD-01 | | | | |

### 4.2 モバイル (Android / iOS)

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| PM-01 | | | | |

---

## 5. モック / スタブ定義

<!-- テストで使用するモックやスタブの仕様を定義する -->

```cpp
// 例: ダミーレンダラー (ハードウェア非依存)
struct MockRenderer {
    bool initialized = false;
    uint32_t frame_count = 0;

    bool initialize() { initialized = true; return true; }
    void begin_frame() { ++frame_count; }
    void end_frame() {}
    void shutdown() { initialized = false; }
};
static_assert(RendererBackend<MockRenderer>);
```

---

## 6. テスト実装テンプレート

```cpp
#include "framework/test_framework.hpp"
#include "system/[domain]/[module_name].hpp"

using namespace ergo::test;

// ============================================================
// [ModuleName] tests
// ============================================================

static TestSuite suite_[module]("System/[ModuleName]");

static void register_[module]_tests() {
    // --- concept compliance ---
    suite_[module].add("satisfies_concept", [](TestContext& ctx) {
        // static_assert(RendererBackend<[ModuleName]>);
        ERGO_TEST_ASSERT_TRUE(ctx, true);  // コンパイル通過で検証
    });

    // --- initialization ---
    suite_[module].add("default_construction", [](TestContext& ctx) {
        [ModuleName] mod;
        // ERGO_TEST_ASSERT_FALSE(ctx, mod.is_initialized());
    });

    suite_[module].add("initialize_succeeds", [](TestContext& ctx) {
        [ModuleName] mod;  // or MockModule
        ERGO_TEST_ASSERT_TRUE(ctx, mod.initialize());
        mod.shutdown();
    });

    suite_[module].add("shutdown_without_init", [](TestContext& ctx) {
        [ModuleName] mod;
        mod.shutdown();  // should not crash
        ERGO_TEST_ASSERT_TRUE(ctx, true);
    });

    // --- frame cycle ---
    suite_[module].add("begin_end_frame_cycle", [](TestContext& ctx) {
        [ModuleName] mod;
        mod.initialize();
        mod.begin_frame();
        mod.end_frame();
        mod.shutdown();
        ERGO_TEST_ASSERT_TRUE(ctx, true);
    });

    // --- module-specific API ---
    // TODO: モジュール固有の API テストを追加

    // --- error handling ---
    // TODO: エラー系テストを追加
}

// テストスイート登録 (main.cpp から呼び出し)
// void register_[domain]_tests(TestRunner& runner) 内に以下を追加:
//   register_[module]_tests();
//   runner.add_suite(suite_[module]);
```

---

## 7. テストカバレッジ目標

| カテゴリ | テスト数 | カバレッジ目標 |
|---------|---------|--------------|
| concept 準拠 | | 100% |
| 初期化 / シャットダウン | | 100% |
| フレーム処理 | | 基本サイクル |
| 公開 API | | 各メソッド最低1ケース |
| エラー / 異常系 | | 主要パターン |
| プラットフォーム固有 | | 対象プラットフォームごと |

---

## 8. 変更履歴

| 日付 | 変更内容 |
|------|---------|
| YYYY-MM-DD | 初版作成 |
