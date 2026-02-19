# コンポーネント テスト設計書: [コンポーネント名]

> **対象仕様書**: `docs/specs/[component_name]_spec.md`
> **テストファイル**: `tests/test_[domain].cpp`
> **作成日**: YYYY-MM-DD
> **最終更新**: YYYY-MM-DD

---

## 1. テスト概要

### 1.1 テスト対象

| 項目 | 値 |
|------|-----|
| コンポーネント名 | |
| ヘッダ | `engine/[domain]/[component_name].hpp` |
| 満たす concept | `TaskLike` / `BehaviourLike` / `Drawable` / その他 |
| テストスイート名 | `"[Domain]/[ComponentName]"` |

### 1.2 テスト方針

<!-- テスト対象コンポーネントのテスト方針を記述する -->

- GUI 非依存 (テストフレームワーク `ergo::test` を使用)
- 外部依存はモック/スタブで代替
- concept 準拠を静的 or 動的に検証

---

## 2. concept 準拠テスト

<!-- コンポーネントが満たすべき concept の準拠を確認するテスト -->

| テスト ID | テスト名 | 検証内容 | 期待結果 |
|----------|---------|---------|---------|
| CC-01 | `satisfies_concept` | concept 要件をすべて満たすか | コンパイル成功 / static_assert 通過 |
| CC-02 | `type_name_returns_valid` | `type_name()` が空でない文字列を返す (BehaviourLike) | 非空の string_view |
| CC-03 | `threading_policy_declared` | `threading_policy()` の戻り値が妥当 (ThreadAware) | 有効な ThreadingPolicy 値 |

---

## 3. 単体テスト

### 3.1 初期化 / ライフサイクル

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| LC-01 | `default_construction` | なし | デフォルトコンストラクタで生成 | プロパティがデフォルト値 |
| LC-02 | `start_initializes_state` | デフォルト構築済み | `start()` 呼び出し | 内部状態が初期化される |
| LC-03 | `release_clears_resources` | `start()` 済み | `release()` 呼び出し | コールバック等がクリアされる |
| LC-04 | `start_after_release` | `release()` 済み | 再度 `start()` | 正常に再初期化される |

### 3.2 更新処理 (update)

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| UP-01 | `update_with_zero_dt` | `start()` 済み | `update(0.0f)` | 状態変化なし |
| UP-02 | `update_normal_frame` | `start()` 済み | `update(0.016f)` | 正常更新 |
| UP-03 | `update_large_dt` | `start()` 済み | `update(1.0f)` | オーバーフロー等なし |
| | | | | |

### 3.3 公開 API

<!-- 各パブリックメソッドに対するテストを設計する -->

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| API-01 | | | | |
| API-02 | | | | |

### 3.4 コールバック / イベント

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| CB-01 | `callback_fires_on_condition` | コールバック登録済み | 発火条件を満たす | コールバックが呼ばれる |
| CB-02 | `null_callback_no_crash` | コールバック未登録 | 発火条件を満たす | クラッシュしない |

### 3.5 境界値・異常系

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| BV-01 | `negative_values` | `start()` 済み | 負の値を設定 | 適切に処理 |
| BV-02 | `zero_values` | `start()` 済み | ゼロ値を設定 | 適切に処理 |
| BV-03 | `max_values` | `start()` 済み | 極大値を設定 | オーバーフローなし |

---

## 4. 描画テスト (Drawable の場合)

| テスト ID | テスト名 | 前提条件 | 操作 | 期待結果 |
|----------|---------|---------|------|---------|
| DR-01 | `draw_without_start` | 未初期化 | `draw(ctx)` | クラッシュしない |
| DR-02 | `draw_normal` | `start()` 済み | `draw(ctx)` | コマンドが記録される |

---

## 5. エディタ連携テスト

<!-- エディタ対応ありの場合のみ記述する -->

| テスト ID | テスト名 | 検証内容 | 期待結果 |
|----------|---------|---------|---------|
| ED-01 | `serialization_roundtrip` | プロパティのシリアライズ → デシリアライズ | 値が一致 |
| ED-02 | `property_range_clamping` | エディタからの範囲外値の設定 | クランプされるか拒否 |

---

## 6. テスト実装テンプレート

```cpp
#include "framework/test_framework.hpp"
#include "engine/[domain]/[component_name].hpp"

using namespace ergo::test;

// ============================================================
// [ComponentName] tests
// ============================================================

static TestSuite suite_[component]("[Domain]/[ComponentName]");

static void register_[component]_tests() {
    // --- concept compliance ---
    suite_[component].add("satisfies_concept", [](TestContext& ctx) {
        // static_assert(BehaviourLike<[ComponentName]>);
        // static_assert(ThreadAware<[ComponentName]>);
        ERGO_TEST_ASSERT_TRUE(ctx, true);  // コンパイル通過で検証
    });

    // --- lifecycle ---
    suite_[component].add("default_construction", [](TestContext& ctx) {
        [ComponentName] obj;
        // ERGO_TEST_ASSERT_EQ(ctx, obj.property, expected_default);
    });

    suite_[component].add("start_initializes_state", [](TestContext& ctx) {
        [ComponentName] obj;
        obj.start();
        // ERGO_TEST_ASSERT_TRUE(ctx, ...);
    });

    suite_[component].add("release_clears_resources", [](TestContext& ctx) {
        [ComponentName] obj;
        obj.start();
        obj.release();
        // ERGO_TEST_ASSERT_TRUE(ctx, ...);
    });

    // --- update ---
    suite_[component].add("update_normal_frame", [](TestContext& ctx) {
        [ComponentName] obj;
        obj.start();
        obj.update(0.016f);
        // ERGO_TEST_ASSERT_TRUE(ctx, ...);
    });

    // --- public API ---
    // TODO: コンポーネント固有の API テストを追加

    // --- callbacks ---
    // TODO: コールバックのテストを追加

    // --- boundary values ---
    // TODO: 境界値テストを追加
}

// テストスイート登録 (main.cpp から呼び出し)
// void register_[domain]_tests(TestRunner& runner) 内に以下を追加:
//   register_[component]_tests();
//   runner.add_suite(suite_[component]);
```

---

## 7. テストカバレッジ目標

| カテゴリ | テスト数 | カバレッジ目標 |
|---------|---------|--------------|
| concept 準拠 | | 100% |
| ライフサイクル | | 100% |
| 公開 API | | 各メソッド最低1ケース |
| コールバック | | 発火 + null 安全性 |
| 境界値 | | 主要パラメータ |
| 描画 | | Drawable の場合のみ |
| エディタ連携 | | エディタ対応ありの場合のみ |

---

## 8. 変更履歴

| 日付 | 変更内容 |
|------|---------|
| YYYY-MM-DD | 初版作成 |
