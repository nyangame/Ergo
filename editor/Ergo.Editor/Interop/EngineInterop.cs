using System.Runtime.InteropServices;

namespace Ergo.Editor.Interop;

/// <summary>
/// P/Invoke bindings to the native editor API (ergo_editor_native shared library).
/// </summary>
public static partial class EngineInterop
{
    private const string LibName = "ergo_editor_native";

    // -----------------------------------------------------------------
    // Engine lifecycle
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_editor_init")]
    [return: MarshalAs(UnmanagedType.I4)]
    public static partial int Init();

    [LibraryImport(LibName, EntryPoint = "ergo_editor_shutdown")]
    public static partial void Shutdown();

    [LibraryImport(LibName, EntryPoint = "ergo_editor_tick")]
    public static partial void Tick(float dt);

    // -----------------------------------------------------------------
    // Render target management
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_editor_create_render_target")]
    public static partial ErgoRenderTargetHandle CreateRenderTarget(
        IntPtr nativeWindowHandle, uint width, uint height,
        ErgoRenderMode mode);

    [LibraryImport(LibName, EntryPoint = "ergo_editor_destroy_render_target")]
    public static partial void DestroyRenderTarget(ErgoRenderTargetHandle handle);

    [LibraryImport(LibName, EntryPoint = "ergo_editor_resize_render_target")]
    public static partial void ResizeRenderTarget(
        ErgoRenderTargetHandle handle, uint width, uint height);

    [LibraryImport(LibName, EntryPoint = "ergo_editor_render_frame")]
    public static partial void RenderFrame(ErgoRenderTargetHandle handle);

    // -----------------------------------------------------------------
    // Camera
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_editor_set_camera")]
    public static partial void SetCamera(
        ErgoRenderTargetHandle handle,
        ErgoVec3 eye, ErgoVec3 target, ErgoVec3 up,
        float fovDegrees, float nearPlane, float farPlane);

    // -----------------------------------------------------------------
    // Scene query
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_editor_get_object_count")]
    public static partial uint GetObjectCount();

    [LibraryImport(LibName, EntryPoint = "ergo_editor_get_objects")]
    public static partial uint GetObjects(
        [Out] ErgoGameObjectHandle[] outHandles, uint maxCount);

    [LibraryImport(LibName, EntryPoint = "ergo_editor_get_object_name")]
    public static partial IntPtr GetObjectNamePtr(ErgoGameObjectHandle handle);

    public static string GetObjectName(ErgoGameObjectHandle handle)
    {
        var ptr = GetObjectNamePtr(handle);
        return Marshal.PtrToStringAnsi(ptr) ?? "";
    }

    [LibraryImport(LibName, EntryPoint = "ergo_editor_get_object_transform")]
    public static partial ErgoTransform3D GetObjectTransform(
        ErgoGameObjectHandle handle);

    [LibraryImport(LibName, EntryPoint = "ergo_editor_set_object_transform")]
    public static partial void SetObjectTransform(
        ErgoGameObjectHandle handle, ErgoTransform3D transform);

    // -----------------------------------------------------------------
    // Component query
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_editor_get_component_count")]
    public static partial uint GetComponentCount(ErgoGameObjectHandle objectHandle);

    [LibraryImport(LibName, EntryPoint = "ergo_editor_get_components")]
    public static partial uint GetComponents(
        ErgoGameObjectHandle objectHandle,
        [Out] ErgoComponentInfo[] outInfos, uint maxCount);

    // -----------------------------------------------------------------
    // Object picking
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_editor_pick_object")]
    public static partial ErgoGameObjectHandle PickObject(
        ErgoRenderTargetHandle renderTarget,
        float screenX, float screenY);

    // =================================================================
    // UI Editor Hierarchy
    // =================================================================

    // -----------------------------------------------------------------
    // Canvas management
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_ui_create_canvas",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial ErgoUINodeHandle UICreateCanvas(string name);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_remove_canvas")]
    public static partial void UIRemoveCanvas(ErgoUINodeHandle handle);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_get_canvas_count")]
    public static partial uint UIGetCanvasCount();

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_canvas_scale_mode")]
    public static partial void UISetCanvasScaleMode(
        ErgoUINodeHandle canvas, ErgoUIScaleMode mode);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_get_canvas_scale_mode")]
    public static partial ErgoUIScaleMode UIGetCanvasScaleMode(
        ErgoUINodeHandle canvas);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_canvas_reference_resolution")]
    public static partial void UISetCanvasReferenceResolution(
        ErgoUINodeHandle canvas, float width, float height);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_canvas_screen_match_mode")]
    public static partial void UISetCanvasScreenMatchMode(
        ErgoUINodeHandle canvas, ErgoUIScreenMatchMode mode);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_canvas_screen_size")]
    public static partial void UISetCanvasScreenSize(
        ErgoUINodeHandle canvas, float width, float height);

    // -----------------------------------------------------------------
    // Node creation / destruction
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_ui_create_node",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial ErgoUINodeHandle UICreateNode(
        ErgoUINodeHandle parent, string name);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_create_image_node",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial ErgoUINodeHandle UICreateImageNode(
        ErgoUINodeHandle parent, string name, string texturePath);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_remove_node")]
    public static partial void UIRemoveNode(ErgoUINodeHandle handle);

    // -----------------------------------------------------------------
    // Node properties
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_ui_get_node_name")]
    private static partial IntPtr UIGetNodeNamePtr(ErgoUINodeHandle handle);

    public static string UIGetNodeName(ErgoUINodeHandle handle)
    {
        var ptr = UIGetNodeNamePtr(handle);
        return Marshal.PtrToStringAnsi(ptr) ?? "";
    }

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_node_name",
                   StringMarshalling = StringMarshalling.Utf8)]
    public static partial void UISetNodeName(
        ErgoUINodeHandle handle, string name);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_get_rect_transform")]
    public static partial ErgoUIRectTransform UIGetRectTransform(
        ErgoUINodeHandle handle);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_rect_transform")]
    public static partial void UISetRectTransform(
        ErgoUINodeHandle handle, ErgoUIRectTransform rect);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_node_active")]
    public static partial void UISetNodeActive(
        ErgoUINodeHandle handle, int active);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_node_visible")]
    public static partial void UISetNodeVisible(
        ErgoUINodeHandle handle, int visible);

    // -----------------------------------------------------------------
    // Hierarchy query
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_ui_get_hierarchy_count")]
    public static partial uint UIGetHierarchyCount();

    [LibraryImport(LibName, EntryPoint = "ergo_ui_get_hierarchy")]
    public static partial uint UIGetHierarchy(
        [Out] ErgoUINodeInfo[] outInfos, uint maxCount);

    // -----------------------------------------------------------------
    // Hierarchy manipulation
    // -----------------------------------------------------------------

    [LibraryImport(LibName, EntryPoint = "ergo_ui_reparent")]
    public static partial void UIReparent(
        ErgoUINodeHandle node, ErgoUINodeHandle newParent);

    [LibraryImport(LibName, EntryPoint = "ergo_ui_set_sibling_index")]
    public static partial void UISetSiblingIndex(
        ErgoUINodeHandle node, int index);
}
