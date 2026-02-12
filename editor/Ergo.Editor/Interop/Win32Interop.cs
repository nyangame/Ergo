using System.Runtime.InteropServices;

namespace Ergo.Editor.Interop;

#if WINDOWS

/// <summary>
/// Win32 P/Invoke declarations for creating a child HWND that
/// Vulkan can render into. The child window is embedded inside
/// the MAUI WinUI host panel.
/// </summary>
internal static partial class Win32
{
    // ---------------------------------------------------------------
    // Constants
    // ---------------------------------------------------------------

    public const uint WS_CHILD       = 0x40000000;
    public const uint WS_VISIBLE     = 0x10000000;
    public const uint WS_CLIPSIBLINGS = 0x04000000;
    public const uint WS_CLIPCHILDREN = 0x02000000;

    public const uint CS_OWNDC = 0x0020;

    public const int CW_USEDEFAULT = unchecked((int)0x80000000);

    public const int GWLP_WNDPROC = -4;

    public const uint WM_SIZE    = 0x0005;
    public const uint WM_DESTROY = 0x0002;

    // ---------------------------------------------------------------
    // Structures
    // ---------------------------------------------------------------

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct WNDCLASSEXW
    {
        public uint cbSize;
        public uint style;
        public IntPtr lpfnWndProc;
        public int cbClsExtra;
        public int cbWndExtra;
        public IntPtr hInstance;
        public IntPtr hIcon;
        public IntPtr hCursor;
        public IntPtr hbrBackground;
        [MarshalAs(UnmanagedType.LPWStr)]
        public string? lpszMenuName;
        [MarshalAs(UnmanagedType.LPWStr)]
        public string lpszClassName;
        public IntPtr hIconSm;
    }

    // ---------------------------------------------------------------
    // Functions
    // ---------------------------------------------------------------

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial ushort RegisterClassExW(ref WNDCLASSEXW wc);

    [LibraryImport("user32.dll", EntryPoint = "CreateWindowExW",
                    SetLastError = true, StringMarshalling = StringMarshalling.Utf16)]
    public static partial IntPtr CreateWindowExW(
        uint dwExStyle,
        string lpClassName,
        string lpWindowName,
        uint dwStyle,
        int x, int y, int nWidth, int nHeight,
        IntPtr hWndParent,
        IntPtr hMenu,
        IntPtr hInstance,
        IntPtr lpParam);

    [LibraryImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static partial bool DestroyWindow(IntPtr hWnd);

    [LibraryImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static partial bool MoveWindow(
        IntPtr hWnd, int x, int y, int nWidth, int nHeight,
        [MarshalAs(UnmanagedType.Bool)] bool bRepaint);

    [LibraryImport("user32.dll")]
    public static partial IntPtr DefWindowProcW(
        IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

    [LibraryImport("kernel32.dll")]
    public static partial IntPtr GetModuleHandleW(IntPtr lpModuleName);

    // ---------------------------------------------------------------
    // Child window factory
    // ---------------------------------------------------------------

    private static bool s_classRegistered;
    private const string ClassName = "ErgoRenderHost";

    /// <summary>
    /// Create a child window (HWND) parented to <paramref name="parent"/>.
    /// This HWND is passed to the native engine so Vulkan can create a
    /// VkSurfaceKHR from it.
    /// </summary>
    public static IntPtr CreateRenderChildWindow(IntPtr parent,
                                                  int width, int height)
    {
        IntPtr hInstance = GetModuleHandleW(IntPtr.Zero);

        if (!s_classRegistered)
        {
            var wc = new WNDCLASSEXW
            {
                cbSize = (uint)Marshal.SizeOf<WNDCLASSEXW>(),
                style = CS_OWNDC,
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(
                    new WndProcDelegate(WndProc)),
                hInstance = hInstance,
                lpszClassName = ClassName,
            };
            RegisterClassExW(ref wc);
            s_classRegistered = true;
        }

        IntPtr hwnd = CreateWindowExW(
            0,
            ClassName,
            "ErgoRenderSurface",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, width, height,
            parent,
            IntPtr.Zero,
            hInstance,
            IntPtr.Zero);

        return hwnd;
    }

    // Prevent the delegate from being garbage collected
    private static readonly WndProcDelegate s_wndProcDelegate = WndProc;

    private delegate IntPtr WndProcDelegate(IntPtr hWnd, uint msg,
                                             IntPtr wParam, IntPtr lParam);

    private static IntPtr WndProc(IntPtr hWnd, uint msg,
                                   IntPtr wParam, IntPtr lParam)
    {
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

#endif // WINDOWS
