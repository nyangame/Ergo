/**
 * Ergo エージェントオーケストレーター
 *
 * Claude Code SDK を使って Ergo ゲームエンジンの開発を
 * 複数の専門エージェントで並列駆動する。
 *
 * @example
 * ```typescript
 * import { ERGO_AGENTS, runErgoAgent } from "ergo-agents";
 *
 * // 単一エージェント実行
 * const result = await runErgoAgent("engine-core", "タスクシステム改善");
 *
 * // 並列実行
 * const results = await runErgoAgentsParallel(
 *   ["engine-core", "engine-physics"],
 *   "Phase A 基盤実装"
 * );
 * ```
 */

export { ERGO_AGENTS, listAgentNames, getAgentsByCategory } from "./agents.js";
export type { AgentDefinition } from "./agents.js";

import { query } from "@anthropic-ai/claude-code";
import { ERGO_AGENTS } from "./agents.js";

/**
 * 単一の Ergo エージェントを実行する
 */
export async function runErgoAgent(
  agentName: string,
  task: string,
  options?: {
    cwd?: string;
    maxTurns?: number;
    maxBudgetUsd?: number;
    permissionMode?: "default" | "acceptEdits" | "bypassPermissions" | "plan";
  }
): Promise<{ success: boolean; output: string; error?: string }> {
  const agentDef = ERGO_AGENTS[agentName];
  if (!agentDef) {
    throw new Error(
      `不明なエージェント: ${agentName}. 利用可能: ${Object.keys(ERGO_AGENTS).join(", ")}`
    );
  }

  const outputChunks: string[] = [];

  try {
    const q = query({
      prompt: `${agentDef.instructions}\n\n--- タスク ---\n${task}`,
      options: {
        allowedTools: agentDef.allowedTools,
        maxTurns: options?.maxTurns ?? 30,
        maxBudgetUsd: options?.maxBudgetUsd,
        cwd: options?.cwd ?? process.cwd(),
        permissionMode: options?.permissionMode ?? "acceptEdits",
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
      }
    }

    return { success: true, output: outputChunks.join("\n") };
  } catch (error) {
    return {
      success: false,
      output: outputChunks.join("\n"),
      error: error instanceof Error ? error.message : String(error),
    };
  }
}

/**
 * 複数の Ergo エージェントを並列実行する
 */
export async function runErgoAgentsParallel(
  agentNames: string[],
  task: string,
  options?: {
    cwd?: string;
    maxTurns?: number;
    maxBudgetUsd?: number;
  }
): Promise<
  Array<{
    agentName: string;
    success: boolean;
    output: string;
    error?: string;
  }>
> {
  const promises = agentNames.map(async (name) => {
    const result = await runErgoAgent(name, task, options);
    return { agentName: name, ...result };
  });

  return Promise.all(promises);
}

/**
 * エージェントごとに異なるタスクを並列実行する
 */
export async function runErgoTasksParallel(
  assignments: Array<{ agent: string; task: string; maxTurns?: number }>,
  options?: {
    cwd?: string;
    maxBudgetUsd?: number;
  }
): Promise<
  Array<{
    agentName: string;
    task: string;
    success: boolean;
    output: string;
    error?: string;
  }>
> {
  const promises = assignments.map(async (assignment) => {
    const result = await runErgoAgent(assignment.agent, assignment.task, {
      ...options,
      maxTurns: assignment.maxTurns,
    });
    return { agentName: assignment.agent, task: assignment.task, ...result };
  });

  return Promise.all(promises);
}
