import {
    ShaderNode, ShaderConnection, ShaderNodeData, ShaderDataType, ShaderPort
} from './shader_node.js';

// ============================================================
// ShaderGraph: DAG of shader nodes (TypeScript/WebGPU)
// ============================================================

export class ShaderGraph {
    private nodes_ = new Map<number, ShaderNode>();
    private connections_: ShaderConnection[] = [];
    private nextNodeId_ = 1;
    private nextConnId_ = 1;
    name: string;

    constructor(name = 'Untitled') {
        this.name = name;
    }

    get nodeCount(): number { return this.nodes_.size; }
    get connectionCount(): number { return this.connections_.length; }

    addNode(node: ShaderNode): number {
        const id = this.nextNodeId_++;
        node.id = id;
        this.nodes_.set(id, node);
        return id;
    }

    removeNode(id: number): void {
        this.nodes_.delete(id);
        this.connections_ = this.connections_.filter(
            c => c.sourceNode !== id && c.targetNode !== id
        );
    }

    getNode(id: number): ShaderNode | undefined {
        return this.nodes_.get(id);
    }

    get nodes(): Map<number, ShaderNode> { return this.nodes_; }
    get connections(): ShaderConnection[] { return this.connections_; }

    connect(srcNode: number, srcPort: number, dstNode: number, dstPort: number): number {
        if (!this.nodes_.has(srcNode) || !this.nodes_.has(dstNode)) return 0;

        // Replace existing connection to same input
        const existing = this.connections_.findIndex(
            c => c.targetNode === dstNode && c.targetPort === dstPort
        );
        if (existing >= 0) {
            this.connections_[existing].sourceNode = srcNode;
            this.connections_[existing].sourcePort = srcPort;
            return this.connections_[existing].id;
        }

        const id = this.nextConnId_++;
        this.connections_.push({ id, sourceNode: srcNode, sourcePort: srcPort,
                                 targetNode: dstNode, targetPort: dstPort });
        return id;
    }

    findInputConnection(nodeId: number, portIndex: number): ShaderConnection | undefined {
        return this.connections_.find(
            c => c.targetNode === nodeId && c.targetPort === portIndex
        );
    }

    findOutputNode(): number {
        for (const [id, node] of this.nodes_) {
            if (node.data.type === 'Output') return id;
        }
        return 0;
    }

    topologicalSort(): number[] {
        const inDegree = new Map<number, number>();
        const adjacency = new Map<number, number[]>();

        for (const [id] of this.nodes_) {
            inDegree.set(id, 0);
            adjacency.set(id, []);
        }

        for (const conn of this.connections_) {
            adjacency.get(conn.sourceNode)?.push(conn.targetNode);
            inDegree.set(conn.targetNode, (inDegree.get(conn.targetNode) ?? 0) + 1);
        }

        const queue: number[] = [];
        for (const [id, deg] of inDegree) {
            if (deg === 0) queue.push(id);
        }

        const sorted: number[] = [];
        while (queue.length > 0) {
            const current = queue.pop()!;
            sorted.push(current);
            for (const next of adjacency.get(current) ?? []) {
                const newDeg = (inDegree.get(next) ?? 1) - 1;
                inDegree.set(next, newDeg);
                if (newDeg === 0) queue.push(next);
            }
        }
        return sorted;
    }

    validate(): boolean {
        let outputCount = 0;
        for (const [, node] of this.nodes_) {
            if (node.data.type === 'Output') outputCount++;
        }
        if (outputCount !== 1) return false;

        const sorted = this.topologicalSort();
        return sorted.length === this.nodes_.size;
    }

    static typesCompatible(from: ShaderDataType, to: ShaderDataType): boolean {
        if (from === to) return true;
        if (from === ShaderDataType.Float &&
            (to === ShaderDataType.Vec2 || to === ShaderDataType.Vec3 || to === ShaderDataType.Vec4))
            return true;
        if (from === ShaderDataType.Vec3 && to === ShaderDataType.Vec4) return true;
        if (from === ShaderDataType.Vec4 && to === ShaderDataType.Vec3) return true;
        return false;
    }
}
