import { Transform2D } from '../math/transform.js';

export class GameObject {
    id: number = 0;
    name: string = '';
    objectType: number = 0;
    transform: Transform2D = new Transform2D();
    private components: Map<string, unknown> = new Map();

    addComponent<T>(key: string, component: T): void {
        this.components.set(key, component);
    }

    getComponent<T>(key: string): T | undefined {
        return this.components.get(key) as T | undefined;
    }
}
