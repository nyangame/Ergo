export class Vec2 {
    constructor(
        public x: number = 0,
        public y: number = 0
    ) {}

    add(o: Vec2): Vec2 { return new Vec2(this.x + o.x, this.y + o.y); }
    sub(o: Vec2): Vec2 { return new Vec2(this.x - o.x, this.y - o.y); }
    scale(s: number): Vec2 { return new Vec2(this.x * s, this.y * s); }

    addAssign(o: Vec2): this { this.x += o.x; this.y += o.y; return this; }
    subAssign(o: Vec2): this { this.x -= o.x; this.y -= o.y; return this; }
    scaleAssign(s: number): this { this.x *= s; this.y *= s; return this; }

    lengthSq(): number { return this.x * this.x + this.y * this.y; }
    length(): number { return Math.sqrt(this.lengthSq()); }
    normalized(): Vec2 {
        const l = this.length();
        return l > 0 ? new Vec2(this.x / l, this.y / l) : new Vec2();
    }

    static zero(): Vec2 { return new Vec2(0, 0); }
}

export class Size2 {
    constructor(
        public w: number = 0,
        public h: number = 0
    ) {}

    halfW(): number { return this.w * 0.5; }
    halfH(): number { return this.h * 0.5; }
    radius(): number { return this.w * 0.5; }
}

export class Color {
    constructor(
        public r: number = 255,
        public g: number = 255,
        public b: number = 255,
        public a: number = 255
    ) {}
}
