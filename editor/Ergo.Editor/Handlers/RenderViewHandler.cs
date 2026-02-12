using Ergo.Editor.Controls;
using Ergo.Editor.Interop;

#if WINDOWS
using Microsoft.Maui.Handlers;
using Microsoft.UI.Xaml.Controls;
using WinRT.Interop;
#endif

namespace Ergo.Editor.Handlers;

/// <summary>
/// Cross-platform handler for RenderView.
///
/// On Windows, it creates a SwapChainPanel (or a native HWND child) and
/// passes the window handle to the engine so Vulkan can create a surface.
/// </summary>
public partial class RenderViewHandler : ViewHandler<RenderView,
#if WINDOWS
    Panel
#else
    object
#endif
>
{
    public static IPropertyMapper<RenderView, RenderViewHandler> Mapper =
        new PropertyMapper<RenderView, RenderViewHandler>(ViewHandler.ViewMapper)
        {
            [nameof(RenderView.RenderMode)] = MapRenderMode,
        };

    public RenderViewHandler() : base(Mapper)
    {
    }

    private static void MapRenderMode(RenderViewHandler handler, RenderView view)
    {
        // Render mode is read when creating the render target.
        // Changing at runtime would require recreating the target.
    }

#if WINDOWS
    protected override Panel CreatePlatformView()
    {
        // Use a simple Panel as the host. The render target is created
        // once the panel has a valid HWND.
        var panel = new Canvas();
        return panel;
    }

    protected override void ConnectHandler(Panel platformView)
    {
        base.ConnectHandler(platformView);

        // Defer render target creation until the panel is loaded and has a size.
        platformView.Loaded += OnPanelLoaded;
        platformView.SizeChanged += OnPanelSizeChanged;
    }

    protected override void DisconnectHandler(Panel platformView)
    {
        platformView.Loaded -= OnPanelLoaded;
        platformView.SizeChanged -= OnPanelSizeChanged;

        if (VirtualView.RenderTargetHandle.IsValid)
        {
            EngineInterop.DestroyRenderTarget(VirtualView.RenderTargetHandle);
            VirtualView.RenderTargetHandle = default;
        }

        base.DisconnectHandler(platformView);
    }

    private void OnPanelLoaded(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
    {
        CreateRenderTarget();
    }

    private void OnPanelSizeChanged(object sender, Microsoft.UI.Xaml.SizeChangedEventArgs e)
    {
        var w = (uint)e.NewSize.Width;
        var h = (uint)e.NewSize.Height;
        if (w > 0 && h > 0)
        {
            VirtualView?.OnNativeSurfaceResized(w, h);
        }
    }

    private void CreateRenderTarget()
    {
        if (PlatformView == null || VirtualView == null) return;

        var hwnd = WindowNative.GetWindowHandle(
            PlatformView.XamlRoot?.ContentIslandEnvironment?.AppWindowId
            ?? default);

        // Fallback: get HWND from the hosting window
        if (hwnd == IntPtr.Zero)
        {
            var window = PlatformView.XamlRoot?.Content?.XamlRoot;
            // In MAUI, we get the HWND from the MauiWinUIWindow
            // This is a simplified approach
        }

        var w = (uint)Math.Max(PlatformView.ActualWidth, 1);
        var h = (uint)Math.Max(PlatformView.ActualHeight, 1);

        var handle = EngineInterop.CreateRenderTarget(
            hwnd, w, h, VirtualView.RenderMode);

        VirtualView.RenderTargetHandle = handle;
        VirtualView.OnNativeSurfaceCreated();
    }
#else
    protected override object CreatePlatformView()
    {
        // Stub for non-Windows platforms.
        // macOS/Linux implementations would go here.
        return new object();
    }
#endif
}
