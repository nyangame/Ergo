#!/usr/bin/env node
/**
 * Ergo エージェントオーケストレーター
 *
 * Claude Code SDK を使って複数の専門エージェントを並列実行し、
 * エンジン開発を駆動する。
 *
 * 使い方:
 *   npx tsx src/orchestrator.ts --mode implement --agents engine-core,engine-physics --task "物理エンジンのブロードフェーズを実装"
 *   npx tsx src/orchestrator.ts --mode review --agents all
 *   npx tsx src/orchestrator.ts --mode implement --category engine --task "IMPLEMENTATION_ROADMAP Phase A を進める"
 */

import { query } from "@anthropic-ai/claude-code";
import { ERGO_AGENTS, getAgentsByCategory, listAgentNames } from "./agents.js";
import type { AgentDefinition } from "./agents.js";

// ─── 型定義 ─────────────────────────────────────────────

interface OrchestratorConfig {
  /** 実行モード */
  mode: "implement" | "review" | "plan";
  /** 使用するエージェント名のリスト (空 = 自動選択) */
  agentNames: string[];
  /** カテゴリ指定 (agentNames が空の場合に使用) */
  category?: "engine" | "system" | "web" | "sample" | "build" | "review";
  /** タスクの説明 */
  task: string;
  /** 最大予算 (USD) */
  maxBudgetUsd?: number;
  /** 各エージェントの最大ターン数 */
  maxTurns?: number;
  /** 作業ディレクトリ */
  cwd: string;
}

interface AgentResult {
  agentName: string;
  success: boolean;
  output: string;
  error?: string;
  durationMs: number;
}

// ─── CLI 引数パース ──────────────────────────────────────

function parseArgs(): OrchestratorConfig {
  const args = process.argv.slice(2);
  const config: OrchestratorConfig = {
    mode: "implement",
    agentNames: [],
    task: "",
    cwd: process.cwd(),
    maxTurns: 30,
  };

  for (let i = 0; i < args.length; i++) {
    switch (args[i]) {
      case "--mode":
        config.mode = args[++i] as OrchestratorConfig["mode"];
        break;
      case "--agents":
        config.agentNames = args[++i].split(",").map((s) => s.trim());
        break;
      case "--category":
        config.category = args[++i] as OrchestratorConfig["category"];
        break;
      case "--task":
        config.task = args[++i];
        break;
      case "--budget":
        config.maxBudgetUsd = parseFloat(args[++i]);
        break;
      case "--max-turns":
        config.maxTurns = parseInt(args[++i], 10);
        break;
      case "--cwd":
        config.cwd = args[++i];
        break;
      case "--help":
        printHelp();
        process.exit(0);
      default:
        // 引数以外のテキストはタスクとして扱う
        if (!args[i].startsWith("--") && !config.task) {
          config.task = args[i];
        }
    }
  }

  if (!config.task) {
    console.error("エラー: --task でタスクを指定してください");
    printHelp();
    process.exit(1);
  }

  return config;
}

function printHelp(): void {
  console.log(`
Ergo エージェントオーケストレーター

使い方:
  npx tsx src/orchestrator.ts [オプション]

オプション:
  --mode <implement|review|plan>  実行モード (デフォルト: implement)
  --agents <name1,name2,...>      使用するエージェント (カンマ区切り)
  --category <category>           カテゴリ指定 (engine|system|web|sample|build|review)
  --task <description>            タスクの説明 (必須)
  --budget <usd>                  最大予算 USD
  --max-turns <n>                 最大ターン数 (デフォルト: 30)
  --cwd <path>                    作業ディレクトリ
  --help                          ヘルプ表示

利用可能なエージェント:
${listAgentNames()
  .map((n) => `  - ${n}: ${ERGO_AGENTS[n].description}`)
  .join("\n")}

例:
  # エンジンコアと物理を並列で実装
  npx tsx src/orchestrator.ts --mode implement --agents engine-core,engine-physics --task "Phase A の基盤実装"

  # エンジン系エージェント全体でレビュー
  npx tsx src/orchestrator.ts --mode review --category engine --task "コーディング規約チェック"

  # 全エージェントを使って計画策定
  npx tsx src/orchestrator.ts --mode plan --agents all --task "Phase B の実装計画を立てる"
`);
}

// ─── エージェント実行 ─────────────────────────────────────

/**
 * 単一エージェントを実行して結果を返す
 */
async function runAgent(
  agentName: string,
  agentDef: AgentDefinition,
  task: string,
  config: OrchestratorConfig
): Promise<AgentResult> {
  const startTime = Date.now();
  const outputChunks: string[] = [];

  console.log(`[${agentName}] 開始: ${task.slice(0, 80)}...`);

  try {
    const permissionMode =
      config.mode === "review"
        ? "default"
        : config.mode === "plan"
          ? "plan"
          : "acceptEdits";

    const prompt = `
${agentDef.instructions}

--- タスク ---
${task}

--- モード ---
${config.mode === "review" ? "コードレビューモード: 変更は加えず、問題点の指摘のみ行ってください。" : ""}
${config.mode === "plan" ? "計画モード: 実装計画のみ策定し、コード変更は行わないでください。" : ""}
${config.mode === "implement" ? "実装モード: タスクに基づいてコードを実装してください。" : ""}
`.trim();

    const q = query({
      prompt,
      options: {
        allowedTools: agentDef.allowedTools,
        maxTurns: config.maxTurns,
        maxBudgetUsd: config.maxBudgetUsd,
        cwd: config.cwd,
        permissionMode: permissionMode as
          | "default"
          | "acceptEdits"
          | "plan"
          | "bypassPermissions",
        systemPrompt: {
          type: "preset" as const,
          preset: "claude_code" as const,
          append: `あなたは Ergo ゲームエンジンの「${agentName}」担当エージェントです。`,
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
      } else if (message.type === "result") {
        outputChunks.push(
          `\n--- 結果 ---\n${JSON.stringify(message.result, null, 2)}`
        );
      }
    }

    const durationMs = Date.now() - startTime;
    console.log(
      `[${agentName}] 完了 (${(durationMs / 1000).toFixed(1)}s)`
    );

    return {
      agentName,
      success: true,
      output: outputChunks.join("\n"),
      durationMs,
    };
  } catch (error) {
    const durationMs = Date.now() - startTime;
    const errorMsg =
      error instanceof Error ? error.message : String(error);
    console.error(`[${agentName}] エラー: ${errorMsg}`);

    return {
      agentName,
      success: false,
      output: outputChunks.join("\n"),
      error: errorMsg,
      durationMs,
    };
  }
}

/**
 * 複数エージェントを並列実行
 */
async function runAgentsParallel(
  agents: Record<string, AgentDefinition>,
  task: string,
  config: OrchestratorConfig
): Promise<AgentResult[]> {
  const promises = Object.entries(agents).map(([name, def]) =>
    runAgent(name, def, task, config)
  );

  return Promise.all(promises);
}

// ─── サブエージェント付きオーケストレーション ───────────────

/**
 * Claude の Task ツールを使ったサブエージェント委任型オーケストレーション
 *
 * メインエージェントが判断して適切なサブエージェントにタスクを振り分ける。
 */
async function runWithSubagents(
  agents: Record<string, AgentDefinition>,
  task: string,
  config: OrchestratorConfig
): Promise<AgentResult> {
  const startTime = Date.now();
  const outputChunks: string[] = [];

  // サブエージェント定義を SDK 形式に変換
  const sdkAgents: Record<
    string,
    { description: string; prompt: string; tools: string[]; model?: string }
  > = {};
  for (const [name, def] of Object.entries(agents)) {
    sdkAgents[name] = {
      description: def.description,
      prompt: def.instructions,
      tools: def.allowedTools,
      model: def.model,
    };
  }

  const orchestratorPrompt = `
あなたは Ergo ゲームエンジンのリードアーキテクトです。
以下のタスクを、適切なサブエージェントに並列で委任してください。

利用可能なサブエージェント:
${Object.entries(agents)
  .map(([name, def]) => `- ${name}: ${def.description}`)
  .join("\n")}

タスク: ${task}

手順:
1. タスクを分析し、どのサブエージェントに何を依頼するか計画
2. Task ツールを使って複数のサブエージェントを同時に起動
3. 各サブエージェントの結果を統合して最終レポートを作成
`.trim();

  try {
    const q = query({
      prompt: orchestratorPrompt,
      options: {
        allowedTools: [
          "Read",
          "Glob",
          "Grep",
          "Task",
          "TodoWrite",
        ],
        agents: sdkAgents,
        maxTurns: config.maxTurns,
        maxBudgetUsd: config.maxBudgetUsd,
        cwd: config.cwd,
        permissionMode: "acceptEdits",
        systemPrompt: {
          type: "preset" as const,
          preset: "claude_code" as const,
          append:
            "あなたは Ergo ゲームエンジンのリードアーキテクトエージェントです。サブエージェントを効果的に活用してタスクを完遂してください。",
        },
      },
    });

    for await (const message of q) {
      if (message.type === "assistant") {
        for (const block of message.content) {
          if (block.type === "text") {
            outputChunks.push(block.text);
            process.stdout.write(block.text);
          }
        }
      } else if (message.type === "result") {
        outputChunks.push(
          `\n--- 最終結果 ---\n${JSON.stringify(message.result, null, 2)}`
        );
      }
    }

    return {
      agentName: "orchestrator",
      success: true,
      output: outputChunks.join("\n"),
      durationMs: Date.now() - startTime,
    };
  } catch (error) {
    return {
      agentName: "orchestrator",
      success: false,
      output: outputChunks.join("\n"),
      error: error instanceof Error ? error.message : String(error),
      durationMs: Date.now() - startTime,
    };
  }
}

// ─── レポート生成 ─────────────────────────────────────────

function printReport(results: AgentResult[]): void {
  console.log("\n" + "=".repeat(60));
  console.log("  Ergo エージェント実行レポート");
  console.log("=".repeat(60));

  const succeeded = results.filter((r) => r.success);
  const failed = results.filter((r) => !r.success);

  console.log(`\n  合計: ${results.length} エージェント`);
  console.log(`  成功: ${succeeded.length}`);
  console.log(`  失敗: ${failed.length}`);
  console.log(
    `  総時間: ${(Math.max(...results.map((r) => r.durationMs)) / 1000).toFixed(1)}s (並列実行)`
  );

  for (const result of results) {
    const status = result.success ? "[OK]" : "[FAIL]";
    const duration = (result.durationMs / 1000).toFixed(1);
    console.log(`\n  ${status} ${result.agentName} (${duration}s)`);
    if (result.error) {
      console.log(`    エラー: ${result.error}`);
    }
    // 出力の最初の数行を表示
    const preview = result.output.split("\n").slice(0, 5).join("\n    ");
    if (preview.trim()) {
      console.log(`    ${preview}`);
    }
  }

  console.log("\n" + "=".repeat(60));
}

// ─── メイン ──────────────────────────────────────────────

async function main(): Promise<void> {
  const config = parseArgs();

  // 使用するエージェントを決定
  let agents: Record<string, AgentDefinition>;

  if (
    config.agentNames.length === 1 &&
    config.agentNames[0] === "all"
  ) {
    agents = { ...ERGO_AGENTS };
  } else if (config.agentNames.length > 0) {
    agents = {};
    for (const name of config.agentNames) {
      if (!ERGO_AGENTS[name]) {
        console.error(
          `エラー: 不明なエージェント '${name}'\n利用可能: ${listAgentNames().join(", ")}`
        );
        process.exit(1);
      }
      agents[name] = ERGO_AGENTS[name];
    }
  } else if (config.category) {
    agents = getAgentsByCategory(config.category);
    if (Object.keys(agents).length === 0) {
      console.error(
        `エラー: カテゴリ '${config.category}' にエージェントがありません`
      );
      process.exit(1);
    }
  } else {
    // デフォルト: 全エージェント
    agents = { ...ERGO_AGENTS };
  }

  console.log(`\nErgo エージェントオーケストレーター`);
  console.log(`モード: ${config.mode}`);
  console.log(
    `エージェント: ${Object.keys(agents).join(", ")}`
  );
  console.log(`タスク: ${config.task}`);
  console.log("");

  // エージェント数に応じて実行方式を選択
  const agentCount = Object.keys(agents).length;

  if (agentCount === 1) {
    // 単一エージェント: 直接実行
    const [name, def] = Object.entries(agents)[0];
    const result = await runAgent(name, def, config.task, config);
    printReport([result]);
  } else if (agentCount <= 3) {
    // 少数エージェント: 並列直接実行
    const results = await runAgentsParallel(
      agents,
      config.task,
      config
    );
    printReport(results);
  } else {
    // 多数エージェント: サブエージェント委任型
    const result = await runWithSubagents(
      agents,
      config.task,
      config
    );
    printReport([result]);
  }
}

main().catch((error) => {
  console.error("致命的エラー:", error);
  process.exit(1);
});
