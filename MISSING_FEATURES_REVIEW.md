# Ergo エンジン — 足りない機能レビュー

> **レビュー日**: 2026-02-12
> **対象**: DESIGN.md / IMPLEMENTATION_ROADMAP.md と実装コードの差分分析

---

## 総合評価

エンジン全体の完成度は **約45%**。Engine Layer の内部ロジック（タスク・物理・シェーダー・テキスト等）は高品質だが、**描画パイプライン（Vulkan）とプラットフォーム統合（ウィンドウ・入力）がスタブ状態**のため、実際にゲームを動作させることができない。

```
  実装済み・高品質   ████████████░░░░░░░░░░░░░  45%
  スタブ・TODO      ░░░░░░░░░░░░████████████░  40%
  未着手            ░░░░░░░░░░░░░░░░░░░░████░  15%
```

---

## 致命的な欠落（ゲーム動作に必須）

### 1. Vulkan レンダラー本体 [完成度: 5%]

| ファイル | 状態 | 問題点 |
|---------|------|--------|
| `system/renderer/vulkan/vk_renderer.cpp` | スタブ | VkInstance, VkDevice 等の初期化が一切なし。`StubRenderContext` で全描画メソッドが空 |
| `system/renderer/vulkan/vk_swapchain.cpp` | スタブ | width/height を保持するのみ。`vkAcquireNextImageKHR` 等が未実装 |
| `system/renderer/vulkan/vk_pipeline.cpp` | スタブ | `create_sprite_pipeline()` / `create_shape_pipeline()` が `return true` のみ |

**影響**: 画面に何も描画されない。ゲームループは動くが出力がない。

**必要な実装**:
- VkInstance / VkPhysicalDevice / VkDevice の作成
- Queue Family 検索・Queue 取得
- Surface 作成 (GLFW連携)
- Swapchain 作成・イメージ取得・再生成
- RenderPass / Framebuffer 作成
- CommandPool / CommandBuffer 管理
- 同期オブジェクト (Semaphore, Fence)
- `begin_frame()` / `end_frame()` の実フレーム処理

---

### 2. ウィンドウシステム (GLFW統合) [完成度: 15%]

| ファイル | 状態 | 問題点 |
|---------|------|--------|
| `system/window/desktop_window.cpp` | スタブ | メタデータ保持のみ。GLFW未導入。`get_surface_handle()` が常に `nullptr` |

**影響**: ウィンドウが開かない。Vulkan の Surface を取得できない。

**必要な実装**:
- GLFW の CMake 導入 (`FetchContent` or `find_package`)
- `glfwCreateWindow` によるウィンドウ生成
- `glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API)` で Vulkan 用設定
- リサイズコールバック
- `GLFWwindow*` を `get_surface_handle()` で返却

---

### 3. 入力システム [完成度: 35%]

| ファイル | 状態 | 問題点 |
|---------|------|--------|
| `system/input/desktop_input.cpp` | 部分実装 | キー状態配列・エッジ検出ロジックは存在するが、`poll_events()` が実際のキー状態を読み取らない |
| `system/input/touch_input.cpp` | スタブ | 全関数が `false` / 空のコレクションを返す |

**影響**: キーボード・マウス入力が一切取れない。

**必要な実装**:
- `DesktopInput::attach(GLFWwindow*)` メソッド追加
- GLFW キーコールバック / マウスコールバック接続
- `poll_events()` での状態更新

---

### 4. 2D 描画パイプライン [完成度: 0%]

IMPLEMENTATION_ROADMAP.md の A3 に記載の機能が全く存在しない。

**必要な実装**:
- 2D シェーダー (shape2d.vert / shape2d.frag)
- `VkBatch2D` — 動的バッチ頂点バッファ
- `draw_rect`, `draw_circle`, `draw_sprite` の実描画処理
- 正射影行列によるプッシュ定数

---

### 5. テクスチャ読み込み [完成度: 0%]

| ファイル | 状態 | 問題点 |
|---------|------|--------|
| `vk_renderer.cpp` の `load_texture()` | スタブ | `TextureHandle{0}` を返すのみ |

**必要な実装**:
- `stb_image` による画像読み込み (`engine/resource/image_loader`)
- VkImage / VkImageView / VkSampler 作成
- ステージングバッファ → GPU 転送
- ディスクリプタセット管理

---

## 重要な欠落（実用レベルに必要）

### 6. フォントレンダリング統合 [完成度: 50%]

テキストレイアウト・グリフ配置のエンジン内部ロジック (`engine/text/`) は **95% 完成**。しかし、Vulkan を通じた実際の描画ができないため、画面上にテキストは表示されない。

**必要な実装**:
- `FontAtlas` の VkImage 化
- `draw_text` 内で `VkBatch2D::push_sprite` 呼び出し

---

### 7. Time 管理 [完成度: 30%]

`runtime/main.cpp` に `FrameRateLimiter` の参照はあるが、`engine/core/time.hpp` に設計書レベルの Time 構造体（`total_time`, `time_scale`, `fps` 等）が不足。

**必要な実装**:
- IMPLEMENTATION_ROADMAP.md A6 に記載の `Time` 構造体
- FPS キャップ (`FrameRateLimiter`)
- `g_time` グローバルインスタンス

---

### 8. ログシステム [完成度: 0%]

`fprintf(stderr, ...)` を直接使用しており、設計書記載のログシステムが未実装。

**必要な実装**:
- IMPLEMENTATION_ROADMAP.md A7 に記載の `ergo::log` 名前空間
- レベル別ログ (Trace, Debug, Info, Warn, Error, Fatal)
- カテゴリフィルタ
- `ERGO_ASSERT` マクロ

---

### 9. カメラシステム [完成度: 0%]

射影行列はVulkan初期化時に固定設定の想定だが、そもそもVulkan初期化がないため存在しない。

**必要な実装**:
- `Camera2D` (スクロール・ズーム・シェイク)
- `Camera3D` (FPS/追従カメラ)
- ビュー射影行列計算

---

### 10. シーンマネージャー [完成度: 0%]

`StateMachine` が存在するが、IMPLEMENTATION_ROADMAP.md B4 に記載のスタック型シーン管理が未実装。

**必要な実装**:
- `SceneManager` (push/pop/change)
- フェードトランジション
- シーン並行管理

---

### 11. オーディオエンジン [完成度: 0%]

`system/audio/` に構造体定義のみ。miniaudio の導入・初期化が未着手。

**必要な実装**:
- miniaudio ヘッダ導入
- `AudioEngine` の初期化 / BGM 再生 / SE 再生
- WAV / MP3 / OGG 対応

---

### 12. スプライトアニメーション [完成度: 0%]

**必要な実装**:
- `SpriteAnimation` — フレームベースのUV切り替え
- `AnimationController` — 複数アニメーション管理

---

### 13. Tween / イージング [完成度: 0%]

**必要な実装**:
- `easing.hpp` — イージング関数集
- `TweenManager` — 値のアニメーション管理

---

## 部分的な欠落（プロダクション品質に必要）

### 14. ポストプロセス [完成度: 20%]

`engine/render/post_process.hpp` にエフェクト定義が存在するが、`apply()` が全て空実装。

**必要な実装**:
- オフスクリーン FBO
- フルスクリーンクワッド描画
- Fade / Bloom / Vignette / ToneMapping の実シェーダー

---

### 15. GPU 物理演算 [完成度: 5%]

`engine/physics/gpu_physics.cpp` に構造体があるが、GPU バッファ操作が全て TODO。

**必要な実装**:
- Vulkan コンピュートパイプライン
- SSBO へのデータ転送
- Dispatch + 結果読み取り

---

### 16. UIウィジェット [完成度: 40%]

`UIManager` のイベント処理は実装済みだが、具体的なウィジェット (Button, Slider, TextInput 等) が不足。

**必要な実装**:
- 各ウィジェットの描画ロジック
- レイアウト計算
- フォーカス管理

---

### 17. デバッグ描画 / ImGui 統合 [完成度: 25%]

`engine/debug/debug_draw.hpp/cpp` にフレームワークは存在するが、実描画がバックエンドに接続されていない。

**必要な実装**:
- Dear ImGui の導入
- `imgui_impl_glfw` + `imgui_impl_vulkan` の統合
- デバッグオーバーレイ表示

---

### 18. シリアライゼーション [完成度: 0%]

**必要な実装**:
- nlohmann/json 導入
- シーン / 設定 / パーティクル設定の読み書き

---

## 拡張機能の欠落

| 機能 | 完成度 | ロードマップ参照 |
|------|--------|----------------|
| Lua スクリプティング | 0% | D2 |
| ホットリロード | 0% | D3 |
| アセットパイプライン | 0% | D4 |
| Android/iOS プラットフォーム | 5% (スタブのみ) | D5 |
| ゲームパッド対応 | 0% | B1 |
| レイキャスト (2D) | 0% | B8 |
| 空間分割 (SpatialGrid) | 一部あり | B8 |
| ライティング (3D) | ヘッダ定義のみ | C4 |
| プロファイラー | ヘッダ定義のみ | C8 |

---

## 実装済み・高品質なモジュール

以下は設計通りまたはそれ以上の品質で実装されている:

| モジュール | 完成度 | 備考 |
|-----------|--------|------|
| タスクシステム (`engine/core/task_system`) | 80-85% | 型消去・レイヤー管理・ライフサイクル |
| 物理システム (`engine/physics/`) | 85% | AABB/Circle 衝突・コールバック・空間分割 |
| シェーダーコンパイラ (`engine/shader/`) | 85-90% | GLSL/WGSL 生成・ノードグラフ・最適化 |
| テキストレイアウト (`engine/text/`) | 90-95% | UTF-8・CJK・ワードラップ・リッチテキスト |
| HTTP クライアント (`engine/net/`) | 95% | POCO + POSIX ソケットデュアルバックエンド |
| ECS システム (`engine/ecs/`) | 85% | Archetype ベース・Dense ストレージ |
| スケルタルアニメーション (`engine/animation/`) | 90% | キーフレーム補間・SLERP・ボーン行列 |
| パーティクルシステム (`engine/render/particle_system`) | 90% | エミッター・色補間・バースト |
| スキンドメッシュレンダラー (`engine/render/skinned_mesh_renderer`) | 85% | スケルトン・再生制御 |
| DLL ローダー (`runtime/dll_loader`) | 95% | Windows/Linux クロスプラットフォーム |
| テストスイート (`tests/`) | 90% | 数学・物理・ECS の網羅的テスト |
| Web 実装 (`web/`) | 70% | TypeScript ポート・WebGPU 基盤 |

---

## 優先実装ロードマップ（推奨順序）

```
[Phase 1] 画面を出す ─────────────────────────────────────
  ① GLFW 統合        → ウィンドウが開く
  ② Vulkan 初期化     → 画面がクリアされる
  ③ 2D 描画パイプライン → 矩形・円が表示される
  ④ テクスチャ         → スプライトが表示される
  ⑤ 入力接続          → キーボード/マウスが動作する

[Phase 2] ゲームとして動く ────────────────────────────────
  ⑥ フォント描画統合    → テキスト表示
  ⑦ Time 管理         → 安定したフレームレート
  ⑧ ログシステム       → デバッグ効率向上
  ⑨ カメラシステム     → スクロール・ズーム
  ⑩ オーディオ         → BGM/SE 再生
  ⑪ シーン管理         → 画面遷移

[Phase 3] 品質向上 ────────────────────────────────────────
  ⑫ UI ウィジェット
  ⑬ ポストプロセス
  ⑭ ImGui 統合
  ⑮ シリアライゼーション
  ⑯ アニメーション・Tween
```

---

## 設計と実装の乖離まとめ

| 設計書の記述 | 現実 |
|-------------|------|
| 「ウィンドウが開き、画面に矩形・円・テキストが描画される」(Phase A ゴール) | ウィンドウも描画も動作しない |
| 「Vulkan の初期化〜画面クリアまでが動作する」(A2 成果物) | Vulkan API の呼び出しが一切ない |
| 「GLFW を使った実装」(A1 成果物) | GLFW 未導入 |
| 「PNG/JPG ファイルを VkImage に読み込み」(A4 成果物) | `load_texture` が `{0}` を返すのみ |
| 「サンプルシューティングゲームが完全に動作する」(Phase B ゴール) | シーン骨格のみ、ゲームプレイなし |

**結論**: エンジンの「頭脳」（ロジック・アルゴリズム）は高品質だが、「手足」（描画・入力・プラットフォーム）が動作しない状態。Phase A の GLFW + Vulkan 統合が最優先課題。
