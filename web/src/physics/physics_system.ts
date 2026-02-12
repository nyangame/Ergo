import { Collider, ColliderTag } from './collider.js';
import { checkHit } from './hit_test.js';

export class PhysicsSystem {
    private colliders: Collider[][] = [];
    private calcStack: Collider[] = [];
    private removeList: { collider: Collider; tag: ColliderTag }[] = [];
    private nextId = 1;

    constructor() {
        for (let i = 0; i < ColliderTag.Max; i++) {
            this.colliders.push([]);
        }
    }

    registerCollider(c: Collider): void {
        c.handle = { id: this.nextId++ };
        const tagIdx = c.tag as number;
        if (tagIdx < this.colliders.length) {
            this.colliders[tagIdx].push(c);
        }
    }

    removeCollider(c: Collider): void {
        this.removeList.push({ collider: c, tag: c.tag });
    }

    markMoved(c: Collider): void {
        this.calcStack.push(c);
    }

    run(): void {
        // Process collision detection for moved objects
        for (const c of this.calcStack) {
            if (c.handle.id === 0) continue;

            for (let i = 0; i < ColliderTag.Max; i++) {
                const targetTag = i as ColliderTag;
                if (targetTag === c.tag) continue;
                if (targetTag === ColliderTag.Invalid) continue;
                this.hitToAll(c, targetTag);
            }
        }
        this.calcStack = [];

        // Process deferred removals
        for (const { collider, tag } of this.removeList) {
            const tagIdx = tag as number;
            if (tagIdx < this.colliders.length) {
                const idx = this.colliders[tagIdx].indexOf(collider);
                if (idx !== -1) {
                    this.colliders[tagIdx].splice(idx, 1);
                }
            }
            collider.handle = { id: 0 };
        }
        this.removeList = [];
    }

    private hitToAll(c: Collider, targetTag: ColliderTag): void {
        const tagIdx = targetTag as number;
        if (tagIdx >= this.colliders.length) return;

        for (const target of this.colliders[tagIdx]) {
            if (c === target) continue;
            if (c.handle.id === 0 || target.handle.id === 0) continue;

            if (checkHit(c, target)) {
                let consumed = false;
                if (c.onHit) {
                    consumed = c.onHit(target);
                }
                if (!consumed && target.onHit) {
                    target.onHit(c);
                }
            }
        }
    }
}

// Global instance (corresponds to C++ inline PhysicsSystem g_physics)
export const physics = new PhysicsSystem();
