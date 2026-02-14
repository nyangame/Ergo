#!/usr/bin/env node
/**
 * 単一エージェント実行スクリプト
 *
 * 特定のエージェントを単体で実行する。
 * 対話的にストリーミング出力を表示する。
 *
 * 使い方:
 *   npx tsx src/run-single.ts engine-core "タスクシステムに優先度ベースのスケジューリングを追加"
 *   npx tsx src/run-single.ts code-reviewer "engine/core/ のコードレビュー"
 */

import { query } from "@anthropic-ai/claude-code";
import { ERGO_AGENTS, listAgentNames } from "./agents.js";

async function main(): Promise<void> {
  const [agentName, ...taskParts] = process.argv.slice(2);
  const task = taskParts.join(" ");

  if (!agentName || !task) {
    console.log(`
単一エージェント実行

使い方:
  npx tsx src/run-single.ts <agent-name> <task>

利用可能なエージェント:
${listAgentNames()
  .map((n) => `  ${n.padEnd(20)} ${ERGO_AGENTS[n].description}`)
  .join("\n")}

例:
  npx tsx src/run-single.ts engine-core "concepts.hpp に Renderable concept を追加"
  npx tsx src/run-single.ts code-reviewer "engine/physics/ 全体のレビュー"
  npx tsx src/run-single.ts web-port "WebGPU レンダラーにスプライトバッチを実装"
`);
    process.exit(1);
  }

  const agentDef = ERGO_AGENTS[agentName];
  if (!agentDef) {
    console.error(
      `エラー: 不明なエージェント '${agentName}'\n利用可能: ${listAgentNames().join(", ")}`
    );
    process.exit(1);
  }

  console.log(`[${agentName}] ${agentDef.description}`);
  console.log(`タスク: ${task}`);
  console.log("-".repeat(60));

  const prompt = `
${agentDef.instructions}

--- タスク ---
${task}
`.trim();

  const q = query({
    prompt,
    options: {
      allowedTools: agentDef.allowedTools,
      maxTurns: 50,
      cwd: process.cwd(),
      permissionMode: "acceptEdits",
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
          process.stdout.write(block.text);
        }
      }
    } else if (message.type === "result") {
      console.log("\n" + "=".repeat(60));
      console.log("完了");
      console.log("=".repeat(60));
    }
  }
}

main().catch((error) => {
  console.error("エラー:", error);
  process.exit(1);
});
