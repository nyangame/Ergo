# モジュール仕様書: [モジュール名]

> **レイヤー**: System Layer
> **ヘッダ**: `system/[domain]/[module_name].hpp`
> **作成日**: YYYY-MM-DD
> **最終更新**: YYYY-MM-DD

---

## 1. 概要

### 1.1 目的

<!-- このモジュールが提供するプラットフォーム抽象化やシステム機能を記述する -->

### 1.2 分類

| 項目 | 値 |
|------|-----|
| ドメイン | `renderer` / `window` / `input` / `audio` |
| 満たす concept | `RendererBackend` / `InputBackend` / `WindowBackend` / その他 |
| プラットフォーム | `Desktop (Windows/Linux)` / `Android` / `iOS` / `Web` / `全プラットフォーム` |
| 外部依存 | (例: Vulkan SDK, GLFW, OpenSL ES) |

---

## 2. インターフェース仕様

### 2.1 concept 要件

<!-- このモジュールが満たすべき concept のメソッドを転記する -->

```cpp
// 例: RendererBackend concept
template<typename T>
concept RendererBackend = requires(T t) {
    { t.initialize() } -> std::same_as<bool>;
    { t.begin_frame() } -> std::same_as<void>;
    { t.end_frame() } -> std::same_as<void>;
    { t.shutdown() } -> std::same_as<void>;
};
```

### 2.2 公開メンバ

| メンバ名 | 型 | 説明 |
|----------|------|------|
| | | |

### 2.3 公開 API

<!-- concept 要件に加え、モジュール固有のパブリックメソッドを記述する -->

| メソッド | シグネチャ | 説明 |
|---------|----------|------|
| | | |

### 2.4 型定義 / 列挙型

<!-- モジュールが公開する型、enum、構造体を列挙する -->

| 型名 | 種別 | 説明 |
|------|------|------|
| | | |

---

## 3. 内部設計

### 3.1 内部状態

| メンバ名 | 型 | 説明 |
|----------|------|------|
| | | |

### 3.2 初期化シーケンス

```
1. [ステップ1: 例: ライブラリのロード]
2. [ステップ2: 例: デバイス選択]
3. [ステップ3: 例: リソースの確保]
```

### 3.3 シャットダウンシーケンス

```
1. [ステップ1: 例: リソースの解放]
2. [ステップ2: 例: デバイスの破棄]
3. [ステップ3: 例: ライブラリのアンロード]
```

### 3.4 フレームループ統合

<!-- begin_frame / end_frame / poll_events 等のフレームあたりの処理を記述する -->

---

## 4. プラットフォーム固有事項

### 4.1 条件コンパイル

```cpp
// 使用するプリプロセッサ定義
#if defined(ERGO_PLATFORM_WINDOWS)
    // Windows 固有実装
#elif defined(ERGO_PLATFORM_LINUX)
    // Linux 固有実装
#elif defined(ERGO_PLATFORM_ANDROID)
    // Android 固有実装
#endif
```

### 4.2 プラットフォーム依存の注意点

| プラットフォーム | 注意事項 |
|----------------|---------|
| Windows | |
| Linux | |
| Android | |
| iOS | |

---

## 5. エラーハンドリング

| エラー状況 | 対応方針 |
|-----------|---------|
| 初期化失敗 | `initialize()` が `false` を返す |
| デバイスロスト | |
| リソース不足 | |

---

## 6. 制約・前提条件

- <!-- 例: Vulkan 1.2 以上が必要 -->
- <!-- 例: initialize() は shutdown() より先に呼ぶこと -->
- <!-- 例: スレッドセーフではない (MainThread で使用) -->

---

## 7. 実装チェックリスト

- [ ] ヘッダファイル (`system/[domain]/[name].hpp`)
- [ ] 実装ファイル (`system/[domain]/[name].cpp`) — 必要な場合
- [ ] concept 準拠の確認
- [ ] 単体テスト (`tests/test_[domain].cpp` に追加)
- [ ] CMakeLists.txt への追加 (`system/CMakeLists.txt`)
- [ ] `platform.hpp` のプラットフォーム定義との整合性確認

---

## 8. Engine Layer との連携

<!-- このモジュールを Engine Layer からどう利用するかを記述する -->

| Engine 側コンポーネント | 利用方法 |
|----------------------|---------|
| | |

---

## 9. 使用例

```cpp
// 基本的な使用例を記述する
```

---

## 10. 変更履歴

| 日付 | 変更内容 |
|------|---------|
| YYYY-MM-DD | 初版作成 |
