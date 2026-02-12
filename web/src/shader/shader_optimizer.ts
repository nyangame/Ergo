import { ShaderGraph } from './shader_graph.js';
import { ShaderNode, MathOp } from './shader_node.js';

// ============================================================
// ShaderOptimizer: AI-driven shader optimization (TypeScript)
// ============================================================

export interface OptimizationResult {
    passName: string;
    changesMade: number;
    description: string;
}

export class ShaderOptimizer {
    private report_: OptimizationResult[] = [];

    enableConstantFolding = true;
    enableDeadCode = true;
    enableAlgebraic = true;

    get results(): OptimizationResult[] { return this.report_; }

    optimizeGraph(graph: ShaderGraph): void {
        this.report_ = [];
        let changed = true;
        let iteration = 0;

        while (changed && iteration < 16) {
            changed = false;
            iteration++;

            if (this.enableDeadCode) {
                const n = this.passDeadNodeElimination(graph);
                if (n > 0) {
                    this.report_.push({
                        passName: 'DeadNodeElimination', changesMade: n,
                        description: `Removed ${n} unreachable nodes`
                    });
                    changed = true;
                }
            }

            if (this.enableConstantFolding) {
                const n = this.passConstantFolding(graph);
                if (n > 0) {
                    this.report_.push({
                        passName: 'ConstantFolding', changesMade: n,
                        description: `Folded ${n} constant expressions`
                    });
                    changed = true;
                }
            }

            if (this.enableAlgebraic) {
                const n = this.passIdentityRemoval(graph);
                if (n > 0) {
                    this.report_.push({
                        passName: 'IdentityRemoval', changesMade: n,
                        description: `Removed ${n} identity operations`
                    });
                    changed = true;
                }
            }
        }
    }

    optimizeSource(src: string): string {
        if (this.enableDeadCode) {
            src = this.passRemoveDeadAssignments(src);
        }
        return src;
    }

    optimizationReport(): string {
        if (this.report_.length === 0) return 'No optimizations applied.\n';
        let s = '=== Shader Optimization Report ===\n';
        let total = 0;
        for (const r of this.report_) {
            s += `  [${r.passName}] ${r.description}\n`;
            total += r.changesMade;
        }
        s += `  Total changes: ${total}\n`;
        s += '==================================\n';
        return s;
    }

    // --- Passes ---

    private passDeadNodeElimination(graph: ShaderGraph): number {
        const outputId = graph.findOutputNode();
        if (outputId === 0) return 0;

        const reachable = new Set<number>();
        const queue = [outputId];
        reachable.add(outputId);

        while (queue.length > 0) {
            const current = queue.pop()!;
            const node = graph.getNode(current);
            if (!node) continue;

            for (let i = 0; i < node.inputs.length; i++) {
                const conn = graph.findInputConnection(current, i);
                if (conn && !reachable.has(conn.sourceNode)) {
                    reachable.add(conn.sourceNode);
                    queue.push(conn.sourceNode);
                }
            }
        }

        const toRemove: number[] = [];
        for (const [id] of graph.nodes) {
            if (!reachable.has(id)) toRemove.push(id);
        }

        for (const id of toRemove) {
            graph.removeNode(id);
        }
        return toRemove.length;
    }

    private tryEvalConstant(node: ShaderNode): number | null {
        if (node.data.type === 'Constant' && typeof node.data.value === 'number') {
            return node.data.value;
        }
        return null;
    }

    private passConstantFolding(graph: ShaderGraph): number {
        let folded = 0;
        const candidates: Array<{ nodeId: number; result: number }> = [];

        for (const [id, node] of graph.nodes) {
            if (node.data.type !== 'Math') continue;

            let allConst = true;
            const vals: number[] = [];

            for (let i = 0; i < node.inputs.length; i++) {
                const conn = graph.findInputConnection(id, i);
                if (!conn) {
                    const d = node.inputs[i].defaultValue.data;
                    if (typeof d === 'number') { vals.push(d); continue; }
                    allConst = false; break;
                }
                const src = graph.getNode(conn.sourceNode);
                if (!src) { allConst = false; break; }
                const v = this.tryEvalConstant(src);
                if (v !== null) vals.push(v);
                else { allConst = false; break; }
            }

            if (!allConst || vals.length === 0) continue;
            const a = vals[0]; const b = vals[1] ?? 0;
            let result = 0;

            switch (node.data.op) {
                case MathOp.Add: result = a + b; break;
                case MathOp.Subtract: result = a - b; break;
                case MathOp.Multiply: result = a * b; break;
                case MathOp.Divide: result = Math.abs(b) > 0.0001 ? a / b : 0; break;
                case MathOp.Abs: result = Math.abs(a); break;
                case MathOp.Negate: result = -a; break;
                case MathOp.Floor: result = Math.floor(a); break;
                case MathOp.Ceil: result = Math.ceil(a); break;
                case MathOp.Min: result = Math.min(a, b); break;
                case MathOp.Max: result = Math.max(a, b); break;
                default: continue;
            }

            candidates.push({ nodeId: id, result });
        }

        for (const { nodeId, result } of candidates) {
            const node = graph.getNode(nodeId);
            if (!node) continue;
            node.data = { type: 'Constant', value: result, outputType: node.outputs[0]?.dataType ?? 'float' as any };
            node.name = `Const(${result})`;
            node.inputs = [];
            folded++;
        }
        return folded;
    }

    private passIdentityRemoval(graph: ShaderGraph): number {
        let removed = 0;
        const bypasses: Array<{ nodeId: number; inputIdx: number; zero: boolean }> = [];

        for (const [id, node] of graph.nodes) {
            if (node.data.type !== 'Math' || node.inputs.length < 2) continue;

            const checkConst = (idx: number): [boolean, number] => {
                const conn = graph.findInputConnection(id, idx);
                if (!conn) {
                    const d = node.inputs[idx]?.defaultValue.data;
                    if (typeof d === 'number') return [true, d];
                    return [false, 0];
                }
                const src = graph.getNode(conn.sourceNode);
                if (!src) return [false, 0];
                const v = this.tryEvalConstant(src);
                return v !== null ? [true, v] : [false, 0];
            };

            const [ac, av] = checkConst(0);
            const [bc, bv] = checkConst(1);

            switch (node.data.op) {
                case MathOp.Add:
                    if (bc && bv === 0) bypasses.push({ nodeId: id, inputIdx: 0, zero: false });
                    else if (ac && av === 0) bypasses.push({ nodeId: id, inputIdx: 1, zero: false });
                    break;
                case MathOp.Multiply:
                    if (bc && bv === 1) bypasses.push({ nodeId: id, inputIdx: 0, zero: false });
                    else if (ac && av === 1) bypasses.push({ nodeId: id, inputIdx: 1, zero: false });
                    else if (bc && bv === 0) bypasses.push({ nodeId: id, inputIdx: 0, zero: true });
                    else if (ac && av === 0) bypasses.push({ nodeId: id, inputIdx: 0, zero: true });
                    break;
                case MathOp.Divide:
                    if (bc && bv === 1) bypasses.push({ nodeId: id, inputIdx: 0, zero: false });
                    break;
            }
        }

        for (const bp of bypasses) {
            if (bp.zero) {
                const node = graph.getNode(bp.nodeId);
                if (!node) continue;
                node.data = { type: 'Constant', value: 0, outputType: node.outputs[0]?.dataType ?? 'float' as any };
                node.inputs = [];
                removed++;
            } else {
                const conn = graph.findInputConnection(bp.nodeId, bp.inputIdx);
                if (!conn) continue;
                const consumers: Array<[number, number]> = [];
                for (const c of graph.connections) {
                    if (c.sourceNode === bp.nodeId) consumers.push([c.targetNode, c.targetPort]);
                }
                for (const [tn, tp] of consumers) {
                    graph.connect(conn.sourceNode, conn.sourcePort, tn, tp);
                }
                graph.removeNode(bp.nodeId);
                removed++;
            }
        }
        return removed;
    }

    private passRemoveDeadAssignments(src: string): string {
        const lines = src.split('\n');
        const declPattern = /\s+(?:let|var)\s+(n\d+_p\d+\w*)/;
        const keep = lines.map(() => true);

        for (let i = 0; i < lines.length; i++) {
            const match = lines[i].match(declPattern);
            if (!match) continue;
            const varName = match[1];
            let used = false;
            for (let j = i + 1; j < lines.length; j++) {
                if (lines[j].includes(varName)) { used = true; break; }
            }
            if (!used) keep[i] = false;
        }

        return lines.filter((_, i) => keep[i]).join('\n');
    }
}
