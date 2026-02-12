import { Vec2, Size2, Color } from '../math/vec2.js';
import type { RenderContext, RendererBackend } from '../engine/interfaces.js';

export class WebGPURenderContext implements RenderContext {
    private ctx: CanvasRenderingContext2D | null = null;

    setContext(ctx: CanvasRenderingContext2D): void {
        this.ctx = ctx;
    }

    drawRect(pos: Vec2, size: Size2, color: Color, filled: boolean): void {
        if (!this.ctx) return;
        const style = `rgba(${color.r},${color.g},${color.b},${color.a / 255})`;
        if (filled) {
            this.ctx.fillStyle = style;
            this.ctx.fillRect(pos.x, pos.y, size.w, size.h);
        } else {
            this.ctx.strokeStyle = style;
            this.ctx.strokeRect(pos.x, pos.y, size.w, size.h);
        }
    }

    drawCircle(center: Vec2, radius: number, color: Color, filled: boolean): void {
        if (!this.ctx) return;
        const style = `rgba(${color.r},${color.g},${color.b},${color.a / 255})`;
        this.ctx.beginPath();
        this.ctx.arc(center.x, center.y, radius, 0, Math.PI * 2);
        if (filled) {
            this.ctx.fillStyle = style;
            this.ctx.fill();
        } else {
            this.ctx.strokeStyle = style;
            this.ctx.stroke();
        }
    }

    drawText(pos: Vec2, text: string, color: Color, scale: number): void {
        if (!this.ctx) return;
        this.ctx.fillStyle = `rgba(${color.r},${color.g},${color.b},${color.a / 255})`;
        this.ctx.font = `${16 * scale}px monospace`;
        this.ctx.fillText(text, pos.x, pos.y);
    }
}

export class WebGPURenderer implements RendererBackend {
    private canvas: HTMLCanvasElement | null = null;
    private ctx2d: CanvasRenderingContext2D | null = null;
    private renderContext: WebGPURenderContext = new WebGPURenderContext();

    async initialize(): Promise<boolean> {
        this.canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
        if (!this.canvas) {
            console.error('[Ergo] Canvas element #game-canvas not found');
            return false;
        }

        // Fallback to Canvas2D for now; WebGPU can be added later
        this.ctx2d = this.canvas.getContext('2d');
        if (!this.ctx2d) {
            console.error('[Ergo] Failed to get 2D context');
            return false;
        }

        this.renderContext.setContext(this.ctx2d);

        // TODO: Initialize WebGPU when available
        // const adapter = await navigator.gpu?.requestAdapter();
        // const device = await adapter?.requestDevice();

        return true;
    }

    beginFrame(): void {
        if (this.ctx2d && this.canvas) {
            this.ctx2d.clearRect(0, 0, this.canvas.width, this.canvas.height);
        }
    }

    endFrame(): void {
        // No-op for Canvas2D; for WebGPU this would submit command buffers
    }

    shutdown(): void {
        this.ctx2d = null;
        this.canvas = null;
    }

    context(): RenderContext {
        return this.renderContext;
    }
}
