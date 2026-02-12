# Ergo — 軽量クロスプラットフォームゲームエンジン 設計書

> **リポジトリ**: https://github.com/nyangame/Ergo
> **ライセンス**: MIT (Copyright (c) 2026 k.mitarai)
> **ブランチ戦略**: `main` をデフォルトとし、機能単位で `feature/*` ブランチを切る

---

## 0. 実装ガイド (Claude Code 向け)

### 0.1 作業手順

1. このファイル (`DESIGN.md`) をリポジトリルートにコミットする
2. 第7章のディレクトリツリーに従いフォルダ構成を作成する
3. 第4章 Engine Layer → 第5章 System Layer → 第6章 Application Layer の順で実装する
4. 各レイヤーは独立してコンパイルできる単位で実装する
5. CMakeLists.txt を第10章に従い作成する

### 0.2 コーディング規約

| 項目 | 規約 |
|------|------|
| 言語規格 | C++20 (`-std=c++20`) |
| 命名 (型) | `PascalCase` — `TaskManager`, `Vec2f` |
| 命名 (関数/変数) | `snake_case` — `is_key_down`, `calc_stack` |
| 命名 (メンバ変数) | `snake_case_` (末尾アンダースコア) — `position_`, `hp_` |
| 命名 (定数/enum値) | `PascalCase` — `ColliderTag::Player` |
| ファイル名 | `snake_case` — `task_system.hpp`, `hit_test.cpp` |
| ヘッダ | `.hpp` (C++), `.h` (C API / DLL境界) |
| ヘッダガード | `#pragma once` |
| 継承 | **使用禁止** — concept + std::variant で代替 |
| スマートポインタ | 最小限。基本は値/参照/ID管理。DLL境界は生ポインタ |
| インクルード | 必要なヘッダのみ明示的に。プリコンパイルヘッダは CMake PCH で設定 |

### 0.3 参考コード (CppSampleGame)

本設計書の添付ドキュメント群に CppSampleGame のソースコードが含まれている。
以下の対応関係に従い、継承ベースの設計を concept ベースに変換して実装すること。

| CppSampleGame | Ergo | 変換方針 |
|--------------|------|---------|
| `TaskBase` (virtual) | `TaskLike` concept | type-erasure でランタイム多態 |
| `IGameObject` (virtual) | `GameObjectLike` concept | コンポーネント集約 |
| `Collider2D` (virtual + enum) | `ColliderLike` concept + variant | std::variant + visit |
| `Singleton<T>` | inline 変数 | `inline PhysicsSystem g_physics;` |
| `State/StateControl` (virtual) | `StateMachine<States...>` | std::variant + visit |
| DxLib 描画関数 | `RendererBackend` concept | Vulkan/WebGPU 実装 |

### 0.4 ブランチとコミット方針

| フェーズ | ブランチ名 | 内容 |
|---------|-----------|------|
| Phase 1 | `feature/design-doc` | DESIGN.md + README.md のコミット |
| Phase 2 | `feature/engine-core` | engine/ ディレクトリ (math, core, physics) |
| Phase 3 | `feature/system-layer` | system/ ディレクトリ (renderer, input, window) |
| Phase 4 | `feature/runtime` | runtime/ + game_interface/ |
| Phase 5 | `feature/sample-game` | samples/shooting/ |
| Phase 6 | `feature/web` | web/ ディレクトリ (TypeScript実装) |

各 phase を PR にして `main` にマージする。コミットメッセージは以下の形式:

```
[layer] 概要

例:
[engine] Add math library (Vec2f, Size2f, Transform2D)
[system] Add Vulkan renderer skeleton
[runtime] Add DLL loader and engine context
[sample] Add shooting game player implementation
[docs] Add DESIGN.md
```

---

## 1. 概要

### 1.1 目的

軽量なクロスプラットフォームゲームエンジン。2Dおよび軽量3D描画に特化する。

### 1.2 設計方針

| 項目 | 方針 |
|------|------|
| 言語規格 | C++20 (フラグシップ)、TypeScript/WebGPU (Web版) |
| 型制約 | 継承を使用せず `concept` で実装表現 |
| 描画基盤 | Vulkan (ネイティブ)、WebGPU (Web) |
| アプリ構造 | 1ゲーム = 1 DLL (共有ライブラリ) |
| コンポーネント仕様 | 共通インターフェース仕様で統一 (ILではなく仕様共通化) |

### 1.3 レイヤー構成

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

### 1.4 ビルドターゲット

| ターゲット | 描画 | コンパイラ | 配布形式 |
|-----------|------|----------|---------|
| Windows | Vulkan | MSVC / Clang | .exe + .dll |
| Linux | Vulkan | GCC / Clang | ELF + .so |
| Android | Vulkan | NDK (Clang) | .apk (.so) |
| iOS | MoltenVK | Xcode (Clang) | .ipa (.dylib) |
| Web | WebGPU | Emscripten / TS | .wasm / .js |

---

## 2. アーキテクチャ詳細

### 2.1 レイヤー依存関係

```
Application Layer (Game DLL)
  │
  │  ← C API (DLL境界) / concept準拠の型
  │
Engine Layer (libErgoEngine.a / .lib)
  ├── core/        : タスクシステム、ステートマシン、ゲームオブジェクト
  ├── math/        : ベクトル、サイズ、変換
  ├── physics/     : コライダー、衝突判定
  └── resource/    : テクスチャハンドル、リソース管理
  │
  │  ← concept準拠の抽象インターフェース
  │
System Layer (libErgoSystem.a / .lib)
  ├── renderer/    : Vulkan / WebGPU バックエンド
  ├── input/       : キーボード / タッチ / ゲームパッド
  ├── window/      : ウィンドウ / サーフェス管理
  └── audio/       : プラットフォーム別音声
```

---

## 3. 共通コンポーネント仕様 (concept 定義)

### 3.1 ファイル: `engine/core/concepts.hpp`

```cpp
#pragma once

#include <concepts>
#include <cstdint>
#include <string_view>

// 前方宣言
struct Transform2D;
struct Vec2f;
struct RenderContext;

// --- ライフサイクル ---
template<typename T>
concept Startable = requires(T t) {
    { t.start() } -> std::same_as<void>;
};

template<typename T>
concept Updatable = requires(T t, float dt) {
    { t.update(dt) } -> std::same_as<void>;
};

template<typename T, typename Ctx = RenderContext>
concept Drawable = requires(T t, Ctx& ctx) {
    { t.draw(ctx) } -> std::same_as<void>;
};

template<typename T>
concept Releasable = requires(T t) {
    { t.release() } -> std::same_as<void>;
};

// --- タスク (CppSampleGame の TaskBase 相当) ---
template<typename T>
concept TaskLike = Startable<T> && Updatable<T> && Releasable<T>;

// --- ゲームオブジェクト (CppSampleGame の IGameObject 相当) ---
template<typename T>
concept GameObjectLike = requires(T t) {
    { t.transform() } -> std::same_as<Transform2D&>;
    { t.name() } -> std::convertible_to<std::string_view>;
    { t.object_type() } -> std::same_as<uint32_t>;
};

// --- コライダー (CppSampleGame の Collider2D 相当) ---
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

// --- ウィンドウ ---
template<typename T>
concept WindowBackend = requires(T t, uint32_t w, uint32_t h, std::string_view title) {
    { t.create(w, h, title) } -> std::same_as<bool>;
    { t.should_close() } -> std::same_as<bool>;
    { t.poll_events() } -> std::same_as<void>;
    { t.width() } -> std::same_as<uint32_t>;
    { t.height() } -> std::same_as<uint32_t>;
};
```

---

## 4. Engine Layer 詳細

### 4.1 数学ライブラリ

#### `engine/math/vec2.hpp`

```cpp
#pragma once
#include <cmath>

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2f() = default;
    constexpr Vec2f(float x, float y) : x(x), y(y) {}

    constexpr Vec2f operator+(Vec2f o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2f operator-(Vec2f o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2f operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2f& operator+=(Vec2f o) { x += o.x; y += o.y; return *this; }
    constexpr Vec2f& operator-=(Vec2f o) { x -= o.x; y -= o.y; return *this; }
    constexpr Vec2f& operator*=(float s) { x *= s; y *= s; return *this; }

    constexpr float length_sq() const { return x * x + y * y; }
    float length() const { return std::sqrt(length_sq()); }
    Vec2f normalized() const {
        float l = length();
        return (l > 0.0f) ? Vec2f{x / l, y / l} : Vec2f{};
    }

    static constexpr Vec2f zero() { return {0.0f, 0.0f}; }
};
```

#### `engine/math/size2.hpp`

CppSampleGame では `Vector2` の union で `X/W/Radius` を兼用していたが、型安全のため分離する。

```cpp
#pragma once

struct Size2f {
    float w = 0.0f;
    float h = 0.0f;

    constexpr Size2f() = default;
    constexpr Size2f(float w, float h) : w(w), h(h) {}

    constexpr float half_w() const { return w * 0.5f; }
    constexpr float half_h() const { return h * 0.5f; }
    constexpr float radius() const { return w * 0.5f; }
};
```

#### `engine/math/transform.hpp`

```cpp
#pragma once
#include "vec2.hpp"
#include "size2.hpp"

struct Transform2D {
    Vec2f position;
    float rotation = 0.0f;  // ラジアン
    Size2f size;
};
```

#### `engine/math/color.hpp`

```cpp
#pragma once
#include <cstdint>

struct Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
};
```

### 4.2 タスクシステム

#### `engine/core/task_system.hpp`

CppSampleGame の `TaskBase` + `TaskManager` を concept + type-erasure で再構築。

```cpp
#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include "concepts.hpp"

struct RenderContext; // 前方宣言

struct TaskHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

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
    // type-erased タスクラッパー
    struct ITask {
        virtual ~ITask() = default;
        virtual void start() = 0;
        virtual void update(float dt) = 0;
        virtual void physics(float dt) = 0;
        virtual void draw(RenderContext& ctx) = 0;
        virtual void release() = 0;
    };

    template<typename T>
    struct TaskModel final : ITask {
        T task;
        template<typename... Args>
        TaskModel(Args&&... args) : task(std::forward<Args>(args)...) {}

        void start() override { task.start(); }
        void update(float dt) override { task.update(dt); }
        void physics(float dt) override {
            // physicsメソッドがあれば呼ぶ
            if constexpr (requires(T& t, float d) { t.physics(d); })
                task.physics(dt);
        }
        void draw(RenderContext& ctx) override {
            if constexpr (Drawable<T, RenderContext>)
                task.draw(ctx);
        }
        void release() override { task.release(); }
    };

    struct TaskEntry {
        uint64_t id;
        std::unique_ptr<ITask> impl;
        bool initialized = false;
        bool pending_destroy = false;
    };

    std::array<std::vector<TaskEntry>, static_cast<size_t>(TaskLayer::Max)> layers_;
    uint64_t next_id_ = 1;

public:
    // concept制約付きの登録
    template<TaskLike T, typename... Args>
    TaskHandle register_task(TaskLayer layer, Args&&... args) {
        uint64_t id = next_id_++;
        auto& vec = layers_[static_cast<size_t>(layer)];
        vec.push_back({id, std::make_unique<TaskModel<T>>(std::forward<Args>(args)...), false, false});
        return {id};
    }

    void destroy(TaskHandle handle);
    void run(RunPhase phase, float dt, RenderContext* ctx = nullptr);
};
```

### 4.3 物理システム

#### `engine/physics/collider.hpp`

CppSampleGame の `Collider2D` (継承 + COLLIDER_TYPE enum) を variant で置換。

```cpp
#pragma once
#include <variant>
#include <functional>
#include <cstdint>
#include "../math/vec2.hpp"
#include "../math/transform.hpp"

enum class ColliderTag : uint32_t {
    Invalid = 0,
    Player,
    Enemy,
    Bullet,
    Max
};

struct AABBData {
    Vec2f half_extent;  // CppSampleGame では Size.X/2, Size.Y/2 で計算していた部分
};

struct CircleData {
    float radius;       // CppSampleGame では Size.Radius を使用
};

using ColliderShape = std::variant<AABBData, CircleData>;

struct ColliderHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

struct Collider {
    ColliderHandle handle;
    ColliderShape shape;
    ColliderTag tag = ColliderTag::Invalid;
    uint64_t owner_id = 0;
    const Transform2D* transform = nullptr;
    std::function<bool(const Collider&)> on_hit;
};
```

#### `engine/physics/hit_test.hpp`

CppSampleGame の `IsHitCircle`, `IsHitCircleToAABB`, `AABBCollider::IsHit`, `CircleCollider::IsHit` に対応。

```cpp
#pragma once
#include "collider.hpp"

// AABB vs AABB (CppSampleGame: AABBCollider::IsHit の AABB case)
bool hit_test(const AABBData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb);

// Circle vs Circle (CppSampleGame: CircleCollider::IsHit の CIRCLE case → IsHitCircle)
bool hit_test(const CircleData& a, const Transform2D& ta,
              const CircleData& b, const Transform2D& tb);

// Circle vs AABB (CppSampleGame: IsHitCircleToAABB)
bool hit_test(const CircleData& circle, const Transform2D& tc,
              const AABBData& aabb, const Transform2D& ta);

// AABB vs Circle (引数逆転)
bool hit_test(const AABBData& aabb, const Transform2D& ta,
              const CircleData& circle, const Transform2D& tc);

// 汎用判定 (variant visitor)
bool check_hit(const Collider& a, const Collider& b);
```

#### `engine/physics/hit_test.cpp`

```cpp
#include "hit_test.hpp"
#include <cmath>

// CppSampleGame の IsHitCircle を移植
static bool is_hit_circle(Vec2f a, Vec2f b, float r) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy < r * r;
}

bool hit_test(const AABBData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb) {
    float xa1 = ta.position.x - a.half_extent.x;
    float xa2 = ta.position.x + a.half_extent.x;
    float ya1 = ta.position.y - a.half_extent.y;
    float ya2 = ta.position.y + a.half_extent.y;
    float xb1 = tb.position.x - b.half_extent.x;
    float xb2 = tb.position.x + b.half_extent.x;
    float yb1 = tb.position.y - b.half_extent.y;
    float yb2 = tb.position.y + b.half_extent.y;
    return xa1 <= xb2 && xa2 >= xb1 && ya1 <= yb2 && ya2 >= yb1;
}

bool hit_test(const CircleData& a, const Transform2D& ta,
              const CircleData& b, const Transform2D& tb) {
    return is_hit_circle(ta.position, tb.position, a.radius + b.radius);
}

// CppSampleGame の IsHitCircleToAABB を移植
bool hit_test(const CircleData& circle, const Transform2D& tc,
              const AABBData& aabb, const Transform2D& ta) {
    float cx = tc.position.x;
    float cy = tc.position.y;
    float r = circle.radius;

    float x1 = ta.position.x - aabb.half_extent.x;
    float x2 = ta.position.x + aabb.half_extent.x;
    float y1 = ta.position.y - aabb.half_extent.y;
    float y2 = ta.position.y + aabb.half_extent.y;

    // 拡張矩形と点の判定
    if (x1 - r < cx && cx < x2 + r && y1 < cy && cy < y2) return true;
    if (x1 < cx && cx < x2 && y1 - r < cy && cy < y2 + r) return true;

    // 四隅と円の判定
    Vec2f pos = tc.position;
    if (is_hit_circle({x1, y1}, pos, r)) return true;
    if (is_hit_circle({x2, y1}, pos, r)) return true;
    if (is_hit_circle({x1, y2}, pos, r)) return true;
    if (is_hit_circle({x2, y2}, pos, r)) return true;

    return false;
}

bool hit_test(const AABBData& aabb, const Transform2D& ta,
              const CircleData& circle, const Transform2D& tc) {
    return hit_test(circle, tc, aabb, ta);
}

bool check_hit(const Collider& a, const Collider& b) {
    return std::visit([&](const auto& sa, const auto& sb) {
        return hit_test(sa, *a.transform, sb, *b.transform);
    }, a.shape, b.shape);
}
```

#### `engine/physics/physics_system.hpp`

CppSampleGame の `SysPhysics` に対応。Singleton継承 → inline グローバル変数。

```cpp
#pragma once
#include <array>
#include <vector>
#include <utility>
#include "collider.hpp"

class PhysicsSystem {
    std::array<std::vector<Collider*>,
               static_cast<size_t>(ColliderTag::Max)> colliders_;
    std::vector<Collider*> calc_stack_;     // 移動したオブジェクト
    std::vector<std::pair<Collider*, ColliderTag>> remove_list_;
    uint64_t next_id_ = 1;

    // 判定マトリクス: colliders_[tag] の全要素と判定
    void hit_to_all(Collider* c, ColliderTag target_tag);

public:
    PhysicsSystem();

    ColliderHandle register_collider(Collider& c);
    void remove_collider(Collider& c);
    void mark_moved(Collider& c);  // CppSampleGame の CalcStack 相当
    void run();                     // 判定実行 + remove処理
};

// グローバルインスタンス (Singleton<T> の代替)
inline PhysicsSystem g_physics;
```

### 4.4 ステートマシン

#### `engine/core/state_machine.hpp`

CppSampleGame の `State / StateControl` を std::variant + visit で置換。

```cpp
#pragma once
#include <variant>
#include <type_traits>

template<typename... States>
class StateMachine {
    using StateVariant = std::variant<std::monostate, States...>;
    StateVariant current_;

public:
    template<typename S, typename... Args>
    void transition(Args&&... args) {
        // 現在のステートの exit
        std::visit([](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>) {
                if constexpr (requires { s.exit(); })
                    s.exit();
            }
        }, current_);
        // 新ステートを構築して enter
        current_.template emplace<S>(std::forward<Args>(args)...);
        std::get<S>(current_).enter();
    }

    void update(float dt) {
        std::visit([dt](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>)
                s.update(dt);
        }, current_);
    }

    void draw(auto& ctx) {
        std::visit([&ctx](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>) {
                if constexpr (requires { s.draw(ctx); })
                    s.draw(ctx);
            }
        }, current_);
    }

    template<typename S>
    bool is_state() const {
        return std::holds_alternative<S>(current_);
    }
};
```

### 4.5 ゲームオブジェクト

#### `engine/core/game_object.hpp`

CppSampleGame の `IGameObject` を継承からコンポーネント集約に変更。

```cpp
#pragma once
#include <string>
#include <cstdint>
#include <any>
#include <unordered_map>
#include <typeindex>
#include "../math/transform.hpp"

struct GameObject {
    uint64_t id = 0;
    std::string name_;
    uint32_t object_type_ = 0;
    Transform2D transform_;
    std::unordered_map<std::type_index, std::any> components_;

    // GameObjectLike concept を満たす
    Transform2D& transform() { return transform_; }
    std::string_view name() const { return name_; }
    uint32_t object_type() const { return object_type_; }

    template<typename T>
    void add_component(T&& comp) {
        components_[std::type_index(typeid(std::decay_t<T>))] = std::forward<T>(comp);
    }

    template<typename T>
    T* get_component() {
        auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }

    template<typename T>
    const T* get_component() const {
        auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }
};
```

### 4.6 ID生成

#### `engine/core/id_generator.hpp`

```cpp
#pragma once
#include <cstdint>
#include <atomic>

struct IdGenerator {
    static uint64_t next() {
        static std::atomic<uint64_t> counter{1};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }
};
```

---

## 5. System Layer 詳細

### 5.1 プラットフォーム選択

#### `system/platform.hpp`

```cpp
#pragma once

// コンパイル時プラットフォーム選択
// CppSampleGame の DxLib 依存を Vulkan/WebGPU バックエンドに置換

#if defined(ERGO_PLATFORM_WEB)
    // Web版は TypeScript で別実装
    #error "Web platform uses TypeScript implementation, not C++"
#else
    // ネイティブ版は全て Vulkan
    #include "renderer/vulkan/vk_renderer.hpp"
    #include "input/desktop_input.hpp"
    #include "window/desktop_window.hpp"

    namespace ergo {
        using PlatformRenderer = VulkanRenderer;
        using PlatformInput = DesktopInput;
        using PlatformWindow = DesktopWindow;
    }
#endif
```

### 5.2 Vulkan レンダラー

ハイグラフィック不要のため、最小限の Vulkan 構成。CppSampleGame の `DrawBox`, `DrawCircle`, `DrawString` 等に対応する2D描画プリミティブを提供する。

#### `system/renderer/vulkan/vk_renderer.hpp`

```cpp
#pragma once
#include <cstdint>
#include "engine/math/vec2.hpp"
#include "engine/math/size2.hpp"
#include "engine/math/color.hpp"

struct TextureHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

struct Rect {
    float x = 0, y = 0, w = 1, h = 1;
};

// RenderContext: ゲーム側が描画に使うインターフェース
// CppSampleGame の DrawBox, DrawCircle, DrawString 等の代替
struct RenderContext {
    virtual ~RenderContext() = default;
    virtual void draw_rect(Vec2f pos, Size2f size, Color color, bool filled) = 0;
    virtual void draw_circle(Vec2f center, float radius, Color color, bool filled) = 0;
    virtual void draw_sprite(Vec2f pos, Size2f size, TextureHandle tex, Rect uv) = 0;
    virtual void draw_text(Vec2f pos, const char* text, Color color, float scale) = 0;
};

class VulkanRenderer {
    // Vulkan内部状態 (VkInstance, VkDevice, etc.)
    struct Impl;
    Impl* impl_ = nullptr;

public:
    // RendererBackend concept を満たす
    bool initialize();
    void begin_frame();
    void end_frame();
    void shutdown();

    // RenderContext取得
    RenderContext* context();

    // リソース
    TextureHandle load_texture(const char* path);
    void unload_texture(TextureHandle handle);
};
```

> **NOTE**: `RenderContext` は type-erasure の境界として virtual を使用する。
> これはDLL境界を跨ぐ描画コマンドのディスパッチのためであり、
> エンジン内部のゲームロジック層での継承使用とは異なる。

### 5.3 入力

#### `system/input/desktop_input.hpp`

```cpp
#pragma once
#include <cstdint>
#include "engine/math/vec2.hpp"

// CppSampleGame の CheckHitKey, GetMousePoint の代替
class DesktopInput {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    // InputBackend concept を満たす
    void poll_events();
    bool is_key_down(uint32_t key) const;
    bool is_key_pressed(uint32_t key) const;   // 押した瞬間のみ
    Vec2f mouse_position() const;
    bool is_mouse_button_down(uint32_t button) const;
};
```

### 5.4 ウィンドウ

#### `system/window/desktop_window.hpp`

```cpp
#pragma once
#include <cstdint>
#include <string_view>

// CppSampleGame の ChangeWindowMode, DxLib_Init 等の代替
class DesktopWindow {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    // WindowBackend concept を満たす
    bool create(uint32_t width, uint32_t height, std::string_view title);
    bool should_close() const;
    void poll_events();
    uint32_t width() const;
    uint32_t height() const;

    // Vulkan用サーフェスハンドル取得
    void* get_surface_handle() const;
};
```

---

## 6. Application Layer (ゲーム DLL)

### 6.1 DLL インターフェース

#### `game_interface/engine_types.h`

エンジンとゲームDLLで共有する型定義 (C互換)。

```c
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { float x, y; } ErgoVec2;
typedef struct { float w, h; } ErgoSize2;
typedef struct { uint8_t r, g, b, a; } ErgoColor;
typedef struct { ErgoVec2 position; float rotation; ErgoSize2 size; } ErgoTransform2D;
typedef struct { uint64_t id; } ErgoTaskHandle;
typedef struct { uint64_t id; } ErgoColliderHandle;
typedef struct { uint64_t id; } ErgoTextureHandle;

#ifdef __cplusplus
}
#endif
```

#### `game_interface/game_interface.h`

```c
#pragma once
#include "engine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// エンジンがゲームDLLに提供するAPI
typedef struct {
    // 描画 (CppSampleGame の DrawBox, DrawCircle 等の代替)
    void (*draw_rect)(ErgoVec2 pos, ErgoSize2 size, ErgoColor color, int filled);
    void (*draw_circle)(ErgoVec2 center, float radius, ErgoColor color, int filled);
    void (*draw_text)(ErgoVec2 pos, const char* text, ErgoColor color, float scale);

    // 入力 (CppSampleGame の CheckHitKey, GetMousePoint の代替)
    int (*is_key_down)(uint32_t key);
    int (*is_key_pressed)(uint32_t key);
    ErgoVec2 (*mouse_position)(void);

    // リソース
    ErgoTextureHandle (*load_texture)(const char* path);
    void (*unload_texture)(ErgoTextureHandle handle);
} ErgoEngineAPI;

// ゲームDLLが公開するコールバック
typedef struct {
    void (*on_init)(const ErgoEngineAPI* api);
    void (*on_update)(float dt);
    void (*on_draw)(void);
    void (*on_shutdown)(void);
} ErgoGameCallbacks;

// DLLエントリーポイント (ゲームDLL側が実装)
#ifdef _WIN32
    #define ERGO_EXPORT __declspec(dllexport)
#else
    #define ERGO_EXPORT __attribute__((visibility("default")))
#endif

ERGO_EXPORT ErgoGameCallbacks* ergo_get_game_callbacks(void);

#ifdef __cplusplus
}
#endif
```

### 6.2 サンプルゲーム (CppSampleGame 移植)

以下は CppSampleGame の Player/Enemy/Bullet を Ergo に移植した場合の構造。

#### `samples/shooting/game_types.hpp`

```cpp
#pragma once
#include <cstdint>

// CppSampleGame の GameType.h に対応
enum class GameObjectType : uint32_t {
    Invalid = 0,
    Player,
    Enemy,
    Bullet
};
```

#### `samples/shooting/player.hpp`

CppSampleGame の `Player` クラスの移植例。継承なし、concept 準拠。

```cpp
#pragma once
#include "engine/core/game_object.hpp"
#include "engine/physics/collider.hpp"
#include "game_types.hpp"

struct Player {
    // データ
    GameObject object;
    Collider collider;
    int hp_ = 100;
    int interval_ = 0;
    float ground_y_ = 0.0f;
    float jump_pow_ = 25.0f;
    float jump_y_ = 0.0f;
    float grav_ = 0.0f;

    // TaskLike concept を満たす
    void start();
    void update(float dt);
    void release();

    // Drawable
    void draw(RenderContext& ctx);

    // コライダーコールバック
    bool hit_callback(const Collider& target);
};

// concept検証
static_assert(TaskLike<Player>);
```

---

## 7. ディレクトリツリー

```
Ergo/
├── CMakeLists.txt                   # ルート CMake
├── DESIGN.md                        # 本設計書
├── README.md                        # プロジェクト説明
├── LICENSE                          # MIT License (既存)
├── .gitignore                       # (既存)
│
├── engine/                          # Engine Layer
│   ├── CMakeLists.txt
│   ├── core/
│   │   ├── concepts.hpp             # 全concept定義
│   │   ├── task_system.hpp          # TaskManager
│   │   ├── task_system.cpp
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
│   │   ├── hit_test.hpp             # 衝突判定宣言
│   │   ├── hit_test.cpp             # 衝突判定実装
│   │   ├── physics_system.hpp       # PhysicsSystem
│   │   └── physics_system.cpp
│   └── resource/
│       └── texture_handle.hpp       # TextureHandle
│
├── system/                          # System Layer
│   ├── CMakeLists.txt
│   ├── platform.hpp                 # プラットフォーム選択 using宣言
│   ├── renderer/
│   │   └── vulkan/
│   │       ├── vk_renderer.hpp
│   │       ├── vk_renderer.cpp
│   │       ├── vk_pipeline.hpp
│   │       ├── vk_pipeline.cpp
│   │       ├── vk_swapchain.hpp
│   │       └── vk_swapchain.cpp
│   ├── input/
│   │   ├── desktop_input.hpp
│   │   ├── desktop_input.cpp
│   │   ├── touch_input.hpp          # Android/iOS用
│   │   └── touch_input.cpp
│   └── window/
│       ├── desktop_window.hpp
│       ├── desktop_window.cpp
│       ├── android_window.hpp       # Android用
│       └── ios_window.hpp           # iOS用 (.mm)
│
├── runtime/                         # エンジンランタイム (実行ファイル)
│   ├── CMakeLists.txt
│   ├── main.cpp                     # エントリーポイント
│   ├── engine_context.hpp           # EngineContext構築
│   ├── engine_context.cpp
│   ├── dll_loader.hpp               # ゲームDLLロード
│   └── dll_loader.cpp
│
├── game_interface/                  # DLL境界定義 (共有)
│   ├── engine_types.h               # C互換型定義
│   └── game_interface.h             # DLL API定義
│
├── web/                             # Web版 (TypeScript)
│   ├── package.json
│   ├── tsconfig.json
│   └── src/
│       ├── main.ts
│       ├── engine/
│       │   ├── interfaces.ts        # concept相当のinterface定義
│       │   ├── task_system.ts
│       │   ├── state_machine.ts
│       │   └── game_object.ts
│       ├── math/
│       │   ├── vec2.ts
│       │   └── transform.ts
│       ├── physics/
│       │   ├── collider.ts
│       │   ├── hit_test.ts
│       │   └── physics_system.ts
│       └── renderer/
│           └── webgpu_renderer.ts
│
└── samples/                         # サンプルゲーム (DLL)
    └── shooting/
        ├── CMakeLists.txt
        ├── game_main.cpp            # DLLエントリー (ergo_get_game_callbacks)
        ├── game_types.hpp
        ├── player.hpp
        ├── player.cpp
        ├── enemy.hpp
        ├── enemy.cpp
        ├── bullet.hpp
        ├── bullet.cpp
        └── scenes/
            ├── title_scene.hpp
            ├── title_scene.cpp
            ├── ingame_scene.hpp
            └── ingame_scene.cpp
```

---

## 8. ゲームループ

#### `runtime/main.cpp` の構造

CppSampleGame の `main.cpp` に対応。

```cpp
#include "system/platform.hpp"
#include "engine/core/task_system.hpp"
#include "engine/physics/physics_system.hpp"
#include "runtime/engine_context.hpp"
#include "runtime/dll_loader.hpp"

int main(int argc, char** argv) {
    // 1. プラットフォーム初期化
    ergo::PlatformWindow window;
    window.create(800, 600, "Ergo Engine");

    ergo::PlatformRenderer renderer;
    renderer.initialize();

    ergo::PlatformInput input;

    // 2. エンジンシステム (グローバルインスタンス使用)
    // g_physics は inline 変数として physics_system.hpp で定義済み

    // 3. タスクマネージャ
    TaskManager task_mgr;

    // 4. EngineContext 構築 & ゲームDLLロード
    auto engine_api = build_engine_api(renderer, input);
    auto game = load_game_dll("game.dll");  // or .so
    game.on_init(&engine_api);

    // 5. メインループ (CppSampleGame の while ループ対応)
    float last_time = 0.0f; // プラットフォームの時間取得関数を使用
    while (!window.should_close()) {
        float now = /* get_time() */;
        float dt = now - last_time;
        last_time = now;

        window.poll_events();
        input.poll_events();

        // DESTROY (CppSampleGame: TaskManager::Run(RUN_TYPE::DESTROY))
        task_mgr.run(RunPhase::Destroy, dt);

        // PHYSICS (CppSampleGame: TaskManager::Run(RUN_TYPE::PHYSICS))
        task_mgr.run(RunPhase::Physics, dt);

        // UPDATE (CppSampleGame: TaskManager::Run(RUN_TYPE::DO))
        task_mgr.run(RunPhase::Update, dt);
        game.on_update(dt);

        // 衝突判定 (CppSampleGame: SysPhysics::Run())
        g_physics.run();

        // DRAW (CppSampleGame: TaskManager::Run(RUN_TYPE::DRAW))
        renderer.begin_frame();
        auto* ctx = renderer.context();
        task_mgr.run(RunPhase::Draw, dt, ctx);
        game.on_draw();
        renderer.end_frame();
    }

    // 6. 終了 (CppSampleGame: SysPhysics::Release(), TaskManager::Release())
    game.on_shutdown();
    renderer.shutdown();
    return 0;
}
```

---

## 9. Web版の対応方針

### 9.1 C++ → TypeScript 対応表

| C++ (Ergo) | TypeScript (Web) |
|------------|-----------------|
| `concept TaskLike` | `interface TaskLike` |
| `std::variant<AABBData, CircleData>` | `type ColliderShape = AABBData \| CircleData` |
| `std::visit` | switch + type guard |
| `StateMachine<States...>` | discriminated union + switch |
| DLL ロード | ES Module の動的 `import()` |
| Vulkan | WebGPU |
| C API 境界 | TypeScript interface |
| `inline PhysicsSystem g_physics` | `export const physics = new PhysicsSystem()` |

### 9.2 Web版ゲームモジュール

```typescript
// web/src/engine/interfaces.ts
export interface TaskLike {
    start(): void;
    update(dt: number): void;
    release(): void;
}

export interface GameModule {
    onInit(ctx: EngineContext): void;
    onUpdate(dt: number): void;
    onDraw(ctx: RenderContext): void;
    onShutdown(): void;
}
```

---

## 10. ビルドシステム

### 10.1 ルート CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(Ergo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# オプション
option(ERGO_BUILD_SAMPLES "Build sample games" ON)

# Engine Layer (静的ライブラリ)
add_subdirectory(engine)

# System Layer (静的ライブラリ)
add_subdirectory(system)

# Runtime (実行ファイル)
add_subdirectory(runtime)

# Sample Game (共有ライブラリ)
if(ERGO_BUILD_SAMPLES)
    add_subdirectory(samples/shooting)
endif()
```

### 10.2 engine/CMakeLists.txt

```cmake
add_library(ergo_engine STATIC
    core/task_system.cpp
    physics/hit_test.cpp
    physics/physics_system.cpp
)
target_include_directories(ergo_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
```

### 10.3 system/CMakeLists.txt

```cmake
find_package(Vulkan REQUIRED)

add_library(ergo_system STATIC
    renderer/vulkan/vk_renderer.cpp
    input/desktop_input.cpp
    window/desktop_window.cpp
)
target_link_libraries(ergo_system PUBLIC Vulkan::Vulkan ergo_engine)
target_include_directories(ergo_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
```

### 10.4 runtime/CMakeLists.txt

```cmake
add_executable(ergo_runtime
    main.cpp
    engine_context.cpp
    dll_loader.cpp
)
target_link_libraries(ergo_runtime PRIVATE ergo_system)
target_include_directories(ergo_runtime PRIVATE
    ${CMAKE_SOURCE_DIR}/game_interface
)
```

### 10.5 samples/shooting/CMakeLists.txt

```cmake
add_library(shooting_game SHARED
    game_main.cpp
    player.cpp
    enemy.cpp
    bullet.cpp
    scenes/title_scene.cpp
    scenes/ingame_scene.cpp
)
target_link_libraries(shooting_game PRIVATE ergo_engine)
target_include_directories(shooting_game PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/game_interface
)
```

### 10.6 プラットフォーム別ビルドコマンド

```bash
# Windows
cmake -B build -G "Visual Studio 17 2022" && cmake --build build

# Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

# Android
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a

# iOS
cmake -B build -G Xcode -DCMAKE_SYSTEM_NAME=iOS

# Web (TypeScript別ビルド)
cd web && npm install && npm run build
```

---

## 11. CppSampleGame からの変更点まとめ

| 項目 | CppSampleGame | Ergo |
|------|--------------|------|
| 型制約 | 仮想関数 + 継承 | concept + variant |
| シングルトン | `Singleton<T>` テンプレート継承 | inline グローバル変数 |
| コライダー種別 | enum + switch + 継承 | `std::variant<AABBData, CircleData>` + visit |
| メモリ管理 | `shared_ptr` 多用 | ID + 値型中心 |
| 描画 | DxLib 直接呼び出し | Vulkan/WebGPU バックエンド |
| ゲーム構造 | 直接リンク | DLL/SO 分離 |
| `Vector2` union | `X/W/Radius` 共用体 | 型安全な `Vec2f` + `Size2f` |
| プリコンパイルヘッダ | `stdafx.h` | CMake PCH |
| プラットフォーム | Windows (DxLib) のみ | PC/Android/iOS/Web |
| ステートマシン | `State` 仮想基底 + `StateControl` | `StateMachine<States...>` variant |
| タスクレイヤー | `TASK_LAYER` enum + 配列 | `TaskLayer` enum class + `std::array` |

---

## 12. 今後の拡張ポイント

| 項目 | 概要 |
|------|------|
| ECS | 現在のコンポーネント集約をフルEntity Component Systemに発展 |
| シリアライゼーション | コンポーネントの保存/復元でシーンエディタ対応 |
| ホットリロード | DLL の動的リロードによる開発中のロジック更新 |
| マルチスレッド | PhysicsSystem の並列化 (CppSampleGame のTODO) |
| アセットパイプライン | テクスチャ/サウンドのビルド時変換 |
| imgui統合 | デバッグUI |
| OBBコライダー | CppSampleGame で未実装の OBBCollider 追加 |
