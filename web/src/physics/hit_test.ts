import { Vec2 } from '../math/vec2.js';
import { Transform2D } from '../math/transform.js';
import type { AABBData, CircleData, Collider, ColliderShape } from './collider.js';

function isHitCircle(a: Vec2, b: Vec2, r: number): boolean {
    const dx = a.x - b.x;
    const dy = a.y - b.y;
    return dx * dx + dy * dy < r * r;
}

function hitTestAABBvsAABB(a: AABBData, ta: Transform2D, b: AABBData, tb: Transform2D): boolean {
    const xa1 = ta.position.x - a.halfExtent.x;
    const xa2 = ta.position.x + a.halfExtent.x;
    const ya1 = ta.position.y - a.halfExtent.y;
    const ya2 = ta.position.y + a.halfExtent.y;
    const xb1 = tb.position.x - b.halfExtent.x;
    const xb2 = tb.position.x + b.halfExtent.x;
    const yb1 = tb.position.y - b.halfExtent.y;
    const yb2 = tb.position.y + b.halfExtent.y;
    return xa1 <= xb2 && xa2 >= xb1 && ya1 <= yb2 && ya2 >= yb1;
}

function hitTestCirclevsCircle(a: CircleData, ta: Transform2D, b: CircleData, tb: Transform2D): boolean {
    return isHitCircle(ta.position, tb.position, a.radius + b.radius);
}

function hitTestCirclevsAABB(circle: CircleData, tc: Transform2D, aabb: AABBData, ta: Transform2D): boolean {
    const cx = tc.position.x;
    const cy = tc.position.y;
    const r = circle.radius;

    const x1 = ta.position.x - aabb.halfExtent.x;
    const x2 = ta.position.x + aabb.halfExtent.x;
    const y1 = ta.position.y - aabb.halfExtent.y;
    const y2 = ta.position.y + aabb.halfExtent.y;

    if (x1 - r < cx && cx < x2 + r && y1 < cy && cy < y2) return true;
    if (x1 < cx && cx < x2 && y1 - r < cy && cy < y2 + r) return true;

    const pos = tc.position;
    if (isHitCircle(new Vec2(x1, y1), pos, r)) return true;
    if (isHitCircle(new Vec2(x2, y1), pos, r)) return true;
    if (isHitCircle(new Vec2(x1, y2), pos, r)) return true;
    if (isHitCircle(new Vec2(x2, y2), pos, r)) return true;

    return false;
}

function hitTestShapes(sa: ColliderShape, ta: Transform2D, sb: ColliderShape, tb: Transform2D): boolean {
    if (sa.type === 'aabb' && sb.type === 'aabb') return hitTestAABBvsAABB(sa, ta, sb, tb);
    if (sa.type === 'circle' && sb.type === 'circle') return hitTestCirclevsCircle(sa, ta, sb, tb);
    if (sa.type === 'circle' && sb.type === 'aabb') return hitTestCirclevsAABB(sa, ta, sb, tb);
    if (sa.type === 'aabb' && sb.type === 'circle') return hitTestCirclevsAABB(sb, tb, sa, ta);
    return false;
}

export function checkHit(a: Collider, b: Collider): boolean {
    return hitTestShapes(a.shape, a.transform, b.shape, b.transform);
}
