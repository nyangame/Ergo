using Ergo.Editor.Controls;
using Ergo.Editor.Interop;
using Microsoft.Maui.Handlers;

#if WINDOWS
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
#endif

namespace Ergo.Editor.Handlers;

/// <summary>
/// Platform handler for <see cref="RenderView"/>.
///
/// On Windows the handler creates a WinUI <see cref="Border"/> as the
/// host element. Once the element is loaded and has a valid HWND tree,
/// it creates a Win32 child window via <see cref="Win32.CreateRenderChildWindow"/>
/// and passes that HWND to the native engine so a VkSurfaceKHR can be
/// created for Vulkan rendering.
/// </summary>
public partial class RenderViewHandler : ViewHandler<RenderView,
#if WINDOWS
    Border
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
        // Mode is set at render-target creation time.
    }

#if WINDOWS

    private IntPtr _childHwnd;

    protected override Border CreatePlatformView()
    {
        return new Border
        {
            Background = new Microsoft.UI.Xaml.Media.SolidColorBrush(
                Microsoft.UI.Colors.Black),
        };
    }

    protected override void ConnectHandler(Border platformView)
    {
        base.ConnectHandler(platformView);
        platformView.Loaded += OnHostLoaded;
        platformView.SizeChanged += OnHostSizeChanged;
        platformView.Unloaded += OnHostUnloaded;
    }

    protected override void DisconnectHandler(Border platformView)
    {
        platformView.Loaded -= OnHostLoaded;
        platformView.SizeChanged -= OnHostSizeChanged;
        platformView.Unloaded -= OnHostUnloaded;
        DestroyRenderTarget();
        base.DisconnectHandler(platformView);
    }

    // ---------------------------------------------------------------
    // Event handlers
    // ---------------------------------------------------------------

    private void OnHostLoaded(object sender, RoutedEventArgs e)
    {
        EnsureRenderTarget();
    }

    private void OnHostSizeChanged(object sender, SizeChangedEventArgs e)
    {
        var w = (int)Math.Max(e.NewSize.Width, 1);
        var h = (int)Math.Max(e.NewSize.Height, 1);

        if (_childHwnd != IntPtr.Zero)
        {
            Win32.MoveWindow(_childHwnd, 0, 0, w, h, true);
        }

        VirtualView?.OnNativeSurfaceResized((uint)w, (uint)h);
    }

    private void OnHostUnloaded(object sender, RoutedEventArgs e)
    {
        DestroyRenderTarget();
    }

    // ---------------------------------------------------------------
    // Render target lifecycle
    // ---------------------------------------------------------------

    private void EnsureRenderTarget()
    {
        if (_childHwnd != IntPtr.Zero) return;
        if (PlatformView?.XamlRoot?.Content == null) return;
        if (VirtualView == null) return;

        // Get the top-level HWND from the MAUI window
        var windowHwnd = GetHostWindowHandle();
        if (windowHwnd == IntPtr.Zero) return;

        var w = (int)Math.Max(PlatformView.ActualWidth, 1);
        var h = (int)Math.Max(PlatformView.ActualHeight, 1);

        // Create a Win32 child window parented to the top-level HWND.
        // Vulkan creates its VkSurfaceKHR from this child HWND.
        _childHwnd = Win32.CreateRenderChildWindow(windowHwnd, w, h);
        if (_childHwnd == IntPtr.Zero) return;

        var handle = EngineInterop.CreateRenderTarget(
            _childHwnd, (uint)w, (uint)h, VirtualView.RenderMode);

        VirtualView.RenderTargetHandle = handle;
        VirtualView.OnNativeSurfaceCreated();
    }

    private void DestroyRenderTarget()
    {
        if (VirtualView?.RenderTargetHandle.IsValid == true)
        {
            EngineInterop.DestroyRenderTarget(VirtualView.RenderTargetHandle);
            VirtualView.RenderTargetHandle = default;
        }

        if (_childHwnd != IntPtr.Zero)
        {
            Win32.DestroyWindow(_childHwnd);
            _childHwnd = IntPtr.Zero;
        }
    }

    private IntPtr GetHostWindowHandle()
    {
        // Walk the MAUI application windows to find the WinUI Window,
        // then retrieve its Win32 HWND.
        if (Application.Current is not null)
        {
            foreach (var window in Application.Current.Windows)
            {
                if (window.Handler?.PlatformView is Microsoft.UI.Xaml.Window winuiWindow)
                {
                    return WinRT.Interop.WindowNative.GetWindowHandle(winuiWindow);
                }
            }
        }
        return IntPtr.Zero;
    }

#else
    protected override object CreatePlatformView() => new object();
#endif
}
