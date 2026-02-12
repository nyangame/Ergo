import { Vec2 } from '../math/vec2.js';
import { Transform2D } from '../math/transform.js';

export enum ColliderTag {
    Invalid = 0,
    Player,
    Enemy,
    Bullet,
    Max,
}

export interface AABBData {
    type: 'aabb';
    halfExtent: Vec2;
}

export interface CircleData {
    type: 'circle';
    radius: number;
}

export type ColliderShape = AABBData | CircleData;

export interface ColliderHandle {
    id: number;
}

export interface Collider {
    handle: ColliderHandle;
    shape: ColliderShape;
    tag: ColliderTag;
    ownerId: number;
    transform: Transform2D;
    onHit?: (target: Collider) => boolean;
}
