using Ergo.Editor.Interop;

namespace Ergo.Editor.Controls;

/// <summary>
/// Base view that hosts a Vulkan rendering surface via the native engine.
///
/// Lifecycle:
///   1. MAUI lays out this view and the handler creates a platform element.
///   2. When the platform element is loaded, the handler creates a Win32
///      child HWND and calls <see cref="EngineInterop.CreateRenderTarget"/>
///      to bind a VkSurfaceKHR to it.
///   3. <see cref="OnNativeSurfaceCreated"/> fires so subclasses can set
///      their initial camera.
///   4. The page's render-loop timer calls <see cref="RenderFrame"/> at
///      ~60 Hz, which invokes <see cref="OnBeforeRender"/> (camera/input
///      update) followed by the native render call.
///   5. On resize, <see cref="OnNativeSurfaceResized"/> is called; the
///      native side recreates the swapchain with the new extent.
///   6. On unload, the handler destroys the render target and child HWND.
///
/// SceneView and GameView derive from this.
/// </summary>
public class RenderView : View
{
    // -----------------------------------------------------------------
    // Bindable properties
    // -----------------------------------------------------------------

    public static readonly BindableProperty RenderModeProperty =
        BindableProperty.Create(
            nameof(RenderMode),
            typeof(ErgoRenderMode),
            typeof(RenderView),
            ErgoRenderMode.Scene);

    public ErgoRenderMode RenderMode
    {
        get => (ErgoRenderMode)GetValue(RenderModeProperty);
        set => SetValue(RenderModeProperty, value);
    }

    // -----------------------------------------------------------------
    // Render target state (managed by the handler)
    // -----------------------------------------------------------------

    internal ErgoRenderTargetHandle RenderTargetHandle { get; set; }

    /// <summary>
    /// True once the native surface has been created and is ready
    /// to accept render calls.
    /// </summary>
    public bool IsSurfaceReady => RenderTargetHandle.IsValid;

    /// <summary>
    /// Fired once the native surface is ready. Subclasses should set
    /// their initial camera here.
    /// </summary>
    internal virtual void OnNativeSurfaceCreated()
    {
    }

    /// <summary>
    /// Fired when the hosting element is resized. Forwards the new
    /// size to the native swapchain so it can be recreated.
    /// </summary>
    internal virtual void OnNativeSurfaceResized(uint width, uint height)
    {
        if (RenderTargetHandle.IsValid)
        {
            EngineInterop.ResizeRenderTarget(RenderTargetHandle, width, height);
        }
    }

    /// <summary>
    /// Called each frame before the native render call. Subclasses
    /// override to update camera, input state, etc.
    /// </summary>
    internal virtual void OnBeforeRender(float dt)
    {
    }

    /// <summary>
    /// Perform one full render frame: pre-render update → native draw
    /// → present. Called by the page's render-loop timer.
    /// </summary>
    public void RenderFrame(float dt)
    {
        if (!RenderTargetHandle.IsValid) return;
        OnBeforeRender(dt);
        EngineInterop.RenderFrame(RenderTargetHandle);
    }
}
