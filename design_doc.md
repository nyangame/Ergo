# 簡易ゲームエンジン設計書

## LightEngine — 軽量クロスプラットフォームゲームエンジン

---

## 1. 概要

### 1.1 目的

本エンジンは、PC (Windows/Linux)・Android・iOS・Web の4プラットフォームに対応する軽量ゲームエンジンである。ハイグラフィックは不要とし、2Dおよび軽量3D描画に特化した設計とする。

### 1.2 設計方針

| 項目 | 方針 |
|------|------|
| 言語規格 | C++20 (フラグシップ)、TypeScript/WebGPU (Web版) |
| 型制約 | 継承を使用せず `concept` で実装表現 |
| 描画基盤 | Vulkan (ネイティブ)、WebGPU (Web) |
| アプリ構造 | 1ゲーム = 1 DLL (共有ライブラリ) |
| コンポーネント仕様 | 共通インターフェース仕様で統一 (ILではなく仕様共通化) |

### 1.3 参考実装

CppSampleGame のクラス分割を参考とし、以下の3層構成を採用する。

```
┌─────────────────────────────────────────────┐
│  Application Layer (ゲーム DLL)               │
│  ゲームロジック / シーン / ゲームオブジェクト       │
├─────────────────────────────────────────────┤
│  Engine Layer (エンジンコア)                    │
│  タスク管理 / 物理 / コライダー / ステート         │
├─────────────────────────────────────────────┤
│  System Layer (プラットフォーム抽象化)            │
│  Vulkan/WebGPU / Input / Audio / FileIO       │
└─────────────────────────────────────────────┘
```

---

## 2. アーキテクチャ全体像

### 2.1 レイヤー構成図

```
Application Layer (Game DLL)
  │
  │  ← C API (DLL境界) / concept準拠の型
  │
Engine Layer (LightEngine.lib / .so)
  ├── Core        : タスクシステム、ステートマシン、メモリ管理
  ├── Physics     : 2Dコライダー、衝突判定
  ├── Scene       : シーン管理、オブジェクト管理
  ├── Math        : ベクトル、行列、変換
  └── Component   : コンポーネントシステム (concept ベース)
  │
  │  ← concept準拠の抽象インターフェース
  │
System Layer (プラットフォーム別実装)
  ├── Renderer    : Vulkan / WebGPU バックエンド
  ├── Input       : キーボード / タッチ / ゲームパッド
  ├── Audio       : プラットフォーム別音声
  ├── FileIO      : ファイルシステム抽象化
  └── Window      : ウィンドウ / サーフェス管理
```

### 2.2 ビルドターゲット

| ターゲット | 描画 | コンパイラ | 配布形式 |
|-----------|------|----------|---------|
| Windows | Vulkan | MSVC / Clang | .exe + .dll |
| Linux | Vulkan | GCC / Clang | ELF + .so |
| Android | Vulkan | NDK (Clang) | .apk (.so) |
| iOS | MoltenVK | Xcode (Clang) | .ipa (.dylib) |
| Web | WebGPU | Emscripten / TS | .wasm / .js |

---

## 3. 共通コンポーネント仕様 (Common Component Specification)

### 3.1 概念

ILや共通ランタイムを用いず、C++20 の concept によってコンポーネントの「振る舞い仕様」を定義する。各プラットフォームの実装はこの concept を満たすことで互換性を保証する。

### 3.2 基本 concept 定義

```cpp
#include <concepts>
#include <cstdint>

// --- ライフサイクル ---
template<typename T>
concept Startable = requires(T t) {
    { t.start() } -> std::same_as<void>;
};

template<typename T>
concept Updatable = requires(T t, float dt) {
    { t.update(dt) } -> std::same_as<void>;
};

template<typename T>
concept Drawable = requires(T t, auto& ctx) {
    { t.draw(ctx) } -> std::same_as<void>;
};

template<typename T>
concept Releasable = requires(T t) {
    { t.release() } -> std::same_as<void>;
};

// --- タスク (MonoBehaviour相当) ---
template<typename T>
concept TaskLike = Startable<T> && Updatable<T> && Releasable<T>;

// --- ゲームオブジェクト ---
template<typename T>
concept GameObjectLike = requires(T t) {
    { t.transform() } -> std::same_as<Transform2D&>;
    { t.name() } -> std::convertible_to<std::string_view>;
    { t.object_type() } -> std::same_as<uint32_t>;
};

// --- コライダー ---
template<typename T>
concept ColliderLike = requires(T a, T b) {
    { a.is_hit(b) } -> std::same_as<bool>;
    { a.tag() } -> std::same_as<uint32_t>;
    { a.owner_transform() } -> std::same_as<const Transform2D&>;
};

// --- レンダラーバックエンド ---
template<typename T>
concept RendererBackend = requires(T t) {
    { t.initialize() } -> std::same_as<bool>;
    { t.begin_frame() } -> std::same_as<void>;
    { t.end_frame() } -> std::same_as<void>;
    { t.shutdown() } -> std::same_as<void>;
};

// --- 入力システム ---
template<typename T>
concept InputBackend = requires(T t, uint32_t key) {
    { t.is_key_down(key) } -> std::same_as<bool>;
    { t.is_key_pressed(key) } -> std::same_as<bool>;
    { t.mouse_position() } -> std::same_as<Vec2f>;
    { t.poll_events() } -> std::same_as<void>;
};
```

### 3.3 CppSampleGame との対応関係

| CppSampleGame | LightEngine | 実現手段 |
|--------------|-------------|---------|
| `TaskBase` (継承) | `TaskLike` concept | concept 制約のテンプレート |
| `IGameObject` (継承) | `GameObjectLike` concept | concept + コンポーネント集約 |
| `Collider2D` (継承 + enum) | `ColliderLike` concept | concept + std::variant |
| `Singleton<T>` | モジュールレベル関数 + `inline` 変数 | C++20 modules / inline |
| `StateControl` (仮想関数) | `StateMachine<States...>` | std::variant + visit |

---

## 4. Engine Layer 詳細設計

### 4.1 数学ライブラリ (Math)

CppSampleGame の `GameMath.h` を拡張し、union トリックを廃止して型安全な設計とする。

```cpp
struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;

    Vec2f operator+(Vec2f o) const { return {x + o.x, y + o.y}; }
    Vec2f operator*(float s) const { return {x * s, y * s}; }
    Vec2f& operator+=(Vec2f o) { x += o.x; y += o.y; return *this; }
    float length_sq() const { return x * x + y * y; }
    float length() const { return std::sqrt(length_sq()); }
    Vec2f normalized() const { float l = length(); return {x/l, y/l}; }
};

struct Size2f {
    float w = 0.0f;
    float h = 0.0f;
    float radius() const { return w * 0.5f; } // 円用
};

struct Transform2D {
    Vec2f position;
    float rotation = 0.0f;  // ラジアン
    Size2f size;
};
```

### 4.2 タスクシステム (Core/TaskSystem)

CppSampleGame の `TaskManager` を concept ベースで再構築する。

```cpp
// タスクはtype-erasedで保持
struct TaskHandle {
    uint64_t id;
    uint32_t layer;
};

// タスクラッパー (type-erased)
class TaskWrapper {
    struct Concept {
        virtual ~Concept() = default;
        virtual void start() = 0;
        virtual void update(float dt) = 0;
        virtual void draw(RenderContext& ctx) = 0;
        virtual void release() = 0;
    };

    template<TaskLike T>
    struct Model : Concept {
        T task;
        template<typename... Args>
        Model(Args&&... args) : task(std::forward<Args>(args)...) {}
        void start() override { task.start(); }
        void update(float dt) override { task.update(dt); }
        void draw(RenderContext& ctx) override {
            if constexpr (Drawable<T, RenderContext>) task.draw(ctx);
        }
        void release() override { task.release(); }
    };

    std::unique_ptr<Concept> impl_;
public:
    template<TaskLike T, typename... Args>
    static TaskWrapper create(Args&&... args) {
        TaskWrapper w;
        w.impl_ = std::make_unique<Model<T>>(std::forward<Args>(args)...);
        return w;
    }
};
```

タスクマネージャ:

```cpp
enum class TaskLayer : uint32_t {
    Default = 0,
    Bullet,
    UI,
    Max
};

enum class RunPhase {
    Start, Update, Physics, Draw, Destroy
};

class TaskManager {
    std::array<std::vector<TaskEntry>, (size_t)TaskLayer::Max> layers_;
    std::vector<TaskHandle> destroy_queue_;

public:
    template<TaskLike T, typename... Args>
    TaskHandle register_task(TaskLayer layer, Args&&... args);
    void destroy(TaskHandle handle);
    void run(RunPhase phase, float dt);
};
```

### 4.3 物理システム (Physics)

CppSampleGame の `SysPhysics` を参考に、concept ベースの衝突判定を構築する。

```cpp
enum class ColliderTag : uint32_t {
    Invalid = 0,
    Player,
    Enemy,
    Bullet,
    Max
};

// コライダーをvariantで表現 (継承不使用)
struct AABBData {
    Vec2f half_extent;
};

struct CircleData {
    float radius;
};

using ColliderShape = std::variant<AABBData, CircleData>;

struct Collider {
    ColliderShape shape;
    ColliderTag tag = ColliderTag::Invalid;
    uint64_t owner_id = 0;            // 所有GameObjectのID
    const Transform2D* transform = nullptr; // 所有者のTransformへの参照
    std::function<bool(const Collider&)> on_hit;
};
```

衝突判定関数群 (フリー関数):

```cpp
// 判定マトリクス (visitor パターン)
bool check_hit(const Collider& a, const Collider& b) {
    return std::visit([&](const auto& sa, const auto& sb) {
        return hit_test(sa, *a.transform, sb, *b.transform);
    }, a.shape, b.shape);
}

// 各組み合わせのオーバーロード
bool hit_test(const AABBData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb);
bool hit_test(const CircleData& a, const Transform2D& ta,
              const CircleData& b, const Transform2D& tb);
bool hit_test(const CircleData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb);
bool hit_test(const AABBData& a, const Transform2D& ta,
              const CircleData& b, const Transform2D& tb);
```

物理システム:

```cpp
class PhysicsSystem {
    std::array<std::vector<Collider*>, (size_t)ColliderTag::Max> colliders_;
    std::vector<Collider*> calc_stack_;  // 移動したオブジェクト
    // 判定マトリクス: どのタグ同士を判定するか
    std::array<std::array<bool, (size_t)ColliderTag::Max>, (size_t)ColliderTag::Max> matrix_;

public:
    void register_collider(Collider& c);
    void remove_collider(Collider& c);
    void mark_moved(Collider& c);      // CalcStack相当
    void run();                          // 判定実行
};
```

### 4.4 ステートマシン (Core/StateMachine)

CppSampleGame の `State / StateControl` を `std::variant` + `std::visit` で置換する。

```cpp
// 各ステートはただの構造体
struct TitleState {
    void enter() { /* ... */ }
    void update(float dt) { /* ... */ }
    void exit() { /* ... */ }
};

struct InGameState {
    void enter() { /* ... */ }
    void update(float dt) { /* ... */ }
    void exit() { /* ... */ }
};

// ステートマシン本体
template<typename... States>
class StateMachine {
    using StateVariant = std::variant<std::monostate, States...>;
    StateVariant current_;

public:
    template<typename S, typename... Args>
    void transition(Args&&... args) {
        // 現在のステートのexit呼び出し
        std::visit([](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>)
                s.exit();
        }, current_);
        // 新ステートへ遷移
        current_.template emplace<S>(std::forward<Args>(args)...);
        std::get<S>(current_).enter();
    }

    void update(float dt) {
        std::visit([dt](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>)
                s.update(dt);
        }, current_);
    }
};

// 使用例
using GameSequence = StateMachine<TitleState, InGameState>;
```

### 4.5 ゲームオブジェクトとコンポーネント

CppSampleGame の `IGameObject` を継承からコンポーネント集約に変更する。

```cpp
using ComponentStorage = std::unordered_map<std::type_index, std::any>;

struct GameObject {
    uint64_t id;
    std::string name;
    uint32_t object_type = 0;
    Transform2D transform;
    ComponentStorage components;

    // concept制約付きのコンポーネント追加
    template<typename T>
    void add_component(T&& comp) {
        components[std::type_index(typeid(T))] = std::forward<T>(comp);
    }

    template<typename T>
    T* get_component() {
        auto it = components.find(std::type_index(typeid(T)));
        if (it == components.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }
};
```

---

## 5. System Layer 詳細設計

### 5.1 プラットフォーム抽象化方針

System Layer は concept で定義されたバックエンドインターフェースを各プラットフォームが実装する。コンパイル時にプラットフォームが確定するため、仮想関数は不要。

```cpp
// コンパイル時のプラットフォーム選択
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    using PlatformRenderer = VulkanRenderer;
    using PlatformInput = DesktopInput;
    using PlatformWindow = DesktopWindow;
#elif defined(PLATFORM_ANDROID)
    using PlatformRenderer = VulkanRenderer;  // 共通Vulkan実装
    using PlatformInput = TouchInput;
    using PlatformWindow = AndroidWindow;
#elif defined(PLATFORM_IOS)
    using PlatformRenderer = VulkanRenderer;  // MoltenVK経由
    using PlatformInput = TouchInput;
    using PlatformWindow = IOSWindow;
#elif defined(PLATFORM_WEB)
    using PlatformRenderer = WebGPURenderer;
    using PlatformInput = WebInput;
    using PlatformWindow = CanvasWindow;
#endif

// concept 検証
static_assert(RendererBackend<PlatformRenderer>);
static_assert(InputBackend<PlatformInput>);
```

### 5.2 Vulkan レンダラー (軽量実装)

ハイグラフィック不要のため、最小限のVulkan構成とする。

```
VulkanRenderer
├── VkInstance / VkDevice / VkQueue
├── Swapchain管理
├── シンプルレンダーパス (1パス)
├── 2Dスプライトパイプライン
│   ├── 頂点バッファ (動的バッチ)
│   └── テクスチャアトラス
├── 基本シェイプパイプライン (矩形/円)
└── テキスト描画 (ビットマップフォント)
```

主要構造体:

```cpp
struct VulkanRenderer {
    // 初期化/終了
    bool initialize();
    void shutdown();

    // フレーム
    void begin_frame();
    void end_frame();

    // 2D描画プリミティブ
    void draw_rect(Vec2f pos, Size2f size, Color color, bool filled);
    void draw_circle(Vec2f center, float radius, Color color, bool filled);
    void draw_sprite(Vec2f pos, Size2f size, TextureHandle tex, Rect uv);
    void draw_text(Vec2f pos, std::string_view text, Color color, float scale);
};
```

### 5.3 WebGPU レンダラー (Web版)

Web版は TypeScript + WebGPU で実装し、Vulkan版と同等のAPIサーフェスを提供する。

```typescript
interface RendererBackend {
    initialize(): Promise<boolean>;
    beginFrame(): void;
    endFrame(): void;
    shutdown(): void;
    drawRect(pos: Vec2, size: Size2, color: Color, filled: boolean): void;
    drawCircle(center: Vec2, radius: number, color: Color, filled: boolean): void;
    drawSprite(pos: Vec2, size: Size2, tex: TextureHandle, uv: Rect): void;
    drawText(pos: Vec2, text: string, color: Color, scale: number): void;
}
```

### 5.4 入力システム

```cpp
struct DesktopInput {
    void poll_events();
    bool is_key_down(uint32_t key) const;
    bool is_key_pressed(uint32_t key) const;
    Vec2f mouse_position() const;
    bool is_mouse_button_down(uint32_t button) const;
};

struct TouchInput {
    void poll_events();
    bool is_key_down(uint32_t key) const;   // 仮想キー
    bool is_key_pressed(uint32_t key) const;
    Vec2f mouse_position() const;            // タッチ座標
    // タッチ固有
    std::span<const TouchPoint> touches() const;
};
```

### 5.5 ウィンドウ / サーフェス

```cpp
template<typename T>
concept WindowBackend = requires(T t) {
    { t.create(uint32_t{}, uint32_t{}, std::string_view{}) } -> std::same_as<bool>;
    { t.should_close() } -> std::same_as<bool>;
    { t.poll_events() } -> std::same_as<void>;
    { t.get_surface_handle() }; // プラットフォーム依存型
    { t.width() } -> std::same_as<uint32_t>;
    { t.height() } -> std::same_as<uint32_t>;
};
```

---

## 6. Application Layer (ゲーム DLL) 設計

### 6.1 DLL インターフェース

1ゲーム = 1 DLL として、エンジンとの境界は C API で定義する。

```cpp
// game_interface.h — エンジンとゲームDLLの契約
extern "C" {
    // ゲームDLLが公開する関数
    struct GameCallbacks {
        void (*on_init)(EngineContext* ctx);
        void (*on_update)(float dt);
        void (*on_draw)(RenderContext* ctx);
        void (*on_shutdown)();
    };

    // DLLエントリーポイント
    __declspec(dllexport) GameCallbacks* get_game_callbacks();
}
```

### 6.2 EngineContext

エンジンがゲームDLLに提供するAPI群:

```cpp
struct EngineContext {
    // タスク管理
    TaskHandle (*register_task)(void* task_data, TaskVTable vtable, TaskLayer layer);
    void (*destroy_task)(TaskHandle handle);

    // 物理
    ColliderHandle (*register_collider)(ColliderDesc desc);
    void (*remove_collider)(ColliderHandle handle);

    // 描画
    void (*draw_rect)(Vec2f pos, Size2f size, Color color, bool filled);
    void (*draw_circle)(Vec2f center, float radius, Color color, bool filled);
    void (*draw_sprite)(Vec2f pos, Size2f size, TextureHandle tex, Rect uv);
    void (*draw_text)(Vec2f pos, const char* text, Color color, float scale);

    // 入力
    bool (*is_key_down)(uint32_t key);
    bool (*is_key_pressed)(uint32_t key);
    Vec2f (*mouse_position)();

    // リソース
    TextureHandle (*load_texture)(const char* path);
    void (*unload_texture)(TextureHandle handle);
};
```

### 6.3 CppSampleGame からの移植例

CppSampleGame の Player を LightEngine で実装した場合:

```cpp
// Player.h — ゲームDLL側
struct PlayerComponent {
    int hp = 100;
    int interval = 0;
    float speed = 5.0f;
    float ground_y = 0.0f;
    float jump_pow = 25.0f;
    float jump_y = 0.0f;
    float grav = 0.0f;
};

struct Player {
    GameObject object;
    PlayerComponent player;
    Collider collider;

    void start() {
        object.object_type = (uint32_t)GameObjectType::Player;
        player.hp = 100;
        player.ground_y = object.transform.position.y;
        object.transform.size = {50.0f, 80.0f};
        collider.shape = AABBData{{25.0f, 40.0f}};
        collider.tag = ColliderTag::Player;
        collider.transform = &object.transform;
    }

    void update(float dt) {
        auto* ctx = get_engine_context();
        if (ctx->is_key_down(KEY_LEFT))  object.transform.position.x -= 1.0f;
        if (ctx->is_key_down(KEY_RIGHT)) object.transform.position.x += 1.0f;
        // ジャンプ・射撃ロジック...
    }

    void draw(RenderContext& ctx) {
        auto& t = object.transform;
        ctx.draw_rect(t.position, t.size, {64, 64, 255, 255}, true);
    }

    void release() {}
};

static_assert(TaskLike<Player>);
static_assert(GameObjectLike<Player>); // transform(), name() 等を実装
```

---

## 7. ファイル構成

### 7.1 ディレクトリツリー

```
LightEngine/
├── CMakeLists.txt
├── engine/                          # Engine Layer
│   ├── core/
│   │   ├── concepts.hpp             # 全concept定義
│   │   ├── task_system.hpp/.cpp     # TaskManager
│   │   ├── state_machine.hpp        # StateMachine<States...>
│   │   ├── game_object.hpp          # GameObject + Component
│   │   └── id_generator.hpp         # ID生成
│   ├── math/
│   │   ├── vec2.hpp                 # Vec2f
│   │   ├── size2.hpp                # Size2f
│   │   ├── transform.hpp            # Transform2D
│   │   └── color.hpp                # Color
│   ├── physics/
│   │   ├── collider.hpp             # Collider, ColliderShape
│   │   ├── hit_test.hpp/.cpp        # 衝突判定関数群
│   │   └── physics_system.hpp/.cpp  # PhysicsSystem
│   └── resource/
│       ├── texture.hpp              # TextureHandle
│       └── resource_manager.hpp     # リソース管理
│
├── system/                          # System Layer
│   ├── renderer/
│   │   ├── vulkan/
│   │   │   ├── vk_renderer.hpp/.cpp
│   │   │   ├── vk_pipeline.hpp/.cpp
│   │   │   ├── vk_swapchain.hpp/.cpp
│   │   │   └── vk_sprite_batch.hpp/.cpp
│   │   └── webgpu/                  # Web版 (TypeScript)
│   │       └── webgpu_renderer.ts
│   ├── input/
│   │   ├── desktop_input.hpp/.cpp
│   │   ├── touch_input.hpp/.cpp
│   │   └── web_input.ts
│   ├── window/
│   │   ├── desktop_window.hpp/.cpp  # GLFW or 直接Win32/X11
│   │   ├── android_window.hpp/.cpp
│   │   ├── ios_window.hpp/.mm
│   │   └── canvas_window.ts
│   ├── audio/
│   │   ├── audio_backend.hpp
│   │   └── ... (プラットフォーム別)
│   └── platform.hpp                 # using宣言 (プラットフォーム選択)
│
├── runtime/                         # エンジンランタイム (実行ファイル)
│   ├── main.cpp                     # エントリーポイント
│   ├── engine_context.hpp/.cpp      # EngineContext構築
│   └── dll_loader.hpp/.cpp          # ゲームDLLロード
│
├── game_interface/                  # DLL境界定義 (エンジン・ゲーム共有)
│   ├── game_interface.h             # C API定義
│   └── engine_types.h               # 共有型定義
│
├── web/                             # Web版エントリー
│   ├── src/
│   │   ├── engine.ts                # Engine Layer (TS版)
│   │   ├── physics.ts
│   │   ├── task_system.ts
│   │   └── main.ts
│   ├── package.json
│   └── tsconfig.json
│
└── samples/                         # サンプルゲーム (DLL)
    └── shooting/
        ├── CMakeLists.txt
        ├── game_main.cpp            # DLLエントリー
        ├── player.hpp/.cpp
        ├── enemy.hpp/.cpp
        ├── bullet.hpp/.cpp
        └── scenes/
            ├── title_scene.hpp/.cpp
            └── ingame_scene.hpp/.cpp
```

### 7.2 CppSampleGame との対応

| CppSampleGame ファイル | LightEngine 対応先 |
|-----------------------|-------------------|
| `Foundation/GameMath.h` | `engine/math/vec2.hpp`, `size2.hpp` |
| `Foundation/Transform.h` | `engine/math/transform.hpp` |
| `Foundation/TaskBase.h/.cpp` | `engine/core/task_system.hpp` |
| `Foundation/TaskManager.h/.cpp` | `engine/core/task_system.hpp/.cpp` |
| `Foundation/Singleton.h` | 廃止 → inline 変数 or モジュール |
| `Foundation/State.h/.cpp` | `engine/core/state_machine.hpp` |
| `Foundation/Collider2D.h/.cpp` | `engine/physics/collider.hpp`, `hit_test.hpp` |
| `Foundation/SysPhysics.h/.cpp` | `engine/physics/physics_system.hpp/.cpp` |
| `Game/IGameObject.h` | `engine/core/game_object.hpp` |
| `Game/Player.h/.cpp` | `samples/shooting/player.hpp/.cpp` |
| `Game/Enemy.h/.cpp` | `samples/shooting/enemy.hpp/.cpp` |
| `Game/Bullet.h/.cpp` | `samples/shooting/bullet.hpp/.cpp` |
| `Game/MainGameSequence.h/.cpp` | `samples/shooting/game_main.cpp` |
| `Game/TitleState.h/.cpp` | `samples/shooting/scenes/title_scene.hpp` |
| `Game/MainGameState.h/.cpp` | `samples/shooting/scenes/ingame_scene.hpp` |
| `main.cpp` | `runtime/main.cpp` |
| `stdafx.h` | 廃止 → CMake PCH 設定 |
| DxLib 依存 | `system/renderer/vulkan/` に置換 |

---

## 8. ゲームループ

CppSampleGame の `main.cpp` に対応するエンジンランタイムのメインループ:

```cpp
int main() {
    // 1. プラットフォーム初期化
    PlatformWindow window;
    window.create(800, 600, "LightEngine");

    PlatformRenderer renderer;
    renderer.initialize(window.get_surface_handle());

    PlatformInput input;

    // 2. エンジンシステム初期化
    TaskManager task_mgr;
    PhysicsSystem physics;

    // 3. EngineContext 構築
    EngineContext ctx = build_engine_context(task_mgr, physics, renderer, input);

    // 4. ゲームDLLロード
    auto game = load_game_dll("game.dll");
    game->on_init(&ctx);

    // 5. メインループ
    float last_time = get_time();
    while (!window.should_close()) {
        float now = get_time();
        float dt = now - last_time;
        last_time = now;

        // イベント処理
        window.poll_events();
        input.poll_events();

        // 破棄処理 (DESTROY相当)
        task_mgr.run(RunPhase::Destroy, dt);

        // 物理演算 (PHYSICS相当)
        task_mgr.run(RunPhase::Physics, dt);

        // メイン更新 (DO相当)
        task_mgr.run(RunPhase::Update, dt);

        // ゲームDLL更新
        game->on_update(dt);

        // 衝突判定
        physics.run();

        // 描画
        renderer.begin_frame();
        task_mgr.run(RunPhase::Draw, dt);
        game->on_draw(&render_ctx);
        renderer.end_frame();
    }

    // 6. 終了処理
    game->on_shutdown();
    renderer.shutdown();
    return 0;
}
```

---

## 9. Web版の対応方針

### 9.1 設計原則

Web版は C++ コードの直接移植ではなく、共通コンポーネント仕様に基づく TypeScript 再実装とする。

| C++ 側 | Web (TypeScript) 側 |
|--------|---------------------|
| `concept TaskLike` | `interface TaskLike` |
| `std::variant` | Union Type / discriminated union |
| `std::visit` | switch + type guard |
| DLL ロード | ES Module の動的 `import()` |
| Vulkan | WebGPU |
| C API 境界 | TypeScript interface |

### 9.2 Web版ゲームモジュール

```typescript
// game_interface.ts — エンジンとゲームモジュールの契約
export interface GameModule {
    onInit(ctx: EngineContext): void;
    onUpdate(dt: number): void;
    onDraw(ctx: RenderContext): void;
    onShutdown(): void;
}

// ゲーム側 (ES Module)
export default {
    onInit(ctx) { /* ... */ },
    onUpdate(dt) { /* ... */ },
    onDraw(ctx) { /* ... */ },
    onShutdown() { /* ... */ },
} satisfies GameModule;
```

---

## 10. ビルドシステム

### 10.1 CMake 構成

```cmake
cmake_minimum_required(VERSION 3.20)
project(LightEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# --- Engine (静的ライブラリ) ---
add_library(light_engine STATIC
    engine/core/task_system.cpp
    engine/physics/hit_test.cpp
    engine/physics/physics_system.cpp
    # ...
)
target_include_directories(light_engine PUBLIC engine/)

# --- System Layer ---
if(PLATFORM_WEB)
    # Web版はTypeScriptで別ビルド
else()
    find_package(Vulkan REQUIRED)
    add_library(light_system STATIC
        system/renderer/vulkan/vk_renderer.cpp
        system/input/desktop_input.cpp
        system/window/desktop_window.cpp
    )
    target_link_libraries(light_system PUBLIC Vulkan::Vulkan light_engine)
endif()

# --- Runtime (実行ファイル) ---
add_executable(light_runtime runtime/main.cpp runtime/dll_loader.cpp)
target_link_libraries(light_runtime PRIVATE light_system)

# --- Sample Game (共有ライブラリ = DLL) ---
add_library(sample_shooting SHARED
    samples/shooting/game_main.cpp
    samples/shooting/player.cpp
    samples/shooting/enemy.cpp
    samples/shooting/bullet.cpp
)
target_link_libraries(sample_shooting PRIVATE light_engine)
target_include_directories(sample_shooting PRIVATE game_interface/)
```

### 10.2 プラットフォーム別ビルド

| プラットフォーム | ビルドコマンド |
|----------------|-------------|
| Windows | `cmake -B build -G "Visual Studio 17 2022" && cmake --build build` |
| Linux | `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` |
| Android | `cmake -B build -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a` |
| iOS | `cmake -B build -G Xcode -DCMAKE_SYSTEM_NAME=iOS` |
| Web | `cd web && npm install && npm run build` |

---

## 11. 改善点まとめ (CppSampleGame からの変更)

| 項目 | CppSampleGame | LightEngine |
|------|--------------|-------------|
| 型制約 | 仮想関数 + 継承 | concept + variant |
| シングルトン | テンプレート継承 | inline変数 or モジュール |
| コライダー | enum switch | std::variant + visit |
| メモリ管理 | shared_ptr 多用 | ID + プール管理 |
| 描画 | DxLib 直接呼び出し | Vulkan/WebGPU バックエンド |
| ゲーム構造 | 直接リンク | DLL 分離 |
| Vector2 union | `X/W/Radius` 共用体 | 型安全な別構造体 |
| プリコンパイルヘッダ | stdafx.h | CMake PCH |
| プラットフォーム | Windows のみ | PC/Android/iOS/Web |

---

## 12. 今後の拡張ポイント

- **ECS (Entity Component System)**: 現在のコンポーネント集約をフルECSに発展可能
- **シリアライゼーション**: コンポーネントの保存/復元でシーンエディタ対応
- **ホットリロード**: DLL の動的リロードによる開発中のゲームロジック更新
- **マルチスレッド**: PhysicsSystem の並列化 (CppSampleGame で保留されていた部分)
- **アセットパイプライン**: テクスチャ/サウンドのビルド時変換
- **imgui統合**: デバッグUI
