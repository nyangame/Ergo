using System.Runtime.InteropServices;

namespace Ergo.Editor.Interop;

/// <summary>
/// P/Invoke bindings for the native plugin API (plugin_api.h).
/// Provides access to the plugin registry and per-instance property
/// read/write on live behaviour components.
/// </summary>
public static partial class PluginInterop
{
    private const string LibName = "ergo_editor_native";

    // =================================================================
    // Plugin registry queries
    // =================================================================

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_count")]
    public static partial uint GetPluginCount();

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_all")]
    public static partial uint GetAllPlugins(
        [Out] ErgoPluginInfo[] outInfos, uint maxCount);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_info",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int GetPluginInfo(
        string typeName, out ErgoPluginInfo outInfo);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_by_category",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial uint GetPluginsByCategory(
        string category,
        [Out] ErgoPluginInfo[] outInfos, uint maxCount);

    // =================================================================
    // Plugin property queries
    // =================================================================

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_properties",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial uint GetPluginProperties(
        string typeName,
        [Out] ErgoPluginPropertyInfo[] outProps, uint maxCount);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_enum_entries",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial uint GetPluginEnumEntries(
        string typeName,
        string propertyName,
        [Out] ErgoPluginEnumEntry[] outEntries, uint maxCount);

    // =================================================================
    // Plugin property read/write on live behaviour instances
    // =================================================================

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_float",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int GetFloat(
        ErgoGameObjectHandle objectHandle,
        string typeName,
        string propertyName,
        out float outValue);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_set_float",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int SetFloat(
        ErgoGameObjectHandle objectHandle,
        string typeName,
        string propertyName,
        float value);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_get_int",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int GetInt(
        ErgoGameObjectHandle objectHandle,
        string typeName,
        string propertyName,
        out int outValue);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_set_int",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int SetInt(
        ErgoGameObjectHandle objectHandle,
        string typeName,
        string propertyName,
        int value);

    // =================================================================
    // Plugin instance management
    // =================================================================

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_add_to_object",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int AddPluginToObject(
        ErgoGameObjectHandle objectHandle,
        string typeName);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_remove_from_object",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int RemovePluginFromObject(
        ErgoGameObjectHandle objectHandle,
        string typeName);

    [LibraryImport(LibName, EntryPoint = "ergo_plugin_object_has",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial int ObjectHasPlugin(
        ErgoGameObjectHandle objectHandle,
        string typeName);
}
