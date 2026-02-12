import { Vec2, Size2, Color } from '../math/vec2.js';

// Corresponds to C++ concept definitions

export interface TaskLike {
    start(): void;
    update(dt: number): void;
    release(): void;
}

export interface Drawable {
    draw(ctx: RenderContext): void;
}

export interface GameObjectLike {
    transform(): Transform2D;
    name(): string;
    objectType(): number;
}

export interface RenderContext {
    drawRect(pos: Vec2, size: Size2, color: Color, filled: boolean): void;
    drawCircle(center: Vec2, radius: number, color: Color, filled: boolean): void;
    drawText(pos: Vec2, text: string, color: Color, scale: number): void;
}

export interface RendererBackend {
    initialize(): Promise<boolean>;
    beginFrame(): void;
    endFrame(): void;
    shutdown(): void;
}

export interface InputBackend {
    isKeyDown(key: number): boolean;
    isKeyPressed(key: number): boolean;
    mousePosition(): Vec2;
    pollEvents(): void;
}

export interface GameModule {
    onInit(ctx: EngineContext): void;
    onUpdate(dt: number): void;
    onDraw(ctx: RenderContext): void;
    onShutdown(): void;
}

export interface EngineContext {
    renderer: RenderContext;
    input: InputBackend;
}

// Shader composition concepts (matches C++ ShaderComposable / ShaderOptimizable)
export interface ShaderComposable {
    compile(): boolean;
    vertexSource(): string;
    fragmentSource(): string;
    isCompiled(): boolean;
}

export interface ShaderOptimizable {
    optimizationReport(): string;
    isOptimized(): boolean;
}

// Re-export for convenience
import { Transform2D } from '../math/transform.js';
export { Transform2D };
