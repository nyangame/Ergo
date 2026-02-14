using System.Runtime.InteropServices;

namespace Ergo.Editor.Interop;

// ---------------------------------------------------------------------------
// Mirror types from engine_types.h / editor_api.h
// ---------------------------------------------------------------------------

[StructLayout(LayoutKind.Sequential)]
public struct ErgoVec2
{
    public float X, Y;
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoVec3
{
    public float X, Y, Z;

    public ErgoVec3(float x, float y, float z)
    {
        X = x; Y = y; Z = z;
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoQuat
{
    public float X, Y, Z, W;

    public ErgoQuat(float x, float y, float z, float w)
    {
        X = x; Y = y; Z = z; W = w;
    }

    public static ErgoQuat Identity => new(0, 0, 0, 1);
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoSize2
{
    public float W, H;
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoColor
{
    public byte R, G, B, A;

    public ErgoColor(byte r, byte g, byte b, byte a = 255)
    {
        R = r; G = g; B = b; A = a;
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoTransform3D
{
    public ErgoVec3 Position;
    public ErgoQuat Rotation;
    public ErgoVec3 Scale;

    public static ErgoTransform3D Identity => new()
    {
        Position = new ErgoVec3(0, 0, 0),
        Rotation = ErgoQuat.Identity,
        Scale = new ErgoVec3(1, 1, 1),
    };
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoRenderTargetHandle
{
    public ulong Id;
    public bool IsValid => Id != 0;
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoGameObjectHandle
{
    public ulong Id;
    public bool IsValid => Id != 0;
}

public enum ErgoRenderMode
{
    Scene = 0,
    Game = 1,
}

public enum ErgoPropertyType
{
    Float = 0,
    Int = 1,
    Bool = 2,
    Vec3 = 3,
    String = 4,
    Color = 5,
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoComponentInfo
{
    public IntPtr Name;       // const char*
    public IntPtr TypeName;   // const char*
    public uint PropertyCount;

    public string NameStr => Marshal.PtrToStringAnsi(Name) ?? "";
    public string TypeNameStr => Marshal.PtrToStringAnsi(TypeName) ?? "";
}

// ---------------------------------------------------------------------------
// UI Editor Hierarchy types
// ---------------------------------------------------------------------------

[StructLayout(LayoutKind.Sequential)]
public struct ErgoUINodeHandle
{
    public ulong Id;
    public bool IsValid => Id != 0;
}

public enum ErgoUIScaleMode
{
    DotByDot = 0,
    ScaleWithScreen = 1,
}

public enum ErgoUIScreenMatchMode
{
    MatchWidth = 0,
    MatchHeight = 1,
    MatchMinAxis = 2,
    MatchMaxAxis = 3,
}

public enum ErgoUINodeType
{
    Base = 0,
    Canvas = 1,
    Image = 2,
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoUIRectTransform
{
    public ErgoVec2 AnchorMin;
    public ErgoVec2 AnchorMax;
    public ErgoVec2 Pivot;
    public ErgoVec2 Position;
    public ErgoSize2 SizeDelta;
}

[StructLayout(LayoutKind.Sequential)]
public struct ErgoUINodeInfo
{
    public ErgoUINodeHandle Handle;
    public ErgoUINodeHandle Parent;
    public ErgoUINodeType NodeType;
    public IntPtr Name;         // const char*
    public int Depth;
    public int ChildCount;
    public int Active;
    public int Visible;

    public string NameStr => Marshal.PtrToStringAnsi(Name) ?? "";
}
