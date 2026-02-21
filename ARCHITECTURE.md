# Ergo — アーキテクチャ & コンポーネント/プラグイン要件書

> **対象バージョン**: 現行実装 (2026-02-21 時点)
> **前提ドキュメント**: `DESIGN.md` (設計書)

---

## 目次

1. [全体アーキテクチャ](#1-全体アーキテクチャ)
2. [レイヤー構成と依存関係](#2-レイヤー構成と依存関係)
3. [設計原則](#3-設計原則)
4. [Engine Layer](#4-engine-layer)
5. [System Layer](#5-system-layer)
6. [Application Layer (Runtime)](#6-application-layer-runtime)
7. [Game DLL インターフェース](#7-game-dll-インターフェース)
8. [ECS (Entity Component System)](#8-ecs-entity-component-system)
9. [コンポーネント/プラグイン要件](#9-コンポーネントプラグイン要件)
10. [グローバルシステム一覧](#10-グローバルシステム一覧)
11. [ビルド構成](#11-ビルド構成)
12. [Web 版 (TypeScript)](#12-web-版-typescript)

---

## 1. 全体アーキテクチャ

Ergo は **軽量クロスプラットフォーム 2D/3D ゲームエンジン** である。
C++20 の `concept` による型制約を設計の中核に据え、**継承を使用しない** アーキテクチャを採用している。

```
┌─────────────────────────────────────────────────────────────┐
│  Application Layer (ゲーム DLL)                              │
│  ゲームロジック / シーン / ゲームオブジェクト                     │
│                                                             │
│  ・ergo_get_game_callbacks() で C API を介してエンジンと接続    │
│  ・concept 準拠の型を内部で自由に使用可能                       │
├──────────────── C API (DLL 境界) ────────────────────────────┤
│  Runtime (実行ファイル)                                       │
│  main.cpp / DLL ローダー / EngineContext                      │
├─────────────────────────────────────────────────────────────┤
│  Engine Layer (libErgoEngine.a)                              │
│  core/ math/ physics/ render/ resource/ ecs/ ui/ net/        │
│  animation/ text/ shader/ behaviour/ debug/                  │
├──────────── concept 準拠の抽象インターフェース ─────────────────┤
│  System Layer (libErgoSystem.a)                              │
│  renderer/vulkan/ input/ window/ audio/                      │
└─────────────────────────────────────────────────────────────┘
```

### ディレクトリ構成

```
Ergo/
├── engine/                 # Engine Layer (静的ライブラリ)
│   ├── core/               #   タスクシステム, ステートマシン, ジョブシステム, 入力マップ等
│   │   └── behaviour/      #   ビヘイビアツリー, プレイヤーコントローラー等
│   ├── math/               #   Vec2f, Vec3f, Mat4, Transform2D/3D, Quat, Color
│   ├── physics/            #   2D コライダー, 3D リジッドボディ, 空間分割, レイキャスト
│   ├── render/             #   レンダーパイプライン, コマンドバッファ, メッシュ, パーティクル
│   ├── resource/           #   リソースマネージャー, テクスチャ/フォントローダー
│   ├── ecs/                #   アーキタイプベース ECS (World, Archetype)
│   ├── ui/                 #   UINode 階層, Canvas, ウィジェット
│   ├── net/                #   TCP/UDP ソケット, HTTP クライアント, ネットワークマネージャー
│   ├── animation/          #   AnimationPlayer, AnimationClip, Skeleton
│   ├── text/               #   テキストレイアウト, リッチテキスト, テキストレンダラー
│   ├── shader/             #   シェーダーライブラリ, コンパイラ, オプティマイザ
│   └── debug/              #   デバッグ描画, プロファイラー
│
├── system/                 # System Layer (静的ライブラリ)
│   ├── platform.hpp        #   コンパイル時プラットフォーム選択
│   ├── renderer/vulkan/    #   VulkanRenderer, VkPipeline, VkSwapchain
│   ├── window/             #   DesktopWindow (GLFW)
│   ├── input/              #   DesktopInput, TouchInput, GamepadInput
│   └── audio/              #   (スタブ)
│
├── runtime/                # Application Assembly (実行ファイル)
│   ├── main.cpp            #   メインループ
│   ├── engine_context.hpp  #   C API ブリッジ構築
│   └── dll_loader.hpp      #   DLL 動的ロード (dlopen/LoadLibrary)
│
├── game_interface/         # DLL 境界定義 (C API)
│   ├── engine_types.h      #   C 構造体 (ErgoVec2, ErgoNetMessage 等)
│   └── game_interface.h    #   ErgoEngineAPI / ErgoGameCallbacks
│
├── samples/shooting/       # サンプルゲーム (シューティング)
├── web/                    # Web 版 (TypeScript/WebGPU)
├── editor/                 # MAUI エディタ
├── tests/                  # テストアセンブリ
└── demos/                  # フィーチャーデモ
```

---

## 2. レイヤー構成と依存関係

### 依存方向

```
Application Layer (Game DLL)
        │
        │  C API (game_interface.h) 経由
        ▼
Runtime (main.cpp)
        │
        │  直接参照
        ▼
Engine Layer ──── concept インターフェース ────▶ System Layer
```

- **上位レイヤーは下位レイヤーに依存**する。逆方向の依存は禁止。
- Engine Layer と System Layer の接続は **concept** (`RendererBackend`, `InputBackend`, `WindowBackend`) によって抽象化される。
- Game DLL とエンジンの接続は **C API** (`ErgoEngineAPI` / `ErgoGameCallbacks`) で行い、ABI 互換性を保証する。

### ビルドターゲット

| ターゲット | 描画 | コンパイラ | 配布形式 |
|-----------|------|----------|---------|
| Windows | Vulkan | MSVC / Clang | `.exe` + `.dll` |
| Linux | Vulkan | GCC / Clang | ELF + `.so` |
| Android | Vulkan | NDK (Clang) | `.apk` (`.so`) |
| iOS | MoltenVK | Xcode (Clang) | `.ipa` (`.dylib`) |
| Web | WebGPU | TypeScript | `.js` |

---

## 3. 設計原則

### 3.1 継承禁止 — concept + std::variant で代替

Ergo では仮想関数による継承階層を使用しない。代わりに以下の手法を用いる。

| 従来の手法 | Ergo での代替 | 用途 |
|-----------|-------------|------|
| 仮想基底クラス | C++20 `concept` | コンパイル時インターフェース制約 |
| 継承ポリモーフィズム | `std::variant` + `std::visit` | 有限種の実行時多態 (コライダー形状、レンダーコマンド等) |
| 仮想関数テーブル | type-erasure (`ITask`/`TaskModel<T>`) | 異種型の単一コンテナ格納 (TaskManager) |
| Singleton パターン | `inline` グローバル変数 | `g_physics`, `g_time`, `g_resources` 等 |

### 3.2 データ指向設計 (DOD)

- **SOA (Structure of Arrays)**: ECS の Archetype ストレージは列方向にコンポーネントを配置
- **キャッシュライン整列**: JobSystem のチャンクサイズはキャッシュライン倍数 (デフォルト 64 エンティティ)
- **ダブルバッファリング**: レンダーコマンドはロックフリーのバッファ交換で受け渡し
- **空間分割**: 2D 物理のブロードフェーズに空間グリッドを使用

### 3.3 スレッディングポリシー

すべての型はスレッド安全性を明示的に宣言できる。

```cpp
enum class ThreadingPolicy : uint8_t {
    MainThread,  // メインスレッド限定 (入力, UI, レンダー状態)
    AnyThread,   // 任意の単一スレッドで安全 (共有可変状態なし)
    Parallel,    // データ並列実行向け (ECS クエリ等)
};

template<typename T>
concept ThreadAware = requires {
    { T::threading_policy() } -> std::same_as<ThreadingPolicy>;
};
```

`TaskManager` は各タスクの `ThreadingPolicy` を取得し、スレッディングレポートを生成できる。

---

## 4. Engine Layer

### 4.1 Concept 定義 (`engine/core/concepts.hpp`)

エンジン全体の型制約を定義する中核ファイル。

#### ライフサイクル系

| Concept | 要求 | 説明 |
|---------|------|------|
| `Startable` | `t.start() → void` | 初期化 |
| `Updatable` | `t.update(float) → void` | 毎フレーム更新 |
| `Drawable` | `t.draw(RenderContext&) → void` | 描画 (オプショナル) |
| `Releasable` | `t.release() → void` | 解放 |
| **`TaskLike`** | `Startable && Updatable && Releasable` | **タスク登録の最小要件** |
| `BehaviourLike` | `TaskLike` + `T::type_name() → string_view` | ビヘイビアコンポーネント |

#### ゲームオブジェクト系

| Concept | 要求 | 説明 |
|---------|------|------|
| `GameObjectLike` | `transform()`, `name()`, `object_type()` | ゲームオブジェクトの制約 |
| `ColliderLike` | `is_hit()`, `tag()`, `owner_transform()` | コライダーの制約 |

#### システムバックエンド系

| Concept | 要求 | 説明 |
|---------|------|------|
| `RendererBackend` | `initialize()`, `begin_frame()`, `end_frame()`, `shutdown()` | 描画バックエンド |
| `InputBackend` | `is_key_down()`, `is_key_pressed()`, `mouse_position()`, `poll_events()` | 入力バックエンド |
| `WindowBackend` | `create()`, `should_close()`, `poll_events()`, `width()`, `height()` | ウィンドウバックエンド |

#### 物理系

| Concept | 要求 | 説明 |
|---------|------|------|
| `PhysicsSteppable` | `start()`, `update(float)`, `release()` | 物理コンポーネント |
| `PhysicsBodyProvider` | `body_count()`, `remove_body(uint64_t)` | 物理ボディ管理 |
| `PhysicsForceApplicable` | `apply_force()`, `apply_impulse()` | 力の適用 |

#### レンダー/シェーダー系

| Concept | 要求 | 説明 |
|---------|------|------|
| `CommandSubmittable` | `record_commands(CommandBuffer&)` | コマンドバッファ記録 |
| `ShaderComposable` | `compile()`, `vertex_source()`, `fragment_source()`, `is_compiled()` | シェーダーコンパイル |
| `ShaderOptimizable` | `optimization_report()`, `is_optimized()` | シェーダー最適化 |

#### ネットワーク系

| Concept | 要求 | 説明 |
|---------|------|------|
| `NetworkPollable` | `poll()`, `shutdown()`, `is_active()` | ネットワークポーリング |
| `SocketConnectable` | `connect()`, `close()`, `is_connected()` | ソケット接続 |
| `SocketListenable` | `listen()`, `close()` | ソケットリッスン |
| `StreamSendable` | `send(data, len)` | TCP ストリーム送信 |
| `StreamReceivable` | `recv(buf, max_len)` | TCP ストリーム受信 |
| `DatagramSendable` | `send_to(data, len, host, port)` | UDP 送信 |
| `DatagramReceivable` | `recv_from(buf, max_len, host, port)` | UDP 受信 |
| `NetworkManageable` | `connect()`, `host_server()`, `send()`, `poll()`, `shutdown()` | 高レベルネットワーク管理 |
| `HttpRequestable` | `get(url)`, `post(url, body)` | HTTP リクエスト |

### 4.2 タスクシステム (`engine/core/task_system.hpp`)

ゲームオブジェクトのライフサイクルを管理する中核システム。

#### type-erasure パターン

```
TaskLike concept (コンパイル時制約)
    │
    ▼
TaskModel<T> : ITask (concept → virtual ブリッジ)
    │
    ▼
TaskManager (異種型を単一コンテナで管理)
```

- `ITask` は内部 virtual インターフェース (DLL 境界には露出しない)
- `TaskModel<T>` は `if constexpr` で `Drawable`, `ThreadAware` 等のオプショナル機能を条件分岐
- 具体型 `T` は `TaskLike` concept を満たせば任意

#### レイヤーとフェーズ

```cpp
enum class TaskLayer : uint32_t {
    Default = 0,   // 一般的なゲームオブジェクト
    Bullet,        // 弾丸 (大量生成・破棄)
    Physics,       // 物理タスク
    UI,            // UI 要素
    Max
};

enum class RunPhase {
    Start,    // 初期化 (未初期化タスクの start() 呼び出し)
    Update,   // 毎フレーム更新
    Physics,  // 物理ステップ
    Draw,     // 描画
    Destroy   // 破棄予約済みタスクの release() と削除
};
```

#### 登録と破棄

```cpp
// 登録: T は TaskLike を満たす任意の型
template<TaskLike T, typename... Args>
TaskHandle register_task(TaskLayer layer, Args&&... args);

// 破棄予約
void destroy(TaskHandle handle);

// 実行
void run(RunPhase phase, float dt, RenderContext* ctx = nullptr);
```

### 4.3 ジョブシステム (`engine/core/job_system.hpp`)

データ並列処理のためのワーカースレッドプール。

```cpp
class JobSystem {
    void initialize(uint32_t thread_count = 0);  // 0 = 自動検出
    void parallel_for(uint32_t begin, uint32_t end, uint32_t chunk_size,
                      std::function<void(uint32_t, uint32_t)> fn);
    void submit(std::function<void()> fn);
    void wait();
};

inline JobSystem g_job_system;
```

- ECS の `parallel_each` やレンダーパイプラインのジョブディスパッチで使用
- チャンクサイズはキャッシュライン倍数に調整可能

### 4.4 ステートマシン (`engine/core/state_machine.hpp`)

`std::variant` ベースの型安全なステートマシン。

```cpp
template<typename... States>
class StateMachine {
    std::variant<std::monostate, States...> current_;

    template<typename S, typename... Args>
    void transition(Args&&... args);  // exit → emplace → enter
    void update(float dt);
    void draw(auto& ctx);
    template<typename S> bool is_state() const;
};
```

各ステート型は以下のメソッドを持つことが期待される (if constexpr で条件分岐):
- `enter()` — 必須
- `update(float dt)` — 必須
- `exit()` — オプション
- `draw(auto& ctx)` — オプション

### 4.5 ゲームオブジェクト (`engine/core/game_object.hpp`)

`std::any` によるコンポーネント集約型ゲームオブジェクト。

```cpp
struct GameObject {
    uint64_t id;
    std::string name_;
    uint32_t object_type_;
    Transform2D transform_;
    std::unordered_map<std::type_index, std::any> components_;

    template<typename T> void add_component(T&& comp);
    template<typename T> T* get_component();
};
```

- `GameObjectLike` concept を満たす
- 任意の型をコンポーネントとして追加可能
- ECS (`World`) とは独立した軽量コンポーネントモデル

### 4.6 シーンマネージャー (`engine/core/scene_manager.hpp`)

スタックベースのシーン管理。

```cpp
struct Scene {
    virtual void on_enter() = 0;
    virtual void on_exit() = 0;
    virtual void on_update(float dt) = 0;
    virtual void on_draw(RenderContext& ctx) = 0;
    virtual void on_pause() {}   // スタックで上にシーンが積まれた時
    virtual void on_resume() {}  // 上のシーンが pop された時
};

class SceneManager {
    void change(std::unique_ptr<Scene> scene, float fade_duration = 0.0f);
    void push(std::unique_ptr<Scene> scene);
    void pop();
};
```

**注意**: `Scene` はエンジン内で唯一の仮想基底クラス。DLL 境界をまたがないエンジン内部使用のため許容している。フェードトランジション (`FadeOut → FadeIn`) を内蔵。

### 4.7 入力マップ (`engine/core/input_map.hpp`)

アクション/軸ベースの入力マッピング。

```cpp
struct InputAction {
    std::string name;
    std::vector<uint32_t> keys;           // 複数キーバインド
    std::vector<uint32_t> gamepad_buttons;
    int gamepad_axis = -1;
    float dead_zone = 0.15f;
};

class InputMap {
    void register_action(InputAction action);
    bool is_action_down(std::string_view name) const;
    bool is_action_pressed(std::string_view name) const;
    float get_axis(std::string_view name) const;
};
```

### 4.8 時間管理 (`engine/core/time.hpp`)

```cpp
struct Time {
    float delta_time;           // タイムスケール適用済み
    float unscaled_delta_time;  // 生のフレームデルタ
    float total_time;           // 累積時間
    float time_scale;           // 速度倍率 (0.0 = ポーズ)
    float fixed_delta_time;     // 物理タイムステップ (1/60)
    uint64_t frame_count;
    float fps;                  // 指数移動平均 FPS
};

inline Time g_time;
```

### 4.9 トゥイーン (`engine/core/tween.hpp`)

イージング関数付きの値アニメーション。

```cpp
class TweenManager {
    Tween& add(float* target, float from, float to, float duration,
               EasingFunc ease = easing::linear);
    void update(float dt);
};

inline TweenManager g_tweens;
```

イージング関数: `linear`, `in_quad`, `out_quad`, `in_out_quad`, `in_cubic`, `out_bounce`, `in_elastic` 等多数。

### 4.10 物理システム

#### 2D 物理 (`engine/physics/`)

```cpp
// コライダー形状 — std::variant で表現
using ColliderShape = std::variant<AABBData, CircleData>;

struct Collider {
    ColliderHandle handle;
    ColliderShape shape;             // AABB or Circle
    ColliderTag tag;                 // Player, Enemy, Bullet 等
    uint64_t owner_id;
    const Transform2D* transform;   // オーナーの Transform への非所有ポインタ
    std::function<bool(const Collider&)> on_hit;  // 衝突コールバック
};

class PhysicsSystem {
    ColliderHandle register_collider(Collider& c);
    void remove_collider(Collider& c);
    void mark_moved(Collider& c);   // ブロードフェーズ候補に追加
    void run();                      // 衝突検出 + コールバック実行
};

inline PhysicsSystem g_physics;
```

- `ColliderTag` ごとにコライダーをグループ化
- `mark_moved()` で移動したコライダーのみ判定 (CalcStack パターン)
- 空間グリッド (`SpatialGrid`) によるブロードフェーズ最適化

#### 3D 物理 (`engine/physics/rigid_body_world.hpp`)

```cpp
struct RigidBody {
    RigidBodyType type;        // Static / Dynamic
    float mass, inv_mass;
    float restitution, friction;
    Vec3f velocity, acceleration;
    Vec3f angular_velocity;
    Vec3f force_accumulator, torque_accumulator;
    float linear_damping, angular_damping;
    float gravity_scale;
    bool is_sleeping;
};

class RigidBodyWorld {
    uint64_t add_body(PhysicsBody body);
    void remove_body(uint64_t id);
    void step(float dt);           // 固定タイムステップ蓄積
    void set_gravity(Vec3f g);
};

inline RigidBodyWorld g_rigid_body_world;
```

- 固定タイムステップ蓄積 (`accumulator_`) + 最大サブステップ制限
- 衝突検出 → 衝突応答 → スリープ判定 のパイプライン

### 4.11 レンダーパイプライン (`engine/render/`)

#### レンダーコマンド — std::variant

```cpp
using RenderCommand = std::variant<
    RenderCmd_Clear,
    RenderCmd_SetViewProjection,
    RenderCmd_DrawMesh,
    RenderCmd_DrawSkinnedMesh,
    RenderCmd_DrawRect,
    RenderCmd_DrawCircle,
    RenderCmd_DrawSprite,
    RenderCmd_DrawText,
    RenderCmd_DrawDebugLine,
    RenderCmd_DrawTextBatch
>;
```

#### マルチスレッドレンダーパイプライン

```cpp
class RenderPipeline {
    enum class Stage : uint32_t {
        Shadow, Opaque, Transparent, PostProcess, UI, Max
    };

    void initialize(uint32_t worker_count = 0);
    void begin_frame();          // バッファスワップ
    void submit(Stage stage, const CommandBuffer& buffer);
    void dispatch_jobs(const std::vector<RenderJob>& jobs);
    void wait_for_jobs();
    void end_frame();            // コマンド収集完了
};
```

- ステージごとにダブルバッファリングされたコマンドストリーム
- ワーカースレッドがコマンドを並列生成 → マージ
- レンダースレッドはフロントバッファを消費

#### ポストプロセス (`engine/render/post_process.hpp`)

```
PostProcessStack
  ├── FadeEffect        (シーン遷移フェード)
  ├── VignetteEffect    (ビネット)
  ├── BloomEffect       (ブルーム)
  └── ColorGradeEffect  (カラーグレーディング)
```

### 4.12 リソースマネージャー (`engine/resource/resource_manager.hpp`)

```cpp
class ResourceManager {
    TextureHandle load_texture(std::string_view path);  // 参照カウント
    void release_texture(TextureHandle handle);
    FontAtlas* load_font(std::string_view ttf_path, float size);
    void collect_garbage();   // ref_count == 0 のリソースを解放
    void shutdown();          // 全リソース解放
};

inline ResourceManager g_resources;
```

- パスをキーとした重複ロード防止 (参照カウント方式)
- テクスチャ、フォント (FreeType/Harfbuzz) をサポート

### 4.13 UI システム (`engine/ui/`)

Unity uGUI インスパイアの **RectTransform** ベース UI。

```cpp
struct UIRectTransform {
    Vec2f anchor_min, anchor_max;  // 親内の正規化位置 (0..1)
    Vec2f pivot;                    // ノード内のオリジン (0..1)
    Vec2f position;                 // アンカーからのオフセット (px)
    Size2f size_delta;              // 固定サイズ or インセット
};

class UINode {
    UIRectTransform rect_;
    std::vector<std::unique_ptr<UINode>> children_;
    UINode* parent_;

    UINode* add_child(std::unique_ptr<UINode> child);
    WorldRect compute_world_rect(const WorldRect& parent_rect) const;
    UINode* hit_test(Vec2f pos, const WorldRect& parent_rect);
};
```

- ノード階層 (`UINode` → children)
- `UICanvas` がルートキャンバス
- ウィジェット: `UIButton`, `UILabel`, `UIImageNode` 等

### 4.14 ビヘイビアツリー (`engine/core/behaviour/behaviour_tree.hpp`)

`std::variant` ベースの AI 意思決定ツリー。

```
BTNode (variant)
  ├── BTAction     : ユーザー定義関数を実行
  ├── BTCondition  : 述語を評価
  ├── BTWait       : 一定時間待機
  ├── BTSequence   : 子を順次実行 (最初の失敗で中断)
  ├── BTSelector   : 子を順次実行 (最初の成功で終了)
  ├── BTRepeater   : 子を N 回繰り返す
  └── BTInverter   : 子の結果を反転
```

- ファクトリメソッドによるツリー構築 (`BTNode::make_action()`, `make_sequence()` 等)
- `BehaviourTree` は `TaskLike` + `ThreadAware` を満たし、`TaskManager` に登録可能

### 4.15 ネットワーク (`engine/net/`)

```
net/
├── net_concepts.hpp     # ネットワーク concept 群
├── socket.hpp/cpp       # POSIX/Winsock ベースソケット
├── tcp_socket.hpp/cpp   # TCP ストリームソケット
├── udp_socket.hpp/cpp   # UDP データグラムソケット
├── http_client.hpp/cpp  # HTTP GET/POST (POCO or ネイティブ)
└── network_manager.hpp  # 高レベル接続管理
```

- POCO ライブラリが利用可能な場合は POCO バックエンド、なければ POSIX/Winsock フォールバック
- C API 経由でゲーム DLL からも利用可能 (`net_connect`, `net_send`, `http_get` 等)

### 4.16 デバッグ/プロファイリング (`engine/debug/`)

```cpp
class Profiler {
    void begin(const char* name);
    void end();
    const std::unordered_map<std::string, float>& results() const;
};
inline Profiler g_profiler;

// RAII スコープ計測
#define ERGO_PROFILE_SCOPE(name) ScopedProfile _profile_##__LINE__(name)
```

- メインループで `Destroy`, `Physics`, `Update`, `Draw` 各フェーズを計測
- `DebugDraw` でデバッグ用の線、形状、テキストを描画

---

## 5. System Layer

プラットフォーム固有の実装をコンパイル時に選択する。

```cpp
// system/platform.hpp
namespace ergo {
    using PlatformRenderer = VulkanRenderer;   // RendererBackend concept を満たす
    using PlatformInput    = DesktopInput;      // InputBackend concept を満たす
    using PlatformWindow   = DesktopWindow;     // WindowBackend concept を満たす
}
```

### 5.1 Vulkan レンダラー (`system/renderer/vulkan/`)

| ファイル | 責務 |
|---------|------|
| `vk_renderer.hpp/cpp` | VkInstance / VkDevice / VkSwapchain の初期化・管理 |
| `vk_pipeline.hpp/cpp` | パイプライン作成 |
| `vk_swapchain.hpp/cpp` | スワップチェーン管理 |

### 5.2 入力 (`system/input/`)

| ファイル | 責務 |
|---------|------|
| `desktop_input.hpp/cpp` | GLFW キーボード/マウス入力 |
| `touch_input.hpp/cpp` | Android/iOS タッチ入力 |
| `gamepad_input.hpp/cpp` | ゲームパッド入力 |

### 5.3 ウィンドウ (`system/window/`)

| ファイル | 責務 |
|---------|------|
| `desktop_window.hpp/cpp` | GLFW ウィンドウ管理 |

---

## 6. Application Layer (Runtime)

### メインループ (`runtime/main.cpp`)

```
1. プラットフォーム初期化
   └── Window作成 → Renderer初期化 → Input初期化

2. エンジンシステム初期化
   └── JobSystem → RenderPipeline → TaskManager → FrameRateLimiter

3. ゲーム DLL ロード
   └── load_game_dll() → on_init(&engine_api)

4. メインループ (毎フレーム)
   ├── g_time.tick(dt)
   ├── window.poll_events() / input.poll_events()
   ├── [Destroy] task_mgr.run(Destroy)
   ├── [Physics] task_mgr.run(Physics) → g_physics.run() → g_rigid_body_world.step()
   ├── [Update]  task_mgr.run(Update) → game.on_update() → g_tweens.update()
   ├── [Draw]    render_pipeline.begin_frame() → task_mgr.run(Draw) → game.on_draw() → end_frame()
   └── fps_limiter.wait()

5. シャットダウン
   └── game.on_shutdown() → unload DLL → g_resources → render_pipeline → g_job_system → renderer
```

### フェーズ実行順序

```
Destroy → Physics → Update → Draw
```

この順序により、破棄されたタスクが物理・更新・描画に影響しないことを保証する。

---

## 7. Game DLL インターフェース

### C API 構造体 (`game_interface/engine_types.h`)

```c
typedef struct { float x, y; } ErgoVec2;
typedef struct { float w, h; } ErgoSize2;
typedef struct { uint8_t r, g, b, a; } ErgoColor;
typedef struct { ErgoVec2 position; float rotation; ErgoSize2 size; } ErgoTransform2D;
typedef struct { uint64_t id; } ErgoTaskHandle;
typedef struct { uint64_t id; } ErgoColliderHandle;
typedef struct { uint64_t id; } ErgoTextureHandle;
typedef struct { uint16_t type; const uint8_t* payload; uint32_t payload_len; } ErgoNetMessage;
typedef struct { int status_code; const char* body; uint32_t body_len; } ErgoHttpResponse;
```

### エンジン API (`game_interface/game_interface.h`)

```c
typedef struct {
    // 描画
    void (*draw_rect)(...);
    void (*draw_circle)(...);
    void (*draw_text)(...);

    // 入力
    int (*is_key_down)(uint32_t key);
    int (*is_key_pressed)(uint32_t key);
    ErgoVec2 (*mouse_position)(void);

    // リソース
    ErgoTextureHandle (*load_texture)(const char* path);
    void (*unload_texture)(ErgoTextureHandle handle);

    // ネットワーク
    int (*net_connect)(const char* host, uint16_t port);
    int (*net_host)(uint16_t port, int max_clients);
    void (*net_send)(ErgoNetMessage msg, uint32_t client_id);
    void (*net_poll)(void);
    void (*net_shutdown)(void);
    void (*net_set_handler)(uint16_t msg_type, ErgoNetMessageCallback callback);
    void (*net_set_event_handler)(ErgoNetEventCallback callback);
    ErgoHttpResponse (*http_get)(const char* url);
    ErgoHttpResponse (*http_post)(const char* url, const char* body, const char* content_type);
} ErgoEngineAPI;
```

### ゲームコールバック

```c
typedef struct {
    void (*on_init)(const ErgoEngineAPI* api);  // エンジン API の受け取り
    void (*on_update)(float dt);                 // 毎フレーム更新
    void (*on_draw)(void);                       // 描画
    void (*on_shutdown)(void);                   // 終了処理
} ErgoGameCallbacks;

// DLL エントリーポイント
ERGO_EXPORT ErgoGameCallbacks* ergo_get_game_callbacks(void);
```

### DLL 境界の設計方針

- **C 互換型のみ**使用 (C++ テンプレート、STL コンテナは渡さない)
- **関数ポインタテーブル**でエンジン機能を提供
- ゲーム DLL 内部では C++ 機能 (concept, variant 等) を自由に使用可能
- `ERGO_EXPORT` マクロで Windows (`__declspec(dllexport)`) / Unix (`__attribute__((visibility("default")))`) を切り替え

---

## 8. ECS (Entity Component System)

### アーキタイプベース SOA ストレージ (`engine/ecs/`)

```cpp
class World {
    uint64_t create_entity();
    void destroy_entity(uint64_t id);

    template<typename T> void add_component(uint64_t entity, T component);
    template<typename T> T* get_component(uint64_t entity);
    template<typename T> bool has_component(uint64_t entity) const;

    // 逐次クエリ
    template<typename... Ts, typename Func>
    void each(Func&& fn);

    // 並列クエリ (JobSystem 連携)
    template<typename... Ts, typename Func>
    void parallel_each(Func&& fn, uint32_t chunk_size = 64);
};
```

#### アーキタイプの仕組み

```
Entity: {Position, Velocity, Health}  →  Archetype A
Entity: {Position, Sprite}            →  Archetype B

Archetype A:
  entities:  [e1, e2, e3, ...]
  columns:
    Position:  [p1, p2, p3, ...]   ← SOA 配置 (連続メモリ)
    Velocity:  [v1, v2, v3, ...]
    Health:    [h1, h2, h3, ...]
```

- コンポーネント追加/削除時にエンティティは適切なアーキタイプへマイグレーション
- `parallel_each` は各アーキタイプのエンティティをチャンク分割し、`g_job_system.parallel_for()` で並列処理
- チャンクサイズ 64 がデフォルト (64 byte コンポーネントで ~4KB = L1 キャッシュ収まり)

### GameObject vs ECS の使い分け

| 特性 | GameObject | World (ECS) |
|------|-----------|-------------|
| ストレージ | `std::any` マップ | SOA アーキタイプ |
| パフォーマンス | 少数オブジェクト向け | 大量エンティティ向け |
| クエリ | 個別アクセス | バッチイテレーション |
| 並列化 | 非対応 | `parallel_each` 対応 |
| 用途 | プレイヤー、UIなど | 弾丸、パーティクル、敵群など |

---

## 9. コンポーネント/プラグイン要件

### 9.1 新しいタスク型を作る要件

`TaskManager` に登録する型は `TaskLike` concept を満たす必要がある。

```cpp
struct MyTask {
    // ── 必須 (TaskLike) ──
    void start();
    void update(float dt);
    void release();

    // ── オプション ──
    void draw(RenderContext& ctx);     // Drawable → 描画フェーズで呼ばれる
    void physics(float dt);            // 物理フェーズで呼ばれる

    // ── スレッディング宣言 (オプション) ──
    static constexpr ThreadingPolicy threading_policy() {
        return ThreadingPolicy::AnyThread;
    }
};

// 登録
task_mgr.register_task<MyTask>(TaskLayer::Default, /* constructor args */);
```

### 9.2 新しいビヘイビアを作る要件

`BehaviourLike` concept を満たす必要がある。

```cpp
struct MyBehaviour {
    // ── TaskLike 部分 ──
    void start();
    void update(float dt);
    void release();

    // ── BehaviourLike 追加要件 ──
    static constexpr std::string_view type_name() { return "MyBehaviour"; }
};
```

### 9.3 新しいレンダーバックエンドを作る要件

`RendererBackend` concept を満たす。

```cpp
struct MyRenderer {
    bool initialize();
    void begin_frame();
    void end_frame();
    void shutdown();
};
```

### 9.4 新しい入力バックエンドを作る要件

`InputBackend` concept を満たす。

```cpp
struct MyInput {
    bool is_key_down(uint32_t key);
    bool is_key_pressed(uint32_t key);
    Vec2f mouse_position();
    void poll_events();
};
```

### 9.5 新しいウィンドウバックエンドを作る要件

`WindowBackend` concept を満たす。

```cpp
struct MyWindow {
    bool create(uint32_t w, uint32_t h, std::string_view title);
    bool should_close();
    void poll_events();
    uint32_t width();
    uint32_t height();
};
```

### 9.6 新しいコライダー形状を追加する要件

1. 形状データ構造体を定義
2. `ColliderShape` variant に追加
3. `hit_test.hpp/cpp` に衝突判定関数を追加 (全既存形状との組み合わせ)

```cpp
// 1. 形状定義
struct PolygonData { std::vector<Vec2f> vertices; };

// 2. variant に追加
using ColliderShape = std::variant<AABBData, CircleData, PolygonData>;

// 3. hit_test に判定関数を追加
bool hit_test(const PolygonData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb);
// ... 他の組み合わせも実装
```

### 9.7 新しいレンダーコマンドを追加する要件

1. コマンド構造体を定義
2. `RenderCommand` variant に追加
3. レンダラーバックエンドの `std::visit` ハンドラに処理を追加

```cpp
// 1. コマンド定義
struct RenderCmd_DrawParticles {
    Vec3f origin;
    uint32_t count;
    // ...
};

// 2. variant に追加
using RenderCommand = std::variant<..., RenderCmd_DrawParticles>;
```

### 9.8 新しいポストプロセスエフェクトを追加する要件

`PostProcessEffect` を継承し、`PostProcessStack` に追加。

```cpp
struct MyEffect : PostProcessEffect {
    float my_param = 1.0f;
    MyEffect() { name = "MyEffect"; }
    void apply() override { /* GPU 処理 */ }
};

// 使用
auto& effect = g_post_process.add<MyEffect>();
effect.my_param = 0.5f;
```

### 9.9 新しいネットワークバックエンドを作る要件

該当する concept を満たす型を実装する。

| やりたいこと | 満たす concept |
|------------|---------------|
| TCP クライアント | `SocketConnectable` + `StreamSendable` + `StreamReceivable` |
| UDP 通信 | `DatagramSendable` + `DatagramReceivable` |
| サーバー | `SocketListenable` |
| 高レベル管理 | `NetworkManageable` |
| HTTP | `HttpRequestable` |

### 9.10 新しい UI ウィジェットを作る要件

`UINode` を継承し、`update()` / `draw()` をオーバーライド。

```cpp
class MyWidget : public UINode {
public:
    explicit MyWidget(std::string name) : UINode(std::move(name)) {}
    void update(float dt) override;
    void draw(RenderContext& ctx, const WorldRect& parent_rect) override;
};
```

---

## 10. グローバルシステム一覧

| 変数名 | 型 | 定義場所 | 説明 |
|--------|---|---------|------|
| `g_physics` | `PhysicsSystem` | `engine/physics/physics_system.hpp` | 2D 衝突判定 |
| `g_rigid_body_world` | `RigidBodyWorld` | `engine/physics/rigid_body_world.hpp` | 3D リジッドボディ |
| `g_job_system` | `JobSystem` | `engine/core/job_system.hpp` | ワーカースレッドプール |
| `g_time` | `Time` | `engine/core/time.hpp` | フレーム時間管理 |
| `g_tweens` | `TweenManager` | `engine/core/tween.hpp` | トゥイーンアニメーション |
| `g_resources` | `ResourceManager` | `engine/resource/resource_manager.hpp` | リソースキャッシュ |
| `g_profiler` | `Profiler` | `engine/debug/profiler.hpp` | パフォーマンス計測 |
| `g_post_process` | `PostProcessStack` | `engine/render/post_process.hpp` | ポストプロセスエフェクト |

すべて `inline` グローバル変数として宣言。ヒープ確保なし、静的ストレージ期間。

---

## 11. ビルド構成

### CMake オプション

| オプション | デフォルト | 説明 |
|-----------|----------|------|
| `ERGO_BUILD_SAMPLES` | `OFF` | サンプルゲームをビルド |
| `ERGO_BUILD_DEMOS` | `OFF` | フィーチャーデモをビルド |
| `ERGO_BUILD_EDITOR` | `OFF` | MAUI エディタネイティブライブラリをビルド |
| `ERGO_BUILD_TESTS` | `ON` | テストアセンブリをビルド |
| `ERGO_ENABLE_NETWORK` | `ON` | ネットワークサポートを有効化 |
| `ERGO_FETCH_POCO` | `OFF` | POCO を GitHub からダウンロード |

### POCO 解決順序

1. `find_package(Poco)` — システム / vcpkg / conan
2. `FetchContent` — GitHub から取得 (`ERGO_FETCH_POCO=ON` の場合)
3. フォールバック — POSIX / BSD ソケットバックエンド

### ビルドアーティファクト

```
libErgoEngine.a   ← Engine Layer (静的ライブラリ)
libErgoSystem.a   ← System Layer (静的ライブラリ)
ergo_runtime      ← Runtime (実行ファイル)
libshooting_game.so/.dll  ← サンプルゲーム DLL
```

---

## 12. Web 版 (TypeScript)

`web/` ディレクトリに TypeScript/WebGPU による Web 実装を配置。

### C++ との対応

| C++ 機能 | TypeScript 等価 |
|---------|----------------|
| `concept` | TypeScript `interface` |
| `std::variant` + `std::visit` | `type` フィールド + `switch` 文 |
| `inline` グローバル | モジュールスコープのシングルトン |
| `JobSystem` / `parallel_for` | N/A (シングルスレッド) |
| Vulkan | WebGPU |
| DLL ロード | 動的 `import()` |

### ディレクトリ構成

```
web/src/
├── main.ts
├── engine/     # TaskSystem, StateMachine, World 等
├── math/       # Vec2, Vec3, Transform 等
├── physics/    # Collider, HitTest, PhysicsSystem
└── renderer/   # WebGPU バックエンド
```
