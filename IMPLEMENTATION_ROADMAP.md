# Ergo エンジン — 実装ロードマップ詳細手順書

> **対象**: DESIGN.md に記載された設計を前提に、一般的なゲームエンジンとして必要な残タスクを網羅する
> **優先度**: Phase A (必須・動作に不可欠) → Phase B (実用レベル) → Phase C (プロダクション品質) → Phase D (拡張)

---

## 目次

- [Phase A: エンジン基盤の実動作](#phase-a-エンジン基盤の実動作)
  - [A1. ウィンドウシステム (GLFW統合)](#a1-ウィンドウシステム-glfw統合)
  - [A2. Vulkanレンダラー本体実装](#a2-vulkanレンダラー本体実装)
  - [A3. 2Dスプライト描画パイプライン](#a3-2dスプライト描画パイプライン)
  - [A4. テクスチャ読み込み](#a4-テクスチャ読み込み)
  - [A5. フォントレンダリング](#a5-フォントレンダリング)
  - [A6. Time管理](#a6-time管理)
  - [A7. ログシステム](#a7-ログシステム)
- [Phase B: ゲーム開発に必要な最低限の機能](#phase-b-ゲーム開発に必要な最低限の機能)
  - [B1. 入力システム完成](#b1-入力システム完成)
  - [B2. オーディオエンジン](#b2-オーディオエンジン)
  - [B3. カメラシステム](#b3-カメラシステム)
  - [B4. シーンマネージャー](#b4-シーンマネージャー)
  - [B5. リソースマネージャー](#b5-リソースマネージャー)
  - [B6. スプライトアニメーション](#b6-スプライトアニメーション)
  - [B7. Tween/イージング](#b7-tweenイージング)
  - [B8. 2D物理拡張](#b8-2d物理拡張)
- [Phase C: プロダクション品質](#phase-c-プロダクション品質)
  - [C1. UIフレームワーク](#c1-uiフレームワーク)
  - [C2. パーティクルシステム](#c2-パーティクルシステム)
  - [C3. シリアライゼーション](#c3-シリアライゼーション)
  - [C4. ライティング (軽量3D)](#c4-ライティング-軽量3d)
  - [C5. スケルタルアニメーション](#c5-スケルタルアニメーション)
  - [C6. ポストプロセス](#c6-ポストプロセス)
  - [C7. デバッグ描画・imgui統合](#c7-デバッグ描画imgui統合)
  - [C8. プロファイラー](#c8-プロファイラー)
- [Phase D: 拡張・運用](#phase-d-拡張運用)
  - [D1. ネットワーク基盤](#d1-ネットワーク基盤)
  - [D2. スクリプティング (Lua)](#d2-スクリプティング-lua)
  - [D3. ホットリロード](#d3-ホットリロード)
  - [D4. アセットパイプライン](#d4-アセットパイプライン)
  - [D5. Android/iOS対応](#d5-androidios対応)
  - [D6. ECS移行](#d6-ecs移行)
  - [D7. テスト・CI/CD](#d7-テストcicd)

---

## Phase A: エンジン基盤の実動作

**ゴール**: ウィンドウが開き、画面に矩形・円・テキストが描画され、キー入力で動くデモが動作する状態

---

### A1. ウィンドウシステム (GLFW統合)

**現状**: `DesktopWindow` がスタブ実装。ウィンドウは生成されない。

**成果物**:
- `system/window/desktop_window.cpp` — GLFW を使った実装
- CMake に GLFW 依存を追加

#### 手順

1. **GLFW の導入**
   ```cmake
   # system/CMakeLists.txt に追加
   find_package(glfw3 3.3 REQUIRED)
   # または FetchContent で取得
   include(FetchContent)
   FetchContent_Declare(glfw
       GIT_REPOSITORY https://github.com/glfw/glfw.git
       GIT_TAG 3.4
   )
   FetchContent_MakeAvailable(glfw)
   target_link_libraries(ergo_system PUBLIC glfw)
   ```

2. **DesktopWindow::Impl の実装**
   ```cpp
   // system/window/desktop_window.cpp
   #include <GLFW/glfw3.h>

   struct DesktopWindow::Impl {
       GLFWwindow* window = nullptr;
       uint32_t width = 0;
       uint32_t height = 0;
   };

   bool DesktopWindow::create(uint32_t w, uint32_t h, std::string_view title) {
       if (!glfwInit()) return false;
       glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Vulkan用
       glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
       impl_ = new Impl{};
       impl_->window = glfwCreateWindow(w, h, std::string(title).c_str(), nullptr, nullptr);
       if (!impl_->window) return false;
       impl_->width = w;
       impl_->height = h;
       // リサイズコールバック設定
       glfwSetWindowUserPointer(impl_->window, impl_);
       glfwSetFramebufferSizeCallback(impl_->window,
           [](GLFWwindow* win, int nw, int nh) {
               auto* p = static_cast<Impl*>(glfwGetWindowUserPointer(win));
               p->width = static_cast<uint32_t>(nw);
               p->height = static_cast<uint32_t>(nh);
           });
       return true;
   }

   bool DesktopWindow::should_close() const {
       return !impl_ || glfwWindowShouldClose(impl_->window);
   }

   void DesktopWindow::poll_events() {
       glfwPollEvents();
   }

   void* DesktopWindow::get_surface_handle() const {
       return impl_ ? impl_->window : nullptr;
   }
   ```

3. **GLFW入力連携の準備**
   - `GLFWwindow*` を `DesktopInput` から参照できるようにする
   - 方法: `DesktopInput::attach(GLFWwindow*)` メソッドを追加

4. **動作確認**: ウィンドウが表示され、×ボタンで閉じることを確認

#### ファイル変更一覧
| ファイル | 変更内容 |
|---------|---------|
| `system/CMakeLists.txt` | GLFW依存追加 |
| `system/window/desktop_window.hpp` | デストラクタ追加 |
| `system/window/desktop_window.cpp` | GLFW実装 |
| `system/input/desktop_input.hpp` | `attach(GLFWwindow*)` 追加 |
| `system/input/desktop_input.cpp` | GLFWキーコールバック実装 |

---

### A2. Vulkanレンダラー本体実装

**現状**: `VulkanRenderer` が完全スタブ。描画処理なし。

**成果物**: Vulkanの初期化〜画面クリアまでが動作する

#### 手順

1. **VkInstance 作成**
   ```cpp
   // system/renderer/vulkan/vk_renderer.cpp
   struct VulkanRenderer::Impl {
       VkInstance instance = VK_NULL_HANDLE;
       VkPhysicalDevice physical_device = VK_NULL_HANDLE;
       VkDevice device = VK_NULL_HANDLE;
       VkQueue graphics_queue = VK_NULL_HANDLE;
       VkQueue present_queue = VK_NULL_HANDLE;
       VkSurfaceKHR surface = VK_NULL_HANDLE;
       VkSwapchainKHR swapchain = VK_NULL_HANDLE;
       VkRenderPass render_pass = VK_NULL_HANDLE;
       VkCommandPool command_pool = VK_NULL_HANDLE;
       std::vector<VkCommandBuffer> command_buffers;
       std::vector<VkImageView> swapchain_image_views;
       std::vector<VkFramebuffer> framebuffers;
       VkSemaphore image_available = VK_NULL_HANDLE;
       VkSemaphore render_finished = VK_NULL_HANDLE;
       VkFence in_flight_fence = VK_NULL_HANDLE;
       uint32_t image_index = 0;
       GLFWwindow* window = nullptr;
       // RenderContext実装
       // ...
   };
   ```

2. **initialize() の実装順序**
   ```
   ① vkCreateInstance (VK_KHR_surface + プラットフォーム拡張)
   ② glfwCreateWindowSurface で VkSurfaceKHR 取得
   ③ 物理デバイス選択 (vkEnumeratePhysicalDevices)
   ④ キューファミリー検索 (Graphics + Present)
   ⑤ 論理デバイス作成 (vkCreateDevice)
   ⑥ スワップチェーン作成 (vkCreateSwapchainKHR)
   ⑦ イメージビュー作成
   ⑧ レンダーパス作成 (1パス、カラーアタッチメントのみ)
   ⑨ フレームバッファ作成
   ⑩ コマンドプール + コマンドバッファ
   ⑪ 同期オブジェクト (セマフォ + フェンス)
   ```

3. **begin_frame() / end_frame()**
   ```
   begin_frame:
     vkWaitForFences → vkResetFences
     vkAcquireNextImageKHR
     vkResetCommandBuffer
     vkBeginCommandBuffer
     vkCmdBeginRenderPass (clearColor: 黒)

   end_frame:
     vkCmdEndRenderPass
     vkEndCommandBuffer
     vkQueueSubmit
     vkQueuePresentKHR
   ```

4. **スワップチェーン再生成**
   - ウィンドウリサイズ時に `vkCreateSwapchainKHR` を再呼び出し
   - `VK_ERROR_OUT_OF_DATE_KHR` のハンドリング

5. **shutdown()** — 全Vulkanリソースの逆順破棄

6. **動作確認**: 黒い画面が表示されれば成功

#### ファイル変更一覧
| ファイル | 変更内容 |
|---------|---------|
| `system/renderer/vulkan/vk_renderer.hpp` | `set_window()` 追加 |
| `system/renderer/vulkan/vk_renderer.cpp` | Vulkan初期化の全実装 |
| `system/renderer/vulkan/vk_swapchain.hpp` | Vulkan型に変更 |
| `system/renderer/vulkan/vk_swapchain.cpp` | スワップチェーン実装 |
| `system/CMakeLists.txt` | `Vulkan::Vulkan` リンク必須化 |
| `runtime/main.cpp` | `renderer.set_window(window)` 呼び出し追加 |

#### 参考リソース
- Vulkan Tutorial (https://vulkan-tutorial.com) の Drawing a triangle まで

---

### A3. 2Dスプライト描画パイプライン

**現状**: `RenderContext` のメソッドが全て空実装

**成果物**: `draw_rect`, `draw_circle`, `draw_text` が画面に描画される

#### 手順

1. **シェーダー作成**

   ```
   engine/shaders/
   ├── shape2d.vert      # 2D形状用頂点シェーダー
   ├── shape2d.frag      # 2D形状用フラグメントシェーダー
   ├── sprite.vert       # スプライト用頂点シェーダー
   ├── sprite.frag       # スプライト用フラグメントシェーダー
   └── compile.sh        # glslc でSPIR-Vにコンパイル
   ```

   **shape2d.vert**:
   ```glsl
   #version 450
   layout(push_constant) uniform PushConstants {
       mat4 projection;
   } pc;
   layout(location = 0) in vec2 in_pos;
   layout(location = 1) in vec4 in_color;
   layout(location = 0) out vec4 frag_color;
   void main() {
       gl_Position = pc.projection * vec4(in_pos, 0.0, 1.0);
       frag_color = in_color;
   }
   ```

   **shape2d.frag**:
   ```glsl
   #version 450
   layout(location = 0) in vec4 frag_color;
   layout(location = 0) out vec4 out_color;
   void main() {
       out_color = frag_color;
   }
   ```

2. **2D頂点バッファの動的バッチ**

   ```cpp
   // system/renderer/vulkan/vk_batch2d.hpp — 新規ファイル
   struct Vertex2D {
       float x, y;          // 位置
       float u, v;           // UV
       uint8_t r, g, b, a;   // 色
   };

   class VkBatch2D {
       static constexpr size_t MAX_VERTICES = 65536;
       static constexpr size_t MAX_INDICES = MAX_VERTICES * 6 / 4;

       VkBuffer vertex_buffer_ = VK_NULL_HANDLE;
       VkDeviceMemory vertex_memory_ = VK_NULL_HANDLE;
       VkBuffer index_buffer_ = VK_NULL_HANDLE;
       VkDeviceMemory index_memory_ = VK_NULL_HANDLE;

       Vertex2D* mapped_vertices_ = nullptr;
       uint32_t vertex_count_ = 0;
       uint32_t index_count_ = 0;

       TextureHandle current_texture_;

   public:
       void create(VkDevice device, VkPhysicalDevice phys_device);
       void destroy(VkDevice device);

       void begin();    // フレーム開始時にカウンタリセット
       void flush(VkCommandBuffer cmd, VkPipeline pipeline, VkPipelineLayout layout);

       // 描画プリミティブ (頂点を積む)
       void push_rect(Vec2f pos, Size2f size, Color color, bool filled);
       void push_circle(Vec2f center, float radius, Color color, bool filled, int segments = 32);
       void push_sprite(Vec2f pos, Size2f size, TextureHandle tex, Rect uv, Color tint);
   };
   ```

3. **VkPipeline 作成**
   - `vk_pipeline.cpp` に shape2d パイプラインを作成
   - 頂点入力: `Vertex2D` のバインディング/アトリビュート記述
   - プッシュ定数: 射影行列 (正射影)
   - ブレンド: アルファブレンド有効
   - トポロジ: `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST`

4. **RenderContext 実装クラス**
   ```cpp
   struct VulkanRenderContext final : RenderContext {
       VkBatch2D* batch = nullptr;
       Mat4 projection;

       void draw_rect(Vec2f pos, Size2f size, Color color, bool filled) override {
           batch->push_rect(pos, size, color, filled);
       }
       void draw_circle(Vec2f center, float radius, Color color, bool filled) override {
           batch->push_circle(center, radius, color, filled);
       }
       // ...
   };
   ```

5. **フレームループへの組み込み**
   ```
   begin_frame:
     batch.begin()
   (各タスクの draw が batch に頂点を積む)
   end_frame:
     batch.flush(cmd, pipeline, layout)  // 一括描画
     vkCmdEndRenderPass → submit → present
   ```

6. **動作確認**: `draw_rect({100,100}, {50,80}, {255,0,0,255}, true)` で赤い矩形が表示される

#### 新規ファイル
| ファイル | 内容 |
|---------|------|
| `system/renderer/vulkan/vk_batch2d.hpp` | バッチ描画ヘッダ |
| `system/renderer/vulkan/vk_batch2d.cpp` | バッチ描画実装 |
| `engine/shaders/shape2d.vert` | 頂点シェーダー |
| `engine/shaders/shape2d.frag` | フラグメントシェーダー |
| `engine/shaders/sprite.vert` | スプライト頂点シェーダー |
| `engine/shaders/sprite.frag` | スプライトフラグメントシェーダー |
| `engine/shaders/compile.sh` | SPIR-Vコンパイルスクリプト |

---

### A4. テクスチャ読み込み

**現状**: `load_texture` が `{0}` を返すスタブ

**成果物**: PNG/JPGファイルをVkImageに読み込み、スプライト描画で使用できる

#### 手順

1. **stb_image の導入**
   ```
   engine/third_party/
   └── stb_image.h    # https://github.com/nothings/stb から取得
   ```

   ```cpp
   // engine/resource/image_loader.cpp (新規)
   #define STB_IMAGE_IMPLEMENTATION
   #include "third_party/stb_image.h"
   ```

2. **ImageData 構造体**
   ```cpp
   // engine/resource/image_loader.hpp (新規)
   #pragma once
   #include <cstdint>
   #include <vector>
   #include <string_view>

   struct ImageData {
       uint32_t width = 0;
       uint32_t height = 0;
       uint32_t channels = 4;
       std::vector<uint8_t> pixels;
       bool valid() const { return !pixels.empty(); }
   };

   ImageData load_image(std::string_view path);
   ImageData load_image_from_memory(const uint8_t* data, size_t size);
   ```

3. **VkImage / VkImageView / VkSampler 作成**
   ```cpp
   // system/renderer/vulkan/vk_texture.hpp (新規)
   struct VkTextureEntry {
       VkImage image = VK_NULL_HANDLE;
       VkDeviceMemory memory = VK_NULL_HANDLE;
       VkImageView view = VK_NULL_HANDLE;
       uint32_t width = 0;
       uint32_t height = 0;
   };

   class VkTextureManager {
       VkDevice device_;
       VkPhysicalDevice phys_device_;
       VkCommandPool cmd_pool_;
       VkQueue queue_;
       VkSampler sampler_ = VK_NULL_HANDLE;
       std::unordered_map<uint64_t, VkTextureEntry> textures_;
       uint64_t next_id_ = 1;

   public:
       void create(VkDevice dev, VkPhysicalDevice phys, VkCommandPool pool, VkQueue q);
       void destroy();
       TextureHandle load(std::string_view path);
       TextureHandle load_from_memory(const uint8_t* pixels, uint32_t w, uint32_t h);
       void unload(TextureHandle handle);
       VkDescriptorImageInfo get_descriptor(TextureHandle handle) const;
   };
   ```

4. **ステージングバッファ → VkImage 転送**
   ```
   ① VkBuffer (TRANSFER_SRC) にピクセルデータコピー
   ② VkImage 作成 (VK_FORMAT_R8G8B8A8_SRGB)
   ③ vkCmdPipelineBarrier (UNDEFINED → TRANSFER_DST)
   ④ vkCmdCopyBufferToImage
   ⑤ vkCmdPipelineBarrier (TRANSFER_DST → SHADER_READ_ONLY)
   ⑥ ステージングバッファ解放
   ```

5. **ディスクリプタセットの更新**
   - スプライトパイプラインに `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` バインディング追加
   - テクスチャ切り替え時にディスクリプタセット更新 or バッチフラッシュ

6. **1x1 白テクスチャ** — 色付き矩形/円の描画用デフォルトテクスチャ

#### 新規ファイル
| ファイル | 内容 |
|---------|------|
| `engine/third_party/stb_image.h` | stb_image ヘッダ |
| `engine/resource/image_loader.hpp` | 画像読み込みAPI |
| `engine/resource/image_loader.cpp` | stb_image実装 |
| `system/renderer/vulkan/vk_texture.hpp` | テクスチャ管理ヘッダ |
| `system/renderer/vulkan/vk_texture.cpp` | VkImage生成・転送 |

---

### A5. フォントレンダリング

**現状**: `draw_text` がスタブ

**成果物**: 画面にASCII + 日本語テキストを描画できる

#### 手順

1. **方式選択**: BMFont (ビットマップフォント) 方式を採用
   - stb_truetype でTTFからビットマップアトラスを生成
   - 最終的にはSDF方式への移行も可能

2. **stb_truetype の導入**
   ```
   engine/third_party/
   └── stb_truetype.h
   ```

3. **FontAtlas 構造体**
   ```cpp
   // engine/resource/font.hpp (新規)
   #pragma once
   #include <string_view>
   #include <unordered_map>
   #include <cstdint>
   #include "../math/vec2.hpp"
   #include "texture_handle.hpp"

   struct GlyphInfo {
       float u0, v0, u1, v1;    // テクスチャ座標
       float width, height;      // ピクセルサイズ
       float bearing_x, bearing_y;
       float advance;
   };

   struct FontAtlas {
       TextureHandle texture;
       float font_size = 0.0f;
       float line_height = 0.0f;
       std::unordered_map<uint32_t, GlyphInfo> glyphs;  // codepoint → glyph

       const GlyphInfo* get_glyph(uint32_t codepoint) const;
   };
   ```

4. **FontLoader**
   ```cpp
   // engine/resource/font_loader.hpp (新規)
   class FontLoader {
   public:
       // TTFファイルからフォントアトラスを生成
       // char_ranges: 生成する文字コード範囲のリスト
       static FontAtlas load(std::string_view ttf_path, float font_size,
                             const std::vector<std::pair<uint32_t, uint32_t>>& char_ranges);
   };
   ```

   **文字範囲の例**:
   ```cpp
   std::vector<std::pair<uint32_t, uint32_t>> ranges = {
       {0x0020, 0x007E},  // ASCII
       {0x3000, 0x303F},  // CJK記号
       {0x3040, 0x309F},  // ひらがな
       {0x30A0, 0x30FF},  // カタカナ
       {0x4E00, 0x9FFF},  // CJK統合漢字 (全部は巨大なので頻出のみ)
   };
   ```

5. **draw_text の実装**
   - UTF-8文字列をコードポイントに変換
   - 各グリフに対してスプライトとして `VkBatch2D::push_sprite` を呼ぶ
   - カーニングは初期実装では省略

6. **動作確認**: `draw_text({10, 10}, "Hello World スコア: 100", white, 1.0f)` が描画される

#### 新規ファイル
| ファイル | 内容 |
|---------|------|
| `engine/third_party/stb_truetype.h` | stb_truetype ヘッダ |
| `engine/resource/font.hpp` | フォントアトラス定義 |
| `engine/resource/font_loader.hpp` | フォント読み込みAPI |
| `engine/resource/font_loader.cpp` | TTF→アトラス変換 |
| `engine/resource/utf8.hpp` | UTF-8デコードユーティリティ |

---

### A6. Time管理

**現状**: `main.cpp` で `chrono` を直接使用。`dt` 以外の情報なし。

**成果物**: deltaTime, totalTime, frameCount, timeScale, fixedDeltaTime を提供

#### 手順

1. **Time 構造体**
   ```cpp
   // engine/core/time.hpp (新規)
   #pragma once
   #include <cstdint>

   struct Time {
       float delta_time = 0.0f;          // 前フレームからの経過秒
       float unscaled_delta_time = 0.0f; // timeScale適用前
       float total_time = 0.0f;          // 起動からの累計秒
       float time_scale = 1.0f;          // スロー/ファスト
       float fixed_delta_time = 1.0f / 60.0f;
       uint64_t frame_count = 0;
       float fps = 0.0f;                 // 直近のFPS (平滑化)

       void tick(float raw_dt) {
           unscaled_delta_time = raw_dt;
           delta_time = raw_dt * time_scale;
           total_time += delta_time;
           ++frame_count;
           // FPS計算 (指数移動平均)
           float alpha = 0.1f;
           float instant_fps = (raw_dt > 0.0f) ? 1.0f / raw_dt : 0.0f;
           fps = fps * (1.0f - alpha) + instant_fps * alpha;
       }
   };

   // グローバルアクセス
   inline Time g_time;
   ```

2. **FPSキャップ**
   ```cpp
   // engine/core/time.hpp に追加
   struct FrameRateLimiter {
       float target_fps = 60.0f;
       // ...
       void wait(); // std::this_thread::sleep_until で待機
   };
   ```

3. **main.cpp への組み込み**
   ```cpp
   g_time.tick(dt);
   // 各システムは g_time.delta_time を使用
   ```

---

### A7. ログシステム

**現状**: `fprintf(stderr, ...)` を直接使用

**成果物**: レベル別ログ、カテゴリフィルタ、ファイル出力

#### 手順

1. **Log API**
   ```cpp
   // engine/core/log.hpp (新規)
   #pragma once
   #include <cstdio>
   #include <cstdarg>
   #include <string_view>

   enum class LogLevel : uint8_t {
       Trace, Debug, Info, Warn, Error, Fatal
   };

   namespace ergo::log {
       void set_level(LogLevel min_level);
       void set_file(const char* path);

       void trace(const char* category, const char* fmt, ...);
       void debug(const char* category, const char* fmt, ...);
       void info(const char* category, const char* fmt, ...);
       void warn(const char* category, const char* fmt, ...);
       void error(const char* category, const char* fmt, ...);
       void fatal(const char* category, const char* fmt, ...);
   }

   // 便利マクロ
   #define ERGO_LOG_INFO(cat, ...) ergo::log::info(cat, __VA_ARGS__)
   #define ERGO_LOG_WARN(cat, ...) ergo::log::warn(cat, __VA_ARGS__)
   #define ERGO_LOG_ERROR(cat, ...) ergo::log::error(cat, __VA_ARGS__)
   ```

2. **実装** — `vsnprintf` + タイムスタンプ + カテゴリ + レベル色分け (ターミナル)

3. **アサーション**
   ```cpp
   // engine/core/assert.hpp (新規)
   #define ERGO_ASSERT(cond, msg) \
       do { if (!(cond)) { \
           ergo::log::fatal("ASSERT", "%s:%d: %s", __FILE__, __LINE__, msg); \
           std::abort(); \
       } } while(0)
   ```

---

## Phase B: ゲーム開発に必要な最低限の機能

**ゴール**: サンプルシューティングゲームが完全に動作する

---

### B1. 入力システム完成

**現状**: GLFWコールバック未接続、ゲームパッド非対応

#### 手順

1. **GLFWキーコールバック接続**
   ```cpp
   void DesktopInput::attach(GLFWwindow* window) {
       impl_->window = window;
       glfwSetWindowUserPointer(window, impl_);
       glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
           auto* p = static_cast<Impl*>(glfwGetWindowUserPointer(w));
           if (key >= 0 && key < MAX_KEYS) {
               p->key_current[key] = (action != GLFW_RELEASE);
           }
       });
       glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
           auto* p = static_cast<Impl*>(glfwGetWindowUserPointer(w));
           p->mouse_pos = {static_cast<float>(x), static_cast<float>(y)};
       });
       glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int) {
           auto* p = static_cast<Impl*>(glfwGetWindowUserPointer(w));
           if (button < 8) p->mouse_buttons[button] = (action != GLFW_RELEASE);
       });
   }
   ```

2. **ゲームパッド対応**
   ```cpp
   // system/input/gamepad_input.hpp (新規)
   struct GamepadState {
       float left_stick_x = 0.0f;
       float left_stick_y = 0.0f;
       float right_stick_x = 0.0f;
       float right_stick_y = 0.0f;
       float left_trigger = 0.0f;
       float right_trigger = 0.0f;
       bool buttons[16] = {};
   };

   class GamepadInput {
   public:
       void poll();
       bool is_connected(int index) const;
       GamepadState state(int index) const;
   };
   ```

3. **入力マッピング**
   ```cpp
   // engine/core/input_map.hpp (新規)
   // アクション名 → 入力バインドの設定
   struct InputAction {
       std::string name;                   // "Jump", "Fire", etc.
       std::vector<uint32_t> keys;         // KEY_SPACE, etc.
       std::vector<uint32_t> gamepad_buttons;
       int gamepad_axis = -1;              // アナログ入力
       float dead_zone = 0.15f;
   };

   class InputMap {
       std::unordered_map<std::string, InputAction> actions_;
   public:
       void register_action(InputAction action);
       bool is_action_down(std::string_view name) const;
       bool is_action_pressed(std::string_view name) const;
       float get_axis(std::string_view name) const;
       void load_from_file(std::string_view path);  // JSON設定
   };
   ```

#### 新規ファイル
| ファイル | 内容 |
|---------|------|
| `system/input/gamepad_input.hpp` | ゲームパッドAPI |
| `system/input/gamepad_input.cpp` | GLFW Joystick実装 |
| `engine/core/input_map.hpp` | 入力マッピング定義 |
| `engine/core/input_map.cpp` | 入力マッピング実装 |

---

### B2. オーディオエンジン

**現状**: 未実装

**成果物**: BGM/SE再生ができる

#### 手順

1. **バックエンド選択**: miniaudio (ヘッダオンリー、クロスプラットフォーム) を採用
   ```
   engine/third_party/
   └── miniaudio.h    # https://miniaud.io/
   ```

2. **AudioEngine インターフェース**
   ```cpp
   // system/audio/audio_engine.hpp (新規)
   #pragma once
   #include <string_view>
   #include <cstdint>

   struct SoundHandle {
       uint64_t id = 0;
       bool valid() const { return id != 0; }
   };

   class AudioEngine {
   public:
       bool initialize();
       void shutdown();

       // BGM
       SoundHandle load_music(std::string_view path);
       void play_music(SoundHandle handle, bool loop = true);
       void stop_music();
       void set_music_volume(float volume);  // 0.0 ~ 1.0

       // SE
       SoundHandle load_sound(std::string_view path);
       void play_sound(SoundHandle handle, float volume = 1.0f, float pitch = 1.0f);

       // 全体
       void set_master_volume(float volume);
       void update();  // 毎フレーム呼び出し (ストリーミング更新等)
   };
   ```

3. **miniaudio実装**
   ```cpp
   // system/audio/audio_engine.cpp
   #define MINIAUDIO_IMPLEMENTATION
   #include "miniaudio.h"

   struct AudioEngine::Impl {
       ma_engine engine;
       bool initialized = false;
   };

   bool AudioEngine::initialize() {
       impl_ = new Impl{};
       ma_engine_config config = ma_engine_config_init();
       if (ma_engine_init(&config, &impl_->engine) != MA_SUCCESS) return false;
       impl_->initialized = true;
       return true;
   }
   // ...
   ```

4. **対応フォーマット**: WAV, MP3, OGG (miniaudioが標準対応)

5. **main.cpp への組み込み**
   ```cpp
   AudioEngine audio;
   audio.initialize();
   // ループ内:
   audio.update();
   // 終了時:
   audio.shutdown();
   ```

#### 新規ファイル
| ファイル | 内容 |
|---------|------|
| `engine/third_party/miniaudio.h` | miniaudioヘッダ |
| `system/audio/audio_engine.hpp` | オーディオAPI |
| `system/audio/audio_engine.cpp` | miniaudio実装 |

---

### B3. カメラシステム

**現状**: 射影行列はVulkan初期化時に固定設定のみ

**成果物**: 2Dカメラ (スクロール/ズーム) + 3Dカメラ (FPS/追従)

#### 手順

1. **Camera2D**
   ```cpp
   // engine/core/camera2d.hpp (新規)
   #pragma once
   #include "../math/vec2.hpp"
   #include "../math/mat4.hpp"

   struct Camera2D {
       Vec2f position;
       float zoom = 1.0f;
       float rotation = 0.0f;
       float viewport_width = 800.0f;
       float viewport_height = 600.0f;

       // 正射影行列 (Vulkan座標系: Y下向き)
       Mat4 view_projection() const;

       // ワールド座標 ↔ スクリーン座標 変換
       Vec2f world_to_screen(Vec2f world) const;
       Vec2f screen_to_world(Vec2f screen) const;

       // カメラシェイク
       void shake(float intensity, float duration);
       void update_shake(float dt);
   };
   ```

2. **Camera3D**
   ```cpp
   // engine/core/camera3d.hpp (新規)
   struct Camera3D {
       Vec3f position;
       Vec3f target;
       Vec3f up{0.0f, 1.0f, 0.0f};
       float fov = 60.0f;    // degree
       float near_z = 0.1f;
       float far_z = 1000.0f;
       float aspect = 16.0f / 9.0f;

       Mat4 view_matrix() const;
       Mat4 projection_matrix() const;
       Mat4 view_projection() const;
   };
   ```

3. **レンダラーとの連携**
   - `begin_frame` 時にカメラの `view_projection` をプッシュ定数に設定

---

### B4. シーンマネージャー

**現状**: `StateMachine` でシーン遷移するが、複数シーンの並行管理やトランジションがない

#### 手順

1. **SceneManager**
   ```cpp
   // engine/core/scene_manager.hpp (新規)
   #pragma once
   #include <vector>
   #include <memory>
   #include <string_view>
   #include <functional>

   struct Scene {
       virtual ~Scene() = default;
       virtual void on_enter() = 0;
       virtual void on_exit() = 0;
       virtual void on_update(float dt) = 0;
       virtual void on_draw(RenderContext& ctx) = 0;
       virtual void on_pause() {}    // 上にシーンが重なった時
       virtual void on_resume() {}   // 上のシーンが外れた時
   };

   class SceneManager {
       // スタック型 (ポーズ画面等の重ね置き対応)
       std::vector<std::unique_ptr<Scene>> stack_;

       // トランジション
       enum class TransitionState { None, FadeOut, FadeIn };
       TransitionState trans_state_ = TransitionState::None;
       float trans_timer_ = 0.0f;
       float trans_duration_ = 0.5f;
       std::function<void()> trans_action_;

   public:
       // 現在のシーンを置き換え
       void change(std::unique_ptr<Scene> scene, float fade_duration = 0.0f);

       // シーンをスタックに積む (ポーズ等)
       void push(std::unique_ptr<Scene> scene);

       // スタックトップを除去
       void pop();

       void update(float dt);
       void draw(RenderContext& ctx);
   };
   ```

2. **NOTE**: `Scene` は virtual を使用するが、これはDLL境界と同様の理由による例外的使用。
   ゲーム側でシーンの具体型をDLLから提供するためtype-erasureが必要。

---

### B5. リソースマネージャー

**現状**: テクスチャのload/unloadが散在。ライフサイクル管理なし。

#### 手順

1. **ResourceManager**
   ```cpp
   // engine/resource/resource_manager.hpp (新規)
   #pragma once
   #include "texture_handle.hpp"
   #include "font.hpp"
   #include <string>
   #include <unordered_map>
   #include <memory>

   class ResourceManager {
       struct TextureEntry {
           TextureHandle handle;
           uint32_t ref_count = 0;
           std::string path;
       };
       struct FontEntry {
           FontAtlas atlas;
           uint32_t ref_count = 0;
       };

       std::unordered_map<std::string, TextureEntry> textures_;
       std::unordered_map<std::string, FontEntry> fonts_;

   public:
       // 同じパスの二重ロードを防止 (参照カウント)
       TextureHandle load_texture(std::string_view path);
       void release_texture(TextureHandle handle);

       FontAtlas* load_font(std::string_view ttf_path, float size);
       void release_font(std::string_view key);

       // 参照カウント0のリソースを解放
       void collect_garbage();

       // 全リソース解放
       void shutdown();
   };

   inline ResourceManager g_resources;
   ```

2. **非同期ロード (後で拡張)**
   ```cpp
   // 将来的に追加
   std::future<TextureHandle> load_texture_async(std::string_view path);
   ```

---

### B6. スプライトアニメーション

**現状**: 未実装

#### 手順

1. **SpriteAnimation**
   ```cpp
   // engine/core/sprite_animation.hpp (新規)
   #pragma once
   #include "../resource/texture_handle.hpp"
   #include "../math/vec2.hpp"
   #include <vector>

   struct AnimationFrame {
       Rect uv;           // スプライトシート内のUV座標
       float duration;    // このフレームの表示秒数
   };

   struct SpriteAnimation {
       TextureHandle texture;
       std::vector<AnimationFrame> frames;
       bool loop = true;

       // 再生状態
       float timer = 0.0f;
       uint32_t current_frame = 0;
       bool finished = false;

       void update(float dt) {
           if (finished) return;
           timer += dt;
           while (timer >= frames[current_frame].duration) {
               timer -= frames[current_frame].duration;
               ++current_frame;
               if (current_frame >= frames.size()) {
                   if (loop) current_frame = 0;
                   else { current_frame = frames.size() - 1; finished = true; }
               }
           }
       }

       const Rect& current_uv() const {
           return frames[current_frame].uv;
       }

       void reset() { timer = 0.0f; current_frame = 0; finished = false; }
   };
   ```

2. **AnimationController** — 複数アニメーションの切り替え管理
   ```cpp
   struct AnimationController {
       std::unordered_map<std::string, SpriteAnimation> animations;
       std::string current_name;

       void play(std::string_view name);
       void update(float dt);
       const SpriteAnimation* current() const;
   };
   ```

---

### B7. Tween/イージング

**現状**: 未実装

#### 手順

1. **イージング関数**
   ```cpp
   // engine/core/easing.hpp (新規)
   #pragma once
   #include <cmath>

   namespace easing {
       inline float linear(float t) { return t; }
       inline float in_quad(float t) { return t * t; }
       inline float out_quad(float t) { return t * (2.0f - t); }
       inline float in_out_quad(float t) {
           return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
       }
       inline float in_cubic(float t) { return t * t * t; }
       inline float out_cubic(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
       inline float in_out_cubic(float t) {
           return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
       }
       inline float in_sine(float t) { return 1.0f - std::cos(t * 3.14159265f * 0.5f); }
       inline float out_sine(float t) { return std::sin(t * 3.14159265f * 0.5f); }
       inline float out_bounce(float t) {
           if (t < 1.0f/2.75f) return 7.5625f*t*t;
           if (t < 2.0f/2.75f) { t -= 1.5f/2.75f; return 7.5625f*t*t + 0.75f; }
           if (t < 2.5f/2.75f) { t -= 2.25f/2.75f; return 7.5625f*t*t + 0.9375f; }
           t -= 2.625f/2.75f; return 7.5625f*t*t + 0.984375f;
       }
       // ... 他のイージング関数
   }
   ```

2. **Tweenシステム**
   ```cpp
   // engine/core/tween.hpp (新規)
   using EasingFunc = float(*)(float);

   struct Tween {
       float* target = nullptr;       // 変更対象の float ポインタ
       float start_value = 0.0f;
       float end_value = 0.0f;
       float duration = 0.0f;
       float elapsed = 0.0f;
       EasingFunc easing = easing::linear;
       std::function<void()> on_complete;
       bool finished = false;

       void update(float dt);
   };

   class TweenManager {
       std::vector<Tween> tweens_;
   public:
       Tween& add(float* target, float from, float to, float duration,
                   EasingFunc ease = easing::linear);
       void update(float dt);
       void clear();
   };

   inline TweenManager g_tweens;
   ```

---

### B8. 2D物理拡張

**現状**: AABB/Circle衝突のみ。ブロードフェーズなし。

#### 手順

1. **レイキャスト**
   ```cpp
   // engine/physics/raycast2d.hpp (新規)
   #pragma once
   #include "../math/vec2.hpp"
   #include "collider.hpp"
   #include <optional>

   struct RayHit2D {
       Vec2f point;
       Vec2f normal;
       float distance;
       const Collider* collider;
   };

   std::optional<RayHit2D> raycast2d(
       Vec2f origin, Vec2f direction, float max_distance,
       ColliderTag mask = ColliderTag::Invalid);  // Invalid = 全タグ

   std::vector<RayHit2D> raycast2d_all(
       Vec2f origin, Vec2f direction, float max_distance,
       ColliderTag mask = ColliderTag::Invalid);
   ```

2. **トリガーコライダー**
   ```cpp
   // Collider にフラグ追加
   struct Collider {
       // ... 既存フィールド
       bool is_trigger = false;  // true: 衝突検出のみ、物理応答なし
   };
   ```

3. **空間分割 (グリッド)**
   ```cpp
   // engine/physics/spatial_grid.hpp (新規)
   class SpatialGrid2D {
       float cell_size_;
       std::unordered_map<uint64_t, std::vector<Collider*>> cells_;

       uint64_t hash(int cx, int cy) const;
   public:
       SpatialGrid2D(float cell_size = 64.0f);
       void clear();
       void insert(Collider* c);
       std::vector<Collider*> query(Vec2f min, Vec2f max) const;
       std::vector<Collider*> query_radius(Vec2f center, float radius) const;
   };
   ```

4. **PhysicsSystem への統合**
   - `run()` 内でグリッドベースのブロードフェーズを使用
   - ペア生成後にナローフェーズ (`check_hit`) を実行

---

## Phase C: プロダクション品質

**ゴール**: 商用レベルのゲームを開発できる品質

---

### C1. UIフレームワーク

#### 手順

1. **UIElement 基底**
   ```cpp
   // engine/ui/ui_element.hpp (新規)
   enum class Anchor { TopLeft, Top, TopRight, Left, Center, Right,
                       BottomLeft, Bottom, BottomRight };

   struct UIElement {
       uint64_t id = 0;
       Vec2f position;
       Size2f size;
       Anchor anchor = Anchor::TopLeft;
       Vec2f margin;
       bool visible = true;
       bool interactive = true;

       // 親子関係
       UIElement* parent = nullptr;
       std::vector<UIElement*> children;

       // イベントコールバック
       std::function<void()> on_click;
       std::function<void()> on_hover_enter;
       std::function<void()> on_hover_exit;

       // 計算済みスクリーン座標
       Vec2f computed_position() const;

       virtual void update(float dt) {}
       virtual void draw(RenderContext& ctx) {}
   };
   ```

2. **具体UIウィジェット**

   | ウィジェット | ファイル | 機能 |
   |-------------|---------|------|
   | Label | `ui_label.hpp` | テキスト表示 |
   | Button | `ui_button.hpp` | クリック可能ボタン |
   | Image | `ui_image.hpp` | 画像表示 |
   | Slider | `ui_slider.hpp` | 数値スライダー |
   | TextInput | `ui_text_input.hpp` | テキスト入力 |
   | Panel | `ui_panel.hpp` | 背景パネル + レイアウト |
   | ProgressBar | `ui_progress_bar.hpp` | HP/ロードバー |

3. **UIManager**
   ```cpp
   class UIManager {
       std::vector<std::unique_ptr<UIElement>> roots_;
       UIElement* focused_ = nullptr;
       UIElement* hovered_ = nullptr;
   public:
       void add_root(std::unique_ptr<UIElement> elem);
       void update(float dt, Vec2f mouse_pos, bool mouse_down, bool mouse_clicked);
       void draw(RenderContext& ctx);
       UIElement* hit_test(Vec2f pos);
   };
   ```

---

### C2. パーティクルシステム

#### 手順

1. **Particle / ParticleEmitter**
   ```cpp
   // engine/render/particle_system.hpp (新規)
   struct Particle {
       Vec2f position;
       Vec2f velocity;
       Color color;
       float life = 0.0f;
       float max_life = 1.0f;
       float size = 1.0f;
       float rotation = 0.0f;
   };

   struct EmitterConfig {
       Vec2f position;
       float emit_rate = 10.0f;            // 毎秒放出数
       float particle_life_min = 0.5f;
       float particle_life_max = 1.5f;
       Vec2f velocity_min{-50.0f, -50.0f};
       Vec2f velocity_max{50.0f, 50.0f};
       Color color_start{255, 255, 255, 255};
       Color color_end{255, 255, 255, 0};
       float size_start = 8.0f;
       float size_end = 2.0f;
       Vec2f gravity;
       TextureHandle texture;              // nullならデフォルト円
       uint32_t max_particles = 1000;
       bool loop = true;
   };

   class ParticleEmitter {
       EmitterConfig config_;
       std::vector<Particle> particles_;
       float emit_accumulator_ = 0.0f;
       bool active_ = true;
   public:
       ParticleEmitter(EmitterConfig config);
       void update(float dt);
       void draw(RenderContext& ctx);  // or VkBatch2D
       void start();
       void stop();
       void burst(uint32_t count);
       bool is_alive() const;
   };
   ```

2. **ParticleManager** — 全エミッターの一括管理・更新・描画

---

### C3. シリアライゼーション

#### 手順

1. **JSON 読み書き** — nlohmann/json またはRapidJSON を導入
   ```
   engine/third_party/
   └── nlohmann/json.hpp
   ```

2. **シリアライズ対象**

   | 対象 | フォーマット | 用途 |
   |------|------------|------|
   | シーンデータ | `.scene.json` | エンティティ配置、プロパティ |
   | 入力マッピング | `.input.json` | キーバインド設定 |
   | パーティクル設定 | `.particle.json` | エミッター設定 |
   | アニメーション定義 | `.anim.json` | フレーム定義 |
   | ゲーム設定 | `config.json` | 解像度、音量等 |

3. **Serialize/Deserialize マクロ**
   ```cpp
   // engine/core/serialization.hpp (新規)
   // 簡易的なJSON変換ヘルパー
   template<typename T>
   nlohmann::json serialize(const T& obj);

   template<typename T>
   T deserialize(const nlohmann::json& j);

   // 特殊化例
   template<> nlohmann::json serialize(const Vec2f& v) {
       return {{"x", v.x}, {"y", v.y}};
   }
   template<> Vec2f deserialize(const nlohmann::json& j) {
       return {j["x"].get<float>(), j["y"].get<float>()};
   }
   ```

---

### C4. ライティング (軽量3D)

#### 手順

1. **Light 構造体**
   ```cpp
   // engine/render/light.hpp (新規)
   enum class LightType : uint8_t { Directional, Point, Spot };

   struct Light {
       LightType type = LightType::Directional;
       Vec3f position;
       Vec3f direction{0.0f, -1.0f, 0.0f};
       Color color{255, 255, 255, 255};
       float intensity = 1.0f;
       float range = 10.0f;           // Point/Spot用
       float spot_angle = 45.0f;      // Spot用 (度)
       float spot_softness = 0.5f;
   };
   ```

2. **LightManager** — 最大8〜16ライトをUBOで管理

3. **ライティングシェーダー**
   - Blinn-Phong (軽量3D向け)
   - フラグメントシェーダーでライト配列をループ

4. **シャドウマップ** (オプション)
   - ディレクショナルライト用のシンプルなシャドウマップ
   - 解像度1024〜2048で十分

---

### C5. スケルタルアニメーション

#### 手順

1. **Skeleton / Bone**
   ```cpp
   // engine/animation/skeleton.hpp (新規)
   struct Bone {
       std::string name;
       int32_t parent_index = -1;
       Mat4 local_bind_pose;
       Mat4 inverse_bind_pose;  // スキニング用
   };

   struct Skeleton {
       std::vector<Bone> bones;
       int32_t find_bone(std::string_view name) const;
   };
   ```

2. **AnimationClip / Keyframe**
   ```cpp
   struct Keyframe {
       float time;
       Vec3f position;
       Quat rotation;
       Vec3f scale;
   };

   struct BoneChannel {
       int32_t bone_index;
       std::vector<Keyframe> keyframes;
   };

   struct AnimationClip {
       std::string name;
       float duration;
       std::vector<BoneChannel> channels;
   };
   ```

3. **AnimationPlayer**
   - クリップ再生、ブレンド (線形補間 + Slerp)
   - ボーン行列配列をUBOで頂点シェーダーに渡す
   - FBXローダーからアニメーションデータ抽出

---

### C6. ポストプロセス

#### 手順

1. **オフスクリーンレンダリング**
   - メインシーンをオフスクリーンFBOに描画
   - フルスクリーンクワッドにポストプロセスシェーダーを適用

2. **実装するエフェクト**

   | エフェクト | シェーダー | 用途 |
   |-----------|-----------|------|
   | フェードイン/アウト | `fade.frag` | シーン遷移 |
   | ブルーム | `bloom_extract.frag` + `bloom_blur.frag` + `bloom_composite.frag` | 光の拡散 |
   | トーンマッピング | `tonemap.frag` | HDR→SDR変換 |
   | ヴィネット | `vignette.frag` | 画面端の暗化 |
   | カラーグレーディング | `color_grade.frag` | LUT適用 |

3. **PostProcessStack**
   ```cpp
   class PostProcessStack {
       std::vector<PostProcessEffect*> effects_;
       VkFramebuffer offscreen_fbo_;
       VkImage offscreen_image_;
   public:
       void add_effect(PostProcessEffect* effect);
       void begin_scene();   // オフスクリーンFBOにバインド
       void end_scene();     // ポストプロセスチェーン実行
   };
   ```

---

### C7. デバッグ描画・imgui統合

#### 手順

1. **DebugDraw**
   ```cpp
   // engine/debug/debug_draw.hpp (新規)
   namespace debug_draw {
       void line(Vec2f from, Vec2f to, Color color);
       void line_3d(Vec3f from, Vec3f to, Color color);
       void rect_wireframe(Vec2f pos, Size2f size, Color color);
       void circle_wireframe(Vec2f center, float radius, Color color);
       void aabb_3d(Vec3f min, Vec3f max, Color color);
       void sphere_wireframe(Vec3f center, float radius, Color color);
       void text_world(Vec3f pos, const char* text, Color color);
       void grid(float spacing, int count, Color color);
       // 1フレームだけ有効、end_frame時にクリア
       void flush(RenderContext& ctx);
   }
   ```

2. **Dear ImGui 統合**
   ```
   engine/third_party/imgui/
   ├── imgui.h
   ├── imgui.cpp
   ├── imgui_draw.cpp
   ├── imgui_tables.cpp
   ├── imgui_widgets.cpp
   ├── imgui_impl_glfw.h/.cpp
   └── imgui_impl_vulkan.h/.cpp
   ```

   ```cpp
   // engine/debug/imgui_layer.hpp (新規)
   class ImGuiLayer {
   public:
       void initialize(GLFWwindow* window, VulkanRenderer& renderer);
       void begin_frame();
       void end_frame(VkCommandBuffer cmd);
       void shutdown();
   };
   ```

3. **用途**: FPS表示、エンティティインスペクタ、物理デバッグ、テクスチャ一覧

---

### C8. プロファイラー

#### 手順

1. **CPUプロファイラー**
   ```cpp
   // engine/debug/profiler.hpp (新規)
   #include <chrono>
   #include <string>
   #include <unordered_map>

   struct ProfileScope {
       std::string name;
       float elapsed_ms = 0.0f;
   };

   class Profiler {
       struct ScopeTimer {
           std::chrono::high_resolution_clock::time_point start;
           std::string name;
       };
       std::vector<ScopeTimer> stack_;
       std::unordered_map<std::string, float> results_;
   public:
       void begin(const char* name);
       void end();
       float get(const char* name) const;
       const auto& results() const { return results_; }
       void clear();
   };

   inline Profiler g_profiler;

   // RAII ヘルパー
   struct ScopedProfile {
       ScopedProfile(const char* name) { g_profiler.begin(name); }
       ~ScopedProfile() { g_profiler.end(); }
   };

   #define ERGO_PROFILE_SCOPE(name) ScopedProfile _profile_##__LINE__(name)
   ```

2. **GPUタイムスタンプ** (Vulkan)
   - `vkCmdWriteTimestamp` で描画パスの時間計測
   - `VkQueryPool` で結果取得

3. **表示**: ImGuiウィンドウでフレーム毎の各フェーズ時間をバーグラフ表示

---

## Phase D: 拡張・運用

**ゴール**: 複数プラットフォーム対応、チーム開発、大規模ゲーム対応

---

### D1. ネットワーク基盤

#### 手順

1. **Socket抽象化**
   ```cpp
   // engine/net/socket.hpp (新規)
   class TcpSocket {
   public:
       bool connect(std::string_view host, uint16_t port);
       bool listen(uint16_t port);
       TcpSocket accept();
       int send(const uint8_t* data, size_t len);
       int recv(uint8_t* buffer, size_t max_len);
       void close();
   };

   class UdpSocket {
   public:
       bool bind(uint16_t port);
       int send_to(const uint8_t* data, size_t len,
                   std::string_view host, uint16_t port);
       int recv_from(uint8_t* buffer, size_t max_len,
                     std::string& out_host, uint16_t& out_port);
       void close();
   };
   ```

2. **メッセージシステム**
   ```cpp
   struct NetMessage {
       uint16_t type;
       std::vector<uint8_t> payload;
   };

   class NetworkManager {
   public:
       void connect(std::string_view host, uint16_t port);
       void host(uint16_t port);
       void send(const NetMessage& msg);
       void poll();  // 受信バッファ処理
       void set_handler(uint16_t type, std::function<void(const NetMessage&)> handler);
   };
   ```

---

### D2. スクリプティング (Lua)

#### 手順

1. **Lua 導入**
   ```
   engine/third_party/lua/
   ├── lua.h, lualib.h, lauxlib.h
   └── ... (Lua 5.4 ソース)
   ```

2. **LuaEngine**
   ```cpp
   // engine/script/lua_engine.hpp (新規)
   class LuaEngine {
       lua_State* L_ = nullptr;
   public:
       bool initialize();
       void shutdown();
       bool execute_file(std::string_view path);
       bool execute_string(std::string_view code);

       // エンジンAPIのバインド
       void bind_input();
       void bind_math();
       void bind_physics();
       void bind_rendering();
   };
   ```

3. **ゲームオブジェクトへの組み込み**
   - `LuaScriptComponent` として GameObject に追加
   - `start()`, `update(dt)`, `on_hit()` をLua関数で記述

---

### D3. ホットリロード

#### 手順

1. **DLL ホットリロード**
   ```cpp
   // runtime/hot_reloader.hpp (新規)
   class HotReloader {
       std::string dll_path_;
       std::string temp_path_;
       GameDLL current_;
       std::filesystem::file_time_type last_modified_;

   public:
       void watch(std::string_view dll_path);
       bool check_and_reload();  // ファイル変更検出 → アンロード → コピー → ロード
   };
   ```

2. **ファイルウォッチャー**
   - `std::filesystem::last_write_time` でポーリング (簡易)
   - 将来的には `inotify` (Linux) / `ReadDirectoryChangesW` (Win) 対応

3. **ステート保持**
   - リロード前にゲーム状態をシリアライズ → リロード後にデシリアライズ

---

### D4. アセットパイプライン

#### 手順

1. **アセットコンパイラ CLI**
   ```
   tools/asset_compiler/
   ├── CMakeLists.txt
   ├── main.cpp           # CLI エントリー
   ├── texture_compiler.cpp  # PNG/JPG → BCn/ASTC圧縮
   ├── shader_compiler.cpp   # GLSL → SPIR-V
   ├── model_compiler.cpp    # FBX → エンジン独自バイナリ
   └── audio_compiler.cpp    # WAV → OGG変換
   ```

2. **アセットマニフェスト**
   ```json
   // assets.manifest.json
   {
     "textures": [
       {"src": "raw/player.png", "dst": "data/player.tex", "format": "bc7"},
       ...
     ],
     "shaders": [
       {"src": "shaders/shape2d.vert", "dst": "data/shape2d.vert.spv"},
       ...
     ]
   }
   ```

3. **パックファイル** (.pak)
   - 複数アセットを1ファイルにまとめる
   - ヘッダ (TOC) + 連結データ
   - ストリーミング読み込み対応

---

### D5. Android/iOS対応

#### Android

1. `system/window/android_window.cpp` — `ANativeWindow` 実装
2. `system/input/touch_input.cpp` — `AInputEvent` ハンドリング
3. `android/` ディレクトリに Gradle プロジェクト + JNI ブリッジ
4. `CMakeLists.txt` に NDK ツールチェーン対応分岐

#### iOS

1. `system/window/ios_window.mm` — `UIWindow` + `CAMetalLayer`
2. MoltenVK 経由で Vulkan API を使用
3. `ios/` ディレクトリに Xcode プロジェクトテンプレート

---

### D6. ECS移行

**現状**: `GameObject` + `std::any` コンポーネントマップ (疎結合だが遅い)

#### 手順

1. **Archetype ストレージ**
   ```cpp
   // engine/ecs/archetype.hpp (新規)
   // コンポーネントの型の組み合わせ (Archetype) ごとに
   // 連続メモリで管理する SoA (Structure of Arrays)

   using ComponentId = uint32_t;
   using ArchetypeId = uint64_t;

   struct ComponentArray {
       ComponentId type;
       size_t element_size;
       std::vector<uint8_t> data;
   };

   struct Archetype {
       ArchetypeId id;
       std::vector<ComponentArray> columns;
       size_t entity_count = 0;
   };
   ```

2. **World**
   ```cpp
   class World {
       std::unordered_map<ArchetypeId, Archetype> archetypes_;
       std::unordered_map<uint64_t, ArchetypeId> entity_archetype_;

   public:
       uint64_t create_entity();
       void destroy_entity(uint64_t id);

       template<typename T>
       void add_component(uint64_t entity, T component);

       template<typename T>
       T* get_component(uint64_t entity);

       // クエリ: 指定コンポーネントを全て持つエンティティを列挙
       template<typename... Ts>
       void each(std::function<void(uint64_t, Ts&...)> fn);
   };
   ```

3. **既存 TaskSystem との共存**
   - ECS はデータ管理のみ
   - TaskSystem はロジック実行順序の管理を担当

---

### D7. テスト・CI/CD

#### テスト

1. **テストフレームワーク**: Catch2 or Google Test
   ```
   tests/
   ├── CMakeLists.txt
   ├── test_vec2.cpp
   ├── test_vec3.cpp
   ├── test_mat4.cpp
   ├── test_quat.cpp
   ├── test_hit_test.cpp
   ├── test_collision3d.cpp
   ├── test_task_system.cpp
   ├── test_state_machine.cpp
   ├── test_physics_system.cpp
   └── test_rigid_body_world.cpp
   ```

2. **テスト対象優先度**

   | 優先度 | モジュール | テスト内容 |
   |--------|----------|-----------|
   | 最高 | math | 演算の正確性、エッジケース |
   | 最高 | hit_test | 全コライダー組み合わせの判定 |
   | 高 | collision3d | 3D衝突の正確性 |
   | 高 | task_system | 登録/破棄/フェーズ実行 |
   | 中 | rigid_body_world | 積分、衝突応答、スリープ |
   | 中 | state_machine | 遷移、enter/exit呼び出し |

#### CI/CD

1. **GitHub Actions**
   ```yaml
   # .github/workflows/build.yml
   name: Build
   on: [push, pull_request]
   jobs:
     linux:
       runs-on: ubuntu-latest
       steps:
         - uses: actions/checkout@v4
         - name: Install deps
           run: |
             sudo apt-get update
             sudo apt-get install -y cmake g++ libvulkan-dev libglfw3-dev
         - name: Build
           run: |
             cmake -B build -DCMAKE_BUILD_TYPE=Release -DERGO_BUILD_TESTS=ON
             cmake --build build
         - name: Test
           run: cd build && ctest --output-on-failure

     windows:
       runs-on: windows-latest
       steps:
         - uses: actions/checkout@v4
         - name: Build
           run: |
             cmake -B build -G "Visual Studio 17 2022"
             cmake --build build --config Release
   ```

---

## 実装順序サマリー

```
Phase A (必須)                 Phase B (実用)
─────────────────             ─────────────────
A1. GLFW統合          ──→     B1. 入力完成
A2. Vulkan初期化      ──→     B3. カメラ
A3. 2D描画パイプライン ──→     B6. スプライトアニメ
A4. テクスチャ        ──→     B5. リソース管理
A5. フォント          ──→     B2. オーディオ
A6. Time管理                  B4. シーン管理
A7. ログ                      B7. Tween
                              B8. 2D物理拡張

Phase C (品質)                Phase D (拡張)
─────────────────             ─────────────────
C1. UI                        D1. ネットワーク
C2. パーティクル              D2. Luaスクリプト
C3. シリアライゼーション      D3. ホットリロード
C4. ライティング              D4. アセットパイプライン
C5. スケルタルアニメ          D5. Android/iOS
C6. ポストプロセス            D6. ECS移行
C7. imgui統合                 D7. テスト・CI/CD
C8. プロファイラー
```

**推定工数**:
- Phase A: 2〜4週間
- Phase B: 3〜6週間
- Phase C: 4〜8週間
- Phase D: 各項目 1〜3週間
