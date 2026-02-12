// ============================================================
// Shader composition node system (TypeScript/WebGPU)
// Mirrors the C++ implementation for web platform
// ============================================================

export enum ShaderDataType {
    Float = 'float',
    Vec2 = 'vec2',
    Vec3 = 'vec3',
    Vec4 = 'vec4',
    Mat3 = 'mat3',
    Mat4 = 'mat4',
    Texture2D = 'texture2D',
    Sampler = 'sampler',
    Bool = 'bool',
}

export type ShaderValueData = number | [number, number] | [number, number, number] | [number, number, number, number] | boolean;

export interface ShaderValue {
    data: ShaderValueData;
}

export interface ShaderPort {
    id: number;
    name: string;
    dataType: ShaderDataType;
    defaultValue: ShaderValue;
}

export interface ShaderConnection {
    id: number;
    sourceNode: number;
    sourcePort: number;
    targetNode: number;
    targetPort: number;
}

// ============================================================
// Node operation enums
// ============================================================

export enum MathOp {
    Add, Subtract, Multiply, Divide,
    Power, SquareRoot, Abs,
    Min, Max, Clamp, Lerp,
    Dot, Cross, Normalize, Length,
    Negate, Fract, Floor, Ceil,
    Step, SmoothStep,
}

export enum TrigOp {
    Sin, Cos, Tan, Asin, Acos, Atan, Atan2,
}

export enum LightModel {
    Lambert, BlinnPhong, CookTorrance, Toon, Unlit,
}

export enum BlendMode {
    Normal, Additive, Multiply, Screen, Overlay,
}

// ============================================================
// Node data types (discriminated union)
// ============================================================

export type ShaderNodeData =
    | { type: 'PropertyFloat'; value: number; uniformName: string }
    | { type: 'PropertyVec4'; value: [number, number, number, number]; uniformName: string }
    | { type: 'Math'; op: MathOp }
    | { type: 'Trig'; op: TrigOp }
    | { type: 'TextureSample'; textureUniform: string }
    | { type: 'Time'; speed: number }
    | { type: 'Normal'; worldSpace: boolean }
    | { type: 'Position'; worldSpace: boolean }
    | { type: 'ViewDirection'; worldSpace: boolean }
    | { type: 'Lighting'; model: LightModel }
    | { type: 'Combine'; outputType: ShaderDataType }
    | { type: 'Split' }
    | { type: 'Fresnel'; power: number }
    | { type: 'Blend'; mode: BlendMode }
    | { type: 'Constant'; value: ShaderValueData; outputType: ShaderDataType }
    | { type: 'Custom'; wgslCode: string }
    | { type: 'Output' };

export interface ShaderNode {
    id: number;
    name: string;
    data: ShaderNodeData;
    inputs: ShaderPort[];
    outputs: ShaderPort[];
    posX: number;
    posY: number;
}
