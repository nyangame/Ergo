export interface State {
    enter(): void;
    update(dt: number): void;
    exit(): void;
    draw?(ctx: unknown): void;
}

export class StateMachine {
    private current: State | null = null;

    transition(state: State): void {
        if (this.current) {
            this.current.exit();
        }
        this.current = state;
        this.current.enter();
    }

    update(dt: number): void {
        if (this.current) {
            this.current.update(dt);
        }
    }

    draw(ctx: unknown): void {
        if (this.current && this.current.draw) {
            this.current.draw(ctx);
        }
    }
}
