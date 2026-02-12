#!/usr/bin/env node
/**
 * 並列タスク実行スクリプト
 *
 * 複数の独立したタスクをそれぞれ専用エージェントに割り当てて並列実行する。
 * JSON ファイルまたはコマンドラインからタスク定義を受け取る。
 *
 * 使い方:
 *   npx tsx src/parallel-tasks.ts --file tasks.json
 *   npx tsx src/parallel-tasks.ts \
 *     --assign engine-core:"タスクシステム改善" \
 *     --assign engine-physics:"ブロードフェーズ実装" \
 *     --assign web-port:"WebGPU テクスチャ対応"
 */

import { query } from "@anthropic-ai/claude-code";
import { ERGO_AGENTS, listAgentNames } from "./agents.js";
import type { AgentDefinition } from "./agents.js";
import { readFile } from "node:fs/promises";

// ─── 型定義 ─────────────────────────────────────────────

interface TaskAssignment {
  agent: string;
  task: string;
  maxTurns?: number;
}

interface ParallelResult {
  agent: string;
  task: string;
  success: boolean;
  output: string;
  error?: string;
  durationMs: number;
}

// ─── 引数パース ──────────────────────────────────────────

async function parseAssignments(): Promise<TaskAssignment[]> {
  const args = process.argv.slice(2);
  const assignments: TaskAssignment[] = [];

  for (let i = 0; i < args.length; i++) {
    if (args[i] === "--file") {
      const filePath = args[++i];
      const content = await readFile(filePath, "utf-8");
      const parsed = JSON.parse(content) as TaskAssignment[];
      assignments.push(...parsed);
    } else if (args[i] === "--assign") {
      const value = args[++i];
      const colonIdx = value.indexOf(":");
      if (colonIdx === -1) {
        console.error(
          `エラー: --assign の形式は 'agent-name:タスク説明' です`
        );
        process.exit(1);
      }
      assignments.push({
        agent: value.slice(0, colonIdx),
        task: value.slice(colonIdx + 1),
      });
    } else if (args[i] === "--help") {
      printHelp();
      process.exit(0);
    }
  }

  if (assignments.length === 0) {
    printHelp();
    process.exit(1);
  }

  return assignments;
}

function printHelp(): void {
  console.log(`
並列タスク実行

使い方:
  npx tsx src/parallel-tasks.ts --assign <agent>:<task> [--assign ...]
  npx tsx src/parallel-tasks.ts --file <tasks.json>

tasks.json の形式:
  [
    { "agent": "engine-core", "task": "タスクの説明" },
    { "agent": "engine-physics", "task": "別のタスク" }
  ]

利用可能なエージェント:
${listAgentNames()
  .map((n) => `  ${n}`)
  .join("\n")}

例:
  npx tsx src/parallel-tasks.ts \\
    --assign engine-core:"state_machine.hpp に遷移アニメーション対応を追加" \\
    --assign engine-physics:"衝突判定のSAT アルゴリズムを実装" \\
    --assign web-port:"WebGPU のコンピュートシェーダー対応"
`);
}

// ─── 単一タスク実行 ──────────────────────────────────────

async function executeTask(
  assignment: TaskAssignment
): Promise<ParallelResult> {
  const agentDef = ERGO_AGENTS[assignment.agent];
  if (!agentDef) {
    return {
      agent: assignment.agent,
      task: assignment.task,
      success: false,
      output: "",
      error: `不明なエージェント: ${assignment.agent}`,
      durationMs: 0,
    };
  }

  const startTime = Date.now();
  const outputChunks: string[] = [];

  console.log(
    `[${assignment.agent}] 開始: ${assignment.task.slice(0, 60)}...`
  );

  try {
    const prompt = `
${agentDef.instructions}

--- タスク ---
${assignment.task}

実装モードで作業してください。
`.trim();

    const q = query({
      prompt,
      options: {
        allowedTools: agentDef.allowedTools,
        maxTurns: assignment.maxTurns ?? 30,
        cwd: process.cwd(),
        permissionMode: "acceptEdits",
        systemPrompt: {
          type: "preset" as const,
          preset: "claude_code" as const,
          append: `あなたは Ergo ゲームエンジンの「${assignment.agent}」担当エージェントです。`,
        },
      },
    });

    for await (const message of q) {
      if (message.type === "assistant") {
        for (const block of message.content) {
          if (block.type === "text") {
            outputChunks.push(block.text);
          }
        }
      }
    }

    const durationMs = Date.now() - startTime;
    console.log(
      `[${assignment.agent}] 完了 (${(durationMs / 1000).toFixed(1)}s)`
    );

    return {
      agent: assignment.agent,
      task: assignment.task,
      success: true,
      output: outputChunks.join("\n"),
      durationMs,
    };
  } catch (error) {
    const durationMs = Date.now() - startTime;
    const errorMsg =
      error instanceof Error ? error.message : String(error);
    console.error(`[${assignment.agent}] エラー: ${errorMsg}`);

    return {
      agent: assignment.agent,
      task: assignment.task,
      success: false,
      output: outputChunks.join("\n"),
      error: errorMsg,
      durationMs,
    };
  }
}

// ─── メイン ──────────────────────────────────────────────

async function main(): Promise<void> {
  const assignments = await parseAssignments();

  console.log(`\n並列タスク実行: ${assignments.length} タスク`);
  console.log("-".repeat(60));

  // 全タスクを並列実行
  const results = await Promise.all(
    assignments.map((a) => executeTask(a))
  );

  // レポート
  console.log("\n" + "=".repeat(60));
  console.log("  並列タスク実行レポート");
  console.log("=".repeat(60));

  const succeeded = results.filter((r) => r.success);
  const failed = results.filter((r) => !r.success);

  console.log(`  合計: ${results.length}`);
  console.log(`  成功: ${succeeded.length}`);
  console.log(`  失敗: ${failed.length}`);

  for (const result of results) {
    const icon = result.success ? "[OK]" : "[FAIL]";
    const dur = (result.durationMs / 1000).toFixed(1);
    console.log(`\n  ${icon} ${result.agent} (${dur}s)`);
    console.log(`    タスク: ${result.task}`);
    if (result.error) {
      console.log(`    エラー: ${result.error}`);
    }
  }

  console.log("\n" + "=".repeat(60));

  if (failed.length > 0) {
    process.exit(1);
  }
}

main().catch((error) => {
  console.error("致命的エラー:", error);
  process.exit(1);
});
