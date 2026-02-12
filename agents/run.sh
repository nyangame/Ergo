#!/bin/bash
# Ergo エージェントオーケストレーター起動スクリプト
#
# 使い方:
#   ./run.sh orchestrate --task "Phase A を実装" --category engine
#   ./run.sh single engine-core "タスクシステム改善"
#   ./run.sh parallel --file tasks.json
#   ./run.sh install   # 依存関係インストール

set -euo pipefail
cd "$(dirname "$0")"

# 依存関係チェック
check_deps() {
  if [ ! -d "node_modules" ]; then
    echo "依存関係をインストールしています..."
    npm install
  fi
}

case "${1:-help}" in
  install)
    npm install
    echo "インストール完了"
    ;;

  orchestrate)
    check_deps
    shift
    cd ..
    node --import tsx agents/src/orchestrator.ts "$@"
    ;;

  single)
    check_deps
    shift
    cd ..
    node --import tsx agents/src/run-single.ts "$@"
    ;;

  parallel)
    check_deps
    shift
    cd ..
    node --import tsx agents/src/parallel-tasks.ts "$@"
    ;;

  help|--help|-h|*)
    cat << 'EOF'
Ergo エージェントオーケストレーター

コマンド:
  install                           依存関係をインストール
  orchestrate [options]             メインオーケストレーター実行
  single <agent> <task>             単一エージェント実行
  parallel --assign agent:task ...  並列タスク実行

オーケストレーター オプション:
  --mode <implement|review|plan>    実行モード
  --agents <name1,name2,...>        エージェント指定
  --category <engine|system|web|..> カテゴリ指定
  --task <description>              タスク (必須)
  --budget <usd>                    予算上限
  --max-turns <n>                   最大ターン数

例:
  # エンジン系エージェントで Phase A 実装
  ./run.sh orchestrate --mode implement --category engine --task "Phase A を進める"

  # 物理エンジンの単体作業
  ./run.sh single engine-physics "ブロードフェーズをグリッドベースで実装"

  # 3 つのタスクを並列実行
  ./run.sh parallel \
    --assign engine-core:"タスク優先度対応" \
    --assign engine-render:"スプライトバッチ実装" \
    --assign web-port:"WebGPU テクスチャ対応"

  # JSON ファイルから並列実行
  ./run.sh parallel --file tasks-example.json

  # 全体のコードレビュー
  ./run.sh orchestrate --mode review --agents code-reviewer --task "全モジュールのレビュー"
EOF
    ;;
esac
