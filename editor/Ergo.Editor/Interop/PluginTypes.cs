using System.Runtime.InteropServices;

namespace Ergo.Editor.Interop;

/// <summary>
/// Property types that the editor can display and edit.
/// Mirrors ErgoPluginPropertyType in plugin_api.h.
/// </summary>
public enum ErgoPluginPropertyType
{
    Float  = 0,
    Int    = 1,
    Bool   = 2,
    Vec2   = 3,
    Vec3   = 4,
    Color  = 5,
    String = 6,
    Enum   = 7,
    Asset  = 8,
}

/// <summary>
/// Plugin metadata returned by the native plugin registry.
/// Mirrors ErgoPluginInfo in plugin_api.h.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ErgoPluginInfo
{
    public IntPtr TypeNamePtr;
    public IntPtr DisplayNamePtr;
    public IntPtr CategoryPtr;
    public IntPtr DescriptionPtr;
    public uint   PropertyCount;
    public int    AllowMultiple;
    public int    Removable;
    public int    VisibleInAddMenu;

    public string TypeName    => Marshal.PtrToStringAnsi(TypeNamePtr) ?? "";
    public string DisplayName => Marshal.PtrToStringAnsi(DisplayNamePtr) ?? "";
    public string Category    => Marshal.PtrToStringAnsi(CategoryPtr) ?? "";
    public string Description => Marshal.PtrToStringAnsi(DescriptionPtr) ?? "";
}

/// <summary>
/// Property descriptor returned by the native plugin registry.
/// Mirrors ErgoPluginPropertyInfo in plugin_api.h.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ErgoPluginPropertyInfo
{
    public IntPtr                  NamePtr;
    public IntPtr                  DisplayNamePtr;
    public ErgoPluginPropertyType  Type;
    public float                   RangeMin;
    public float                   RangeMax;
    public float                   RangeStep;
    public int                     HasRange;
    public IntPtr                  TooltipPtr;
    public uint                    EnumEntryCount;

    public string Name        => Marshal.PtrToStringAnsi(NamePtr) ?? "";
    public string DisplayName => Marshal.PtrToStringAnsi(DisplayNamePtr) ?? "";
    public string? Tooltip    => Marshal.PtrToStringAnsi(TooltipPtr);
}

/// <summary>
/// Enum entry for enum-type properties.
/// Mirrors ErgoPluginEnumEntry in plugin_api.h.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ErgoPluginEnumEntry
{
    public IntPtr LabelPtr;
    public int    Value;

    public string Label => Marshal.PtrToStringAnsi(LabelPtr) ?? "";
}
