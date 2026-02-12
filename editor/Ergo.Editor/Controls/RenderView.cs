using Ergo.Editor.Interop;

namespace Ergo.Editor.Controls;

/// <summary>
/// Base view that hosts a Vulkan rendering surface via the native engine.
/// Platform-specific handlers create a native child window and pass its
/// handle to the engine, which creates a VkSurfaceKHR for rendering.
///
/// SceneView and GameView derive from this, setting their respective
/// render modes and camera behaviors.
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
    /// Called by the handler once the native surface is created.
    /// Subclasses can override to set up initial camera, etc.
    /// </summary>
    internal virtual void OnNativeSurfaceCreated()
    {
    }

    /// <summary>
    /// Called by the handler when the native surface is resized.
    /// </summary>
    internal virtual void OnNativeSurfaceResized(uint width, uint height)
    {
        if (RenderTargetHandle.IsValid)
        {
            EngineInterop.ResizeRenderTarget(RenderTargetHandle, width, height);
        }
    }

    /// <summary>
    /// Called each frame by the render loop. Subclasses override to
    /// update camera, input, etc. before the frame is rendered.
    /// </summary>
    internal virtual void OnBeforeRender(float dt)
    {
    }

    /// <summary>
    /// Render one frame to this view's render target.
    /// </summary>
    public void RenderFrame(float dt)
    {
        if (!RenderTargetHandle.IsValid) return;
        OnBeforeRender(dt);
        EngineInterop.RenderFrame(RenderTargetHandle);
    }
}
