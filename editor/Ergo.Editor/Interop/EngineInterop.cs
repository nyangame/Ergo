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
}
