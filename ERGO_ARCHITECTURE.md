# Ergo アーキテクチャ設計書 — Ludus拡張仕様

> **バージョン**: 1.0
> **更新日**: 2026-02-21
> **前提**: Ergo 軽量クロスプラットフォームゲームエンジン (C++20, concept-based)

---

## 目次

1. [概要](#1-概要)
2. [用語定義](#2-用語定義)
3. [全体アーキテクチャ](#3-全体アーキテクチャ)
4. [ドメイン (Domain)](#4-ドメイン-domain)
5. [コンポーネント (Component)](#5-コンポーネント-component)
6. [リレーション (Relation)](#6-リレーション-relation)
7. [仕様記述フォーマット (Spec DSL)](#7-仕様記述フォーマット-spec-dsl)
8. [コンポーネント解決パイプライン](#8-コンポーネント解決パイプライン)
9. [モノリシックビルド](#9-モノリシックビルド)
10. [既存Ergoレイヤーとの統合](#10-既存ergoレイヤーとの統合)
11. [実行フロー](#11-実行フロー)
12. [ディレクトリ構成](#12-ディレクトリ構成)
13. [拡張性と制約](#13-拡張性と制約)

---

## 1. 概要

### 1.1 目的

Ergoは軽量クロスプラットフォームゲームエンジンとして、C++20のconcept-basedポリモーフィズムによる3層アーキテクチャ(Application / Engine / System)を採用している。

本設計書では、Ludusのコンポーネントベース仕様定義とオープンアーキテクチャをErgoの正規拡張として導入し、**宣言的なゲーム仕様からモノリシックなゲームバイナリを自動構築する**仕組みを定義する。

### 1.2 設計思想

```
従来のErgo:
  開発者が Player, Enemy, Bullet ... を手書き → TaskSystemに登録 → ゲーム動作

Ludus拡張後のErgo:
  仕様定義 (Domain/Component/Relation) → 自動検索/生成 → モノリシックビルド → ゲーム動作
```

核となる考え方は以下の3点である。

1. **宣言的仕様記述** — 何が必要かを書く。どう実装するかはErgoが解決する
2. **コンポーネント自動解決** — 既存コンポーネントの検索、なければ生成
3. **モノリシック組立** — 解決済みコンポーネントを単一のゲームバイナリとして合成

---

## 2. 用語定義

| 用語 | 定義 | 例 |
|------|------|-----|
| **ドメイン (Domain)** | ゲーム内に登場する要素の分類単位。1つのドメインは1種類のゲーム内存在を表す | `Player`, `Enemy`, `Bullet`, `Item`, `UI` |
| **コンポーネント (Component)** | ドメインを構成する機能の最小単位。単一の責務を持つ振る舞いまたはデータ | `Input`, `Movement`, `Attack`, `Health`, `Render` |
| **リレーション (Relation)** | 2つのドメイン間の干渉規則。方向性と効果を持つ | `Player --attack--> Enemy`, `Bullet --damage--> Enemy` |
| **仕様 (Spec)** | ドメイン・コンポーネント・リレーションの宣言的記述全体 | `.ergo` ファイル |
| **コンポーネントレジストリ** | 利用可能なコンポーネント実装の索引 | ファイルシステム + メタデータ |
| **モノリシックコンポーネント** | 全ドメインとリレーションを合成した単一の実行可能ゲーム単位 | 最終ゲームDLL/バイナリ |

---

## 3. 全体アーキテクチャ

### 3.1 レイヤー構成

```
┌─────────────────────────────────────────────────────────┐
│                    仕様記述層 (Spec Layer)                │
│  .ergo ファイル: Domain / Component / Relation 定義       │
└───────────────────────┬─────────────────────────────────┘
                        │ パース
                        ▼
┌─────────────────────────────────────────────────────────┐
│               コンポーネント解決層 (Resolver Layer)        │
│  検索 → マッチング → 不足検出 → 生成 → 検証               │
└───────────────────────┬─────────────────────────────────┘
                        │ 解決済みコンポーネント群
                        ▼
┌─────────────────────────────────────────────────────────┐
│              モノリシック合成層 (Assembly Layer)           │
│  コンポーネント結合 → リレーション配線 → ゲーム構築         │
└───────────────────────┬─────────────────────────────────┘
                        │ 合成済みゲームモジュール
                        ▼
┌─────────────────────────────────────────────────────────┐
│  Application Layer  │  Engine Layer  │  System Layer     │
│  (既存Ergo 3層アーキテクチャ)                              │
└─────────────────────────────────────────────────────────┘
```

### 3.2 データフロー

```
.ergo (仕様)
    │
    ├─→ SpecParser         仕様をASTに変換
    │       │
    │       ▼
    ├─→ ComponentResolver  コンポーネントの検索と解決
    │       │
    │       ├─→ Registry検索     既存コンポーネントを走査
    │       ├─→ マッチング判定    concept適合性を検証
    │       └─→ ComponentGenerator  不足分を自動生成
    │               │
    │               ▼
    └─→ GameAssembler      モノリシックコンポーネントとして合成
            │
            ├─→ DomainComposer     ドメインごとのコンポーネント結合
            ├─→ RelationWirer      リレーションの配線
            └─→ MonolithBuilder    単一ゲームモジュール出力
```

---

## 4. ドメイン (Domain)

### 4.1 定義

ドメインはゲーム内に登場する要素の**カテゴリ**である。Ergoの既存アーキテクチャにおける `GameObject` + `TaskLike` の組に対応する。

1つのドメインは以下を持つ:

| 属性 | 型 | 説明 |
|------|-----|------|
| `name` | 識別子 | ドメインの一意な名前 (`Player`, `Enemy` 等) |
| `components` | コンポーネントリスト | このドメインが必要とする機能の列挙 |
| `tags` | タグセット | 分類用のメタデータ (`controllable`, `hostile`, `projectile` 等) |
| `multiplicity` | 単数/複数 | インスタンスが1つか複数か (`single` / `multiple`) |

### 4.2 ドメインとErgo既存型の対応

```
Domain "Player"
  ├─ 対応する既存型: samples/shooting/player.hpp の Player struct
  ├─ ECS上: Archetype { Transform2D, Health, InputState, ... }
  └─ TaskSystem上: TaskLayer::Default に登録される TaskLike 型

Domain "Enemy"
  ├─ 対応する既存型: samples/shooting/enemy.hpp の Enemy struct
  ├─ ECS上: Archetype { Transform2D, Health, AI, ... }
  └─ TaskSystem上: TaskLayer::Default に登録される TaskLike 型

Domain "Bullet"
  ├─ 対応する既存型: samples/shooting/bullet.hpp の Bullet struct
  ├─ ECS上: Archetype { Transform2D, Velocity, Lifetime, ... }
  └─ TaskSystem上: TaskLayer::Bullet に登録される TaskLike 型
```

### 4.3 ドメインの制約

- ドメイン名はSpec内で一意でなければならない
- すべてのドメインは暗黙的に `Transform2D` (または `Transform3D`) コンポーネントを持つ
- `TaskLike` conceptの満足が保証される (`start()`, `update(dt)`, `release()`)

---

## 5. コンポーネント (Component)

### 5.1 定義

コンポーネントはドメインを構成する**機能の最小単位**である。単一責務の原則に従い、1つのコンポーネントは1つの振る舞いまたはデータ集合を担う。

### 5.2 コンポーネントの分類

```
Component
  ├─ DataComponent      データのみを保持する (状態)
  │    例: Health { hp, max_hp }
  │    例: Velocity { vx, vy }
  │
  ├─ BehaviourComponent 毎フレーム実行されるロジック (振る舞い)
  │    例: Movement { update(dt) で位置を更新 }
  │    例: Input { update(dt) でキー入力を取得 }
  │
  └─ EventComponent     特定条件で発火するコールバック (反応)
       例: OnDamage { hp減少時に呼ばれる }
       例: OnDestroy { 破壊時に呼ばれる }
```

### 5.3 標準コンポーネントカタログ

Ergoが提供する組み込みコンポーネント群:

| カテゴリ | コンポーネント名 | 種別 | 説明 |
|---------|----------------|------|------|
| **空間** | `Transform` | Data | 位置・回転・スケール |
| | `Velocity` | Data | 速度ベクトル |
| | `Movement` | Behaviour | Transform + Velocity に基づく移動処理 |
| **入力** | `Input` | Behaviour | プレイヤー入力の取得と状態管理 |
| | `InputState` | Data | 現フレームの入力状態 |
| **戦闘** | `Health` | Data | HP・最大HP |
| | `Attack` | Behaviour | 攻撃判定の発行 |
| | `Damage` | Event | ダメージ受信時の処理 |
| **物理** | `Collider` | Data | 衝突形状 (AABB / Circle) |
| | `RigidBody` | Behaviour | 物理演算 (重力・反発) |
| **描画** | `Sprite` | Data | スプライト画像参照 |
| | `Renderer` | Behaviour | 描画コマンドの発行 |
| | `Animator` | Behaviour | アニメーション再生 |
| **AI** | `AI` | Behaviour | 行動決定ロジック |
| | `Patrol` | Behaviour | パトロール移動 |
| | `Chase` | Behaviour | 対象追跡 |
| **ライフ** | `Lifetime` | Behaviour | 生存時間管理 (Bullet等) |
| | `Spawner` | Behaviour | 他ドメインのインスタンス生成 |

### 5.4 コンポーネントのconcept要件

各コンポーネント種別はErgoのconcept体系に準拠する:

```cpp
// DataComponent: POD的データ構造
template<typename T>
concept DataComponentLike = std::is_default_constructible_v<T> &&
    requires {
        { T::component_name() } -> std::convertible_to<std::string_view>;
    };

// BehaviourComponent: 毎フレーム更新される振る舞い
template<typename T>
concept BehaviourComponentLike = DataComponentLike<T> &&
    Startable<T> && Updatable<T> && Releasable<T>;

// EventComponent: コールバック駆動の反応
template<typename T>
concept EventComponentLike = DataComponentLike<T> &&
    requires(T t) {
        { t.event_name() } -> std::convertible_to<std::string_view>;
    };
```

### 5.5 コンポーネントのメタデータ

各コンポーネント実装は自己記述的なメタデータを公開する:

```cpp
struct ComponentMeta {
    std::string_view name;          // "Movement"
    std::string_view category;      // "spatial"
    ComponentKind kind;             // Data / Behaviour / Event
    std::vector<std::string_view> dependencies;  // {"Transform", "Velocity"}
    std::vector<std::string_view> tags;          // {"physics", "motion"}
};
```

---

## 6. リレーション (Relation)

### 6.1 定義

リレーションは**2つのドメイン間の干渉規則**を宣言的に記述する。Ergoの既存アーキテクチャでは `PhysicsSystem` のコリジョンコールバックとして暗黙的に実装されていたものを、明示的な仕様として昇格させる。

### 6.2 リレーションの構造

```
Relation {
    source:    ドメイン名     (干渉の起点)
    target:    ドメイン名     (干渉の対象)
    kind:      リレーション種別
    effect:    効果記述
    condition: 発動条件 (省略可)
}
```

### 6.3 リレーション種別

| 種別 | 説明 | 実装マッピング |
|------|------|--------------|
| `collision` | 物理的な衝突 | `PhysicsSystem` の衝突判定 |
| `damage` | ダメージの授受 | `Health` コンポーネントへのHP減算 |
| `trigger` | トリガー領域への侵入 | 非物理的な領域判定 |
| `spawn` | 新規インスタンスの生成 | `Spawner` コンポーネント経由 |
| `destroy` | 対象の破壊 | `TaskManager::destroy()` 呼び出し |
| `notify` | イベント通知 | `EventComponent` への通知発行 |
| `custom` | ユーザー定義の干渉 | カスタムコールバック |

### 6.4 リレーションの例

```
Player --[collision]--> Enemy    : "接触時にダメージ交換"
Bullet --[damage]----> Enemy    : "弾がHPを1減らす"
Enemy  --[spawn]-----> Bullet   : "Enemyが弾を生成"
Item   --[trigger]---> Player   : "アイテム取得でHP回復"
Player --[spawn]-----> Bullet   : "攻撃ボタンで弾を発射"
```

### 6.5 リレーションの解決

リレーションは以下の手順でエンジンの実行コードに変換される:

```
1. source と target のドメインが Collider を持つか確認
   └─ 持たなければ Collider コンポーネントを自動付与

2. collision / trigger の場合
   └─ PhysicsSystem にコリジョンペアを登録
      source.collider_tag と target.collider_tag の組で判定

3. damage の場合
   └─ target に Health コンポーネントがあることを確認
      コリジョンコールバック内で Health::apply_damage() を呼び出す

4. spawn の場合
   └─ source に Spawner コンポーネントを付与
      Spawner の生成対象を target のドメインに設定

5. 配線コードを生成
   └─ MonolithBuilder がリレーションごとの接続コードを出力
```

---

## 7. 仕様記述フォーマット (Spec DSL)

### 7.1 `.ergo` ファイル形式

仕様はYAMLベースの `.ergo` ファイルに記述する。

### 7.2 基本構文

```yaml
# game.ergo — シューティングゲームの仕様定義
ergo: "1.0"
game:
  name: "ShootingGame"
  type: "2d"

# ─── ドメイン定義 ───
domains:

  Player:
    tags: [controllable, character]
    multiplicity: single
    components:
      - Input
      - Movement
      - Attack
      - Health
      - Collider:
          shape: aabb
          tag: player
      - Sprite
      - Renderer

  Enemy:
    tags: [hostile, character]
    multiplicity: multiple
    components:
      - AI:
          behaviour: patrol
      - Movement:
          speed: 2.0
      - Health:
          hp: 3
      - Collider:
          shape: aabb
          tag: enemy
      - Sprite
      - Renderer

  Bullet:
    tags: [projectile]
    multiplicity: multiple
    components:
      - Movement:
          speed: 10.0
          direction: forward
      - Lifetime:
          seconds: 3.0
      - Collider:
          shape: circle
          tag: bullet
      - Sprite
      - Renderer

  Item:
    tags: [pickup]
    multiplicity: multiple
    components:
      - Collider:
          shape: aabb
          tag: item
      - Sprite
      - Renderer

# ─── リレーション定義 ───
relations:

  - source: Player
    target: Enemy
    kind: collision
    effect:
      type: damage
      value: 1
      mutual: true

  - source: Bullet
    target: Enemy
    kind: damage
    effect:
      type: hp_reduce
      value: 1
      on_kill: destroy_both

  - source: Player
    target: Bullet
    kind: spawn
    effect:
      trigger: input.attack
      position: source.front
      limit: 5

  - source: Enemy
    target: Bullet
    kind: spawn
    effect:
      trigger: ai.attack
      interval: 2.0

  - source: Item
    target: Player
    kind: trigger
    effect:
      type: heal
      value: 1
      on_pickup: destroy_source
```

### 7.3 仕様のバリデーション規則

| 規則 | 内容 |
|------|------|
| ドメイン名の一意性 | 同名のドメインは定義不可 |
| コンポーネント存在性 | 参照されるコンポーネントはカタログまたはカスタム定義に存在すること |
| リレーション整合性 | source/target のドメインが定義済みであること |
| 循環依存の禁止 | コンポーネントの依存関係に循環がないこと |
| 型安全性 | パラメータの型がコンポーネントのスキーマに適合すること |

---

## 8. コンポーネント解決パイプライン

### 8.1 概要

仕様に記述されたコンポーネントを実行可能なC++実装に変換するパイプライン。

```
Spec AST
    │
    ▼
┌──────────────────────────┐
│  Phase 1: 依存解析        │  コンポーネント間の依存グラフを構築
│  (DependencyAnalyzer)    │  暗黙的依存の追加 (Movement → Transform + Velocity)
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│  Phase 2: レジストリ検索  │  既存コンポーネント実装をファイルシステムから走査
│  (RegistryScanner)       │  メタデータのマッチング判定
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│  Phase 3: 不足検出        │  未解決のコンポーネントを特定
│  (GapDetector)           │  解決レポート出力
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│  Phase 4: コード生成      │  不足コンポーネントのスケルトンを自動生成
│  (ComponentGenerator)    │  concept適合性を保証するテンプレートから生成
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│  Phase 5: 検証            │  全コンポーネントのconcept適合をstatic_assertで確認
│  (ConceptValidator)      │  コンパイルテストによる最終検証
└──────────────────────────┘
```

### 8.2 Phase 1: 依存解析

コンポーネントは他のコンポーネントに依存できる。暗黙的な依存関係を自動的に追加する。

```
明示的依存:
  Movement → Transform, Velocity    (移動にはTransformとVelocityが必要)
  Attack → Collider, Spawner        (攻撃にはColliderとSpawnerが必要)
  AI → Movement                     (AIには移動が必要)

暗黙的依存 (全ドメイン共通):
  すべてのドメイン → Transform      (空間上に存在するため)
```

依存解析後の展開例:

```
Domain "Player" (明示記述):
  Input, Movement, Attack, Health, Collider, Sprite, Renderer

Domain "Player" (依存解析後):
  Transform, Velocity, InputState,          ← 暗黙的に追加
  Input, Movement, Attack, Health,
  Collider, Spawner,                        ← Attackの依存で追加
  Sprite, Renderer
```

### 8.3 Phase 2: レジストリ検索

コンポーネントレジストリは以下の優先順位で検索される:

```
検索順序:
  1. プロジェクトローカル    ./components/
  2. ゲーム固有             ./game/components/
  3. Ergo標準              engine/components/  (組み込みカタログ)
  4. 外部プラグイン         plugins/*/components/
```

マッチング判定:

```
コンポーネント "Movement" の検索
  │
  ├─ ファイル走査: movement.hpp を検索
  ├─ メタデータ確認: ComponentMeta::name == "Movement"
  ├─ concept適合: BehaviourComponentLike<Movement> が真
  ├─ 依存充足: Transform, Velocity が解決済み
  │
  └─ 結果: ✅ 解決 / ❌ 未解決
```

### 8.4 Phase 3: 不足検出

未解決コンポーネントの一覧をレポートとして出力する:

```
=== コンポーネント解決レポート ===
Domain: Player
  ✅ Transform       (engine/components/transform.hpp)
  ✅ Velocity        (engine/components/velocity.hpp)
  ✅ Input           (engine/components/input.hpp)
  ✅ Movement        (engine/components/movement.hpp)
  ❌ Attack          (未解決 → 生成対象)
  ✅ Health          (engine/components/health.hpp)
  ✅ Collider        (engine/physics/collider.hpp)
  ❌ Sprite          (未解決 → 生成対象)
  ❌ Renderer        (未解決 → 生成対象)

未解決: 3件 → Phase 4 で生成
```

### 8.5 Phase 4: コード生成

未解決コンポーネントをテンプレートから自動生成する。

生成テンプレート例 (BehaviourComponent):

```cpp
// 自動生成: components/generated/attack.hpp
#pragma once
#include "engine/core/concepts.hpp"
#include "component_meta.hpp"

struct Attack {
    // --- メタデータ ---
    static constexpr std::string_view component_name() { return "Attack"; }
    static constexpr ComponentMeta meta() {
        return { "Attack", "combat", ComponentKind::Behaviour,
                 {"Collider", "Spawner"}, {"combat"} };
    }

    // --- データ ---
    float cooldown = 0.5f;
    float timer = 0.0f;

    // --- BehaviourComponentLike を満足 ---
    void start() { timer = 0.0f; }
    void update(float dt) {
        timer -= dt;
        // TODO: 攻撃ロジックを実装
    }
    void release() {}
};

static_assert(BehaviourComponentLike<Attack>);
```

### 8.6 Phase 5: 検証

全コンポーネントがconcept要件を満たすことをコンパイル時に検証する:

```cpp
// 自動生成: generated/concept_checks.hpp
#pragma once

// Domain: Player のコンポーネント検証
static_assert(DataComponentLike<Transform>);
static_assert(DataComponentLike<Velocity>);
static_assert(BehaviourComponentLike<Input>);
static_assert(BehaviourComponentLike<Movement>);
static_assert(BehaviourComponentLike<Attack>);
static_assert(DataComponentLike<Health>);
static_assert(DataComponentLike<Collider>);

// Domain: Enemy のコンポーネント検証
static_assert(BehaviourComponentLike<AI>);
// ... (全ドメインについて同様)
```

---

## 9. モノリシックビルド

### 9.1 概要

解決済みの全コンポーネントを**単一のゲームモジュール**として合成する。

### 9.2 合成プロセス

```
Step 1: DomainComposer — ドメインごとのコンポーネント結合
  ┌─────────────────────────────────────────────────┐
  │  Domain "Player" の合成結果:                      │
  │                                                   │
  │  struct PlayerDomain {                            │
  │      Transform2D transform;                       │
  │      Velocity velocity;                           │
  │      InputState input_state;                      │
  │      Input input;                                 │
  │      Movement movement;                           │
  │      Attack attack;                               │
  │      Health health;                               │
  │      Collider collider;                           │
  │      Sprite sprite;                               │
  │      Renderer renderer;                           │
  │                                                   │
  │      void start();     // 全BehaviourのstartをチェーンCall  │
  │      void update(dt);  // 全Behaviourのupdateを順次実行    │
  │      void release();   // 全Behaviourのreleaseを逆順実行   │
  │  };                                               │
  │  static_assert(TaskLike<PlayerDomain>);           │
  └─────────────────────────────────────────────────┘

Step 2: RelationWirer — リレーションの配線コード生成
  ┌─────────────────────────────────────────────────┐
  │  // Player → Enemy の collision リレーション       │
  │  collision_rules.add({                            │
  │      .tag_a = ColliderTag::Player,                │
  │      .tag_b = ColliderTag::Enemy,                 │
  │      .callback = [](auto& a, auto& b) {          │
  │          // effect: damage, value: 1, mutual      │
  │          if (auto* h = b.get<Health>())           │
  │              h->apply_damage(1);                  │
  │          if (auto* h = a.get<Health>())           │
  │              h->apply_damage(1);                  │
  │      }                                            │
  │  });                                              │
  └─────────────────────────────────────────────────┘

Step 3: MonolithBuilder — 単一ゲームモジュール出力
  ┌─────────────────────────────────────────────────┐
  │  // generated/game_monolith.hpp                   │
  │  struct GameMonolith {                            │
  │      // ドメインインスタンス管理                       │
  │      PlayerDomain player;                         │
  │      std::vector<EnemyDomain> enemies;            │
  │      std::vector<BulletDomain> bullets;           │
  │      std::vector<ItemDomain> items;               │
  │                                                   │
  │      // リレーション配線                             │
  │      CollisionRuleSet collision_rules;            │
  │      SpawnRuleSet spawn_rules;                    │
  │                                                   │
  │      // TaskLike インターフェース                    │
  │      void start();                                │
  │      void update(float dt);                       │
  │      void release();                              │
  │                                                   │
  │      // DLL境界エクスポート                          │
  │      static ErgoGameCallbacks* get_callbacks();   │
  │  };                                               │
  └─────────────────────────────────────────────────┘
```

### 9.3 実行順序の決定

モノリシックコンポーネント内の実行順序は、コンポーネントの依存グラフをトポロジカルソートして決定する:

```
実行順序 (1フレームのupdate):

  1. Input系         InputState の更新、キー取得
  2. AI系            敵の行動決定
  3. Movement系      座標の更新
  4. Attack/Spawn系  攻撃判定の発行、新規インスタンス生成
  5. Physics系       物理演算、衝突判定
  6. Relation解決    コリジョンコールバック、ダメージ適用
  7. Lifetime系      生存時間の消費、期限切れの破壊
  8. Render系        描画コマンド発行
```

---

## 10. 既存Ergoレイヤーとの統合

### 10.1 TaskSystem との統合

```
.ergo 仕様
    │
    ▼
DomainComposer が各ドメインを TaskLike 型として合成
    │
    ▼
TaskManager::register_task<PlayerDomain>(TaskLayer::Default)
TaskManager::register_task<EnemyDomain>(TaskLayer::Default)
TaskManager::register_task<BulletDomain>(TaskLayer::Bullet)
    │
    ▼
既存の RunPhase サイクルで実行 (Start → Update → Physics → Draw → Destroy)
```

### 10.2 ECS (World) との統合

```
ドメインのコンポーネント構成 → ECS の Archetype に対応

Domain "Player" { Transform, Velocity, Health, Input, ... }
    ↓
World::create_entity() → entity_id
World::add_component<Transform>(entity_id, transform)
World::add_component<Velocity>(entity_id, velocity)
World::add_component<Health>(entity_id, health)
    ↓
World::each<Transform, Velocity>([](id, t, v) {
    // Movement の処理: 全ドメインの Movement が一括実行される
    t.position += v.direction * v.speed * dt;
});
```

### 10.3 PhysicsSystem との統合

```
Relation { kind: collision }
    ↓
PhysicsSystem のコリジョンペア登録:
  g_physics.register_pair(ColliderTag::Player, ColliderTag::Enemy, callback)
  g_physics.register_pair(ColliderTag::Bullet, ColliderTag::Enemy, callback)
    ↓
既存の PhysicsSystem::run() で衝突判定が自動実行
```

### 10.4 DLL境界との統合

```
GameMonolith
    ↓
game_interface.h の ErgoGameCallbacks に接続:
  on_init    → GameMonolith::start()
  on_update  → GameMonolith::update(dt)
  on_draw    → GameMonolith::draw(ctx)
  on_shutdown → GameMonolith::release()
    ↓
runtime/main.cpp の既存ゲームループで実行
```

---

## 11. 実行フロー

### 11.1 ビルド時フロー

```
[開発者] game.ergo を記述
           │
           ▼
[ergo-cli] ergo build game.ergo
           │
           ├─ 1. SpecParser: .ergoファイルをパース
           ├─ 2. DependencyAnalyzer: 依存グラフ構築
           ├─ 3. RegistryScanner: 既存コンポーネント検索
           ├─ 4. GapDetector: 不足コンポーネント検出
           ├─ 5. ComponentGenerator: 不足分を生成
           ├─ 6. ConceptValidator: concept適合検証
           ├─ 7. DomainComposer: ドメイン合成
           ├─ 8. RelationWirer: リレーション配線
           └─ 9. MonolithBuilder: ゲームDLLを出力
                   │
                   ▼
              game.dll / game.so
```

### 11.2 実行時フロー

```
[ergo-runtime] ゲームDLLをロード
           │
           ├─ ergo_get_game_callbacks() を取得
           │
           ▼
[GameLoop]
  ├─ on_init()
  │    └─ GameMonolith::start()
  │         ├─ 全ドメインの start() を呼び出し
  │         └─ リレーションの初期配線
  │
  ├─ 毎フレーム:
  │    ├─ on_update(dt)
  │    │    └─ GameMonolith::update(dt)
  │    │         ├─ Input → AI → Movement → Attack → Physics → Relation → Lifetime
  │    │         └─ Spawn による新規エンティティ追加
  │    │
  │    └─ on_draw()
  │         └─ GameMonolith::draw(ctx)
  │              └─ 全ドメインの Renderer コンポーネントが描画
  │
  └─ on_shutdown()
       └─ GameMonolith::release()
```

---

## 12. ディレクトリ構成

```
Ergo/
├── engine/
│   ├── core/               # 既存: TaskSystem, concepts, GameObject
│   ├── math/               # 既存: Vec2f, Transform2D, etc.
│   ├── physics/            # 既存: Collider, PhysicsSystem
│   ├── ecs/                # 既存: World, Archetype
│   ├── components/         # 【新規】標準コンポーネントカタログ
│   │   ├── transform.hpp
│   │   ├── velocity.hpp
│   │   ├── health.hpp
│   │   ├── input.hpp
│   │   ├── movement.hpp
│   │   ├── attack.hpp
│   │   ├── sprite.hpp
│   │   ├── renderer.hpp
│   │   ├── ai.hpp
│   │   ├── lifetime.hpp
│   │   ├── spawner.hpp
│   │   └── component_meta.hpp
│   └── ludus/              # 【新規】Ludus拡張コア
│       ├── spec_parser.hpp        # .ergo ファイルパーサー
│       ├── dependency_analyzer.hpp # 依存解析
│       ├── registry_scanner.hpp   # コンポーネントレジストリ検索
│       ├── gap_detector.hpp       # 不足検出
│       ├── component_generator.hpp # コード生成
│       ├── concept_validator.hpp  # concept適合検証
│       ├── domain_composer.hpp    # ドメイン合成
│       ├── relation_wirer.hpp     # リレーション配線
│       └── monolith_builder.hpp   # モノリシックビルド
│
├── tools/
│   └── ergo-cli/           # 【新規】CLIツール
│       ├── main.cpp
│       ├── build_command.cpp
│       └── validate_command.cpp
│
├── specs/                  # 【新規】仕様ファイル置き場
│   └── examples/
│       ├── shooting.ergo
│       ├── platformer.ergo
│       └── puzzle.ergo
│
├── generated/              # 【新規】自動生成コード出力先
│   ├── components/
│   ├── domains/
│   └── game_monolith.hpp
│
├── system/                 # 既存: Vulkan, Input, Window, Audio
├── runtime/                # 既存: main.cpp, DLLローダー
├── samples/                # 既存: サンプルゲーム
└── tests/                  # 既存 + Ludus拡張のテスト追加
    └── ludus/
        ├── test_spec_parser.cpp
        ├── test_dependency_analyzer.cpp
        ├── test_registry_scanner.cpp
        └── test_monolith_builder.cpp
```

---

## 13. 拡張性と制約

### 13.1 オープンアーキテクチャ

Ludus拡張はオープンアーキテクチャとして以下を保証する:

| 拡張ポイント | 方法 |
|-------------|------|
| **カスタムコンポーネントの追加** | `components/` にヘッダを置き、`ComponentMeta` を実装するだけ |
| **カスタムリレーション種別** | `RelationKind::custom` + コールバック関数で任意の干渉を定義 |
| **コンポーネント生成テンプレートの差し替え** | `ComponentGenerator` にカスタムテンプレートを登録可能 |
| **外部プラグイン** | `plugins/` ディレクトリにコンポーネント群を配置して自動検出 |
| **仕様フォーマットの拡張** | `SpecParser` にカスタムセクションハンドラを追加可能 |

### 13.2 設計制約

| 制約 | 理由 |
|------|------|
| **継承禁止** | Ergoの基本原則。concept + variant で代替 |
| **循環依存禁止** | コンポーネント間の依存はDAGであること |
| **ドメイン名の一意性** | 同名ドメインの定義はエラー |
| **コンポーネントの単一責務** | 1コンポーネント = 1機能。複合的な振る舞いはドメインレベルで合成 |
| **リレーションの明示性** | 暗黙的なオブジェクト間干渉は禁止。すべてRelationとして宣言 |

### 13.3 Ergo既存設計との一貫性

| Ergo既存 | Ludus拡張での対応 |
|---------|----------------|
| `TaskLike` concept | 合成されたドメインが `TaskLike` を満たす |
| `GameObjectLike` concept | ドメインが `GameObjectLike` を満たす |
| `ColliderLike` concept | Colliderコンポーネントが `ColliderLike` を満たす |
| `ThreadingPolicy` | ドメイン内のコンポーネントから最も制約の強いポリシーを継承 |
| `std::variant` (非継承) | コンポーネント格納は `std::any` + `type_index` (既存GameObjectと同方式) |
| DLL境界 (C API) | `GameMonolith` が `ErgoGameCallbacks` を実装 |

---

> **本設計書は Ergo エンジンの Ludus 拡張に関するアーキテクチャ定義である。実装は本設計書に基づき、engine/ludus/ ディレクトリを起点として Phase 2 (feature/engine-core) の延長として進める。**
