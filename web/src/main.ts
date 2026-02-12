import { WebGPURenderer } from './renderer/webgpu_renderer.js';
import { TaskManager, RunPhase } from './engine/task_system.js';
import { physics } from './physics/physics_system.js';
import type { GameModule, EngineContext, RenderContext, InputBackend } from './engine/interfaces.js';
import { Vec2 } from './math/vec2.js';

// Simple keyboard input for web
class WebInput implements InputBackend {
    private keyCurrent: Set<number> = new Set();
    private keyPrevious: Set<number> = new Set();
    private mousePos: Vec2 = new Vec2();

    constructor() {
        document.addEventListener('keydown', (e) => {
            this.keyCurrent.add(e.keyCode);
        });
        document.addEventListener('keyup', (e) => {
            this.keyCurrent.delete(e.keyCode);
        });
        document.addEventListener('mousemove', (e) => {
            this.mousePos = new Vec2(e.clientX, e.clientY);
        });
    }

    pollEvents(): void {
        this.keyPrevious = new Set(this.keyCurrent);
    }

    isKeyDown(key: number): boolean {
        return this.keyCurrent.has(key);
    }

    isKeyPressed(key: number): boolean {
        return this.keyCurrent.has(key) && !this.keyPrevious.has(key);
    }

    mousePosition(): Vec2 {
        return this.mousePos;
    }
}

// Main entry point
async function main(): Promise<void> {
    // 1. Initialize renderer
    const renderer = new WebGPURenderer();
    if (!await renderer.initialize()) {
        console.error('[Ergo] Failed to initialize renderer');
        return;
    }

    // 2. Initialize input
    const input = new WebInput();

    // 3. Task manager
    const taskMgr = new TaskManager();

    // 4. Engine context
    const engineCtx: EngineContext = {
        renderer: renderer.context(),
        input: input,
    };

    // 5. Load game module (dynamic import)
    let gameModule: GameModule | null = null;
    try {
        const mod = await import('./game_module.js');
        gameModule = mod.default as GameModule;
        gameModule.onInit(engineCtx);
    } catch {
        console.warn('[Ergo] No game module found, running empty');
    }

    // 6. Main loop
    let lastTime = performance.now();

    function frame(): void {
        const now = performance.now();
        const dt = (now - lastTime) / 1000.0;
        lastTime = now;

        input.pollEvents();

        // Destroy phase
        taskMgr.run(RunPhase.Destroy, dt);

        // Update phase
        taskMgr.run(RunPhase.Update, dt);
        gameModule?.onUpdate(dt);

        // Physics
        physics.run();

        // Draw phase
        renderer.beginFrame();
        const ctx = renderer.context();
        taskMgr.run(RunPhase.Draw, dt, ctx);
        gameModule?.onDraw(ctx);
        renderer.endFrame();

        requestAnimationFrame(frame);
    }

    requestAnimationFrame(frame);
}

main().catch(console.error);
