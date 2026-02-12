import type { TaskLike, Drawable, RenderContext } from './interfaces.js';

export interface TaskHandle {
    id: number;
}

export enum TaskLayer {
    Default = 0,
    Bullet = 1,
    UI = 2,
    Max = 3,
}

export enum RunPhase {
    Start,
    Update,
    Physics,
    Draw,
    Destroy,
}

interface TaskEntry {
    id: number;
    task: TaskLike & Partial<Drawable>;
    initialized: boolean;
    pendingDestroy: boolean;
}

export class TaskManager {
    private layers: TaskEntry[][] = [];
    private nextId = 1;

    constructor() {
        for (let i = 0; i < TaskLayer.Max; i++) {
            this.layers.push([]);
        }
    }

    registerTask(task: TaskLike & Partial<Drawable>, layer: TaskLayer = TaskLayer.Default): TaskHandle {
        const id = this.nextId++;
        this.layers[layer].push({ id, task, initialized: false, pendingDestroy: false });
        return { id };
    }

    destroy(handle: TaskHandle): void {
        for (const layer of this.layers) {
            for (const entry of layer) {
                if (entry.id === handle.id) {
                    entry.pendingDestroy = true;
                    return;
                }
            }
        }
    }

    run(phase: RunPhase, dt: number, ctx?: RenderContext): void {
        for (const layer of this.layers) {
            switch (phase) {
                case RunPhase.Start:
                    for (const entry of layer) {
                        if (!entry.initialized && !entry.pendingDestroy) {
                            entry.task.start();
                            entry.initialized = true;
                        }
                    }
                    break;

                case RunPhase.Update:
                    for (const entry of layer) {
                        if (!entry.initialized && !entry.pendingDestroy) {
                            entry.task.start();
                            entry.initialized = true;
                        }
                    }
                    for (const entry of layer) {
                        if (entry.initialized && !entry.pendingDestroy) {
                            entry.task.update(dt);
                        }
                    }
                    break;

                case RunPhase.Draw:
                    if (ctx) {
                        for (const entry of layer) {
                            if (entry.initialized && !entry.pendingDestroy && entry.task.draw) {
                                entry.task.draw(ctx);
                            }
                        }
                    }
                    break;

                case RunPhase.Destroy:
                    for (let i = layer.length - 1; i >= 0; i--) {
                        if (layer[i].pendingDestroy) {
                            if (layer[i].initialized) {
                                layer[i].task.release();
                            }
                            layer.splice(i, 1);
                        }
                    }
                    break;
            }
        }
    }
}
