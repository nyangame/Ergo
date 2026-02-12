# Ergo

軽量クロスプラットフォームゲームエンジン

## 特徴

- **C++20 / concept ベース**: 継承を使わず、concept + std::variant で型安全な設計
- **クロスプラットフォーム**: Windows / Linux / Android / iOS / Web
- **Vulkan 描画**: ネイティブは Vulkan、Web は WebGPU
- **DLL ベースのゲーム構造**: 1ゲーム = 1 DLL (共有ライブラリ)
- **軽量設計**: ハイグラフィック不要、2D/軽量3D に特化

## アーキテクチャ

```
Application Layer (Game DLL)    ← ゲームロジック
Engine Layer (libErgoEngine)    ← タスク / 物理 / ステート
System Layer (libErgoSystem)    ← Vulkan / Input / Window
```

詳細は [DESIGN.md](DESIGN.md) を参照。

## ビルド

### 必要環境

- CMake 3.20+
- C++20 対応コンパイラ (MSVC 2022 / GCC 12+ / Clang 15+)
- Vulkan SDK

### ビルド手順

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## ライセンス

MIT License - Copyright (c) 2026 k.mitarai
