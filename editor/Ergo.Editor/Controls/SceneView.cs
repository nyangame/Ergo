using Ergo.Editor.Interop;

namespace Ergo.Editor.Controls;

/// <summary>
/// Scene editor view. Renders the 3D scene with editor overlays
/// (grid, gizmos, selection outlines). Supports orbit camera
/// controlled by mouse input.
/// </summary>
public class SceneView : RenderView
{
    // -----------------------------------------------------------------
    // Camera orbit parameters
    // -----------------------------------------------------------------

    private float _orbitDistance = 15.0f;
    private float _orbitYaw;          // radians around Y axis
    private float _orbitPitch = 0.4f; // radians above horizon
    private ErgoVec3 _orbitTarget = new(0, 0, 0);

    private const float PitchMin = -1.4f; // ~-80 degrees
    private const float PitchMax = 1.4f;  // ~+80 degrees
    private const float ZoomSpeed = 1.0f;
    private const float OrbitSpeed = 0.005f;
    private const float PanSpeed = 0.02f;

    // -----------------------------------------------------------------
    // Selection
    // -----------------------------------------------------------------

    public static readonly BindableProperty SelectedObjectProperty =
        BindableProperty.Create(
            nameof(SelectedObject),
            typeof(ErgoGameObjectHandle),
            typeof(SceneView),
            default(ErgoGameObjectHandle),
            propertyChanged: OnSelectedObjectChanged);

    public ErgoGameObjectHandle SelectedObject
    {
        get => (ErgoGameObjectHandle)GetValue(SelectedObjectProperty);
        set => SetValue(SelectedObjectProperty, value);
    }

    public event EventHandler<ErgoGameObjectHandle>? ObjectSelected;

    private static void OnSelectedObjectChanged(
        BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is SceneView sv)
        {
            sv.ObjectSelected?.Invoke(sv, (ErgoGameObjectHandle)newValue);
        }
    }

    // -----------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------

    public SceneView()
    {
        RenderMode = ErgoRenderMode.Scene;
    }

    // -----------------------------------------------------------------
    // Overrides
    // -----------------------------------------------------------------

    internal override void OnNativeSurfaceCreated()
    {
        base.OnNativeSurfaceCreated();
        UpdateCamera();
    }

    internal override void OnBeforeRender(float dt)
    {
        base.OnBeforeRender(dt);
        // Camera updates are driven by input events, not per-frame
    }

    // -----------------------------------------------------------------
    // Camera control (called from input handling in the page)
    // -----------------------------------------------------------------

    public void Orbit(float deltaX, float deltaY)
    {
        _orbitYaw -= deltaX * OrbitSpeed;
        _orbitPitch += deltaY * OrbitSpeed;
        _orbitPitch = Math.Clamp(_orbitPitch, PitchMin, PitchMax);
        UpdateCamera();
    }

    public void Pan(float deltaX, float deltaY)
    {
        // Compute right and up vectors from current camera orientation
        float cosYaw = MathF.Cos(_orbitYaw);
        float sinYaw = MathF.Sin(_orbitYaw);

        float rightX = cosYaw;
        float rightZ = sinYaw;
        float upX = 0;
        float upY = 1;
        float upZ = 0;

        float panAmount = PanSpeed * _orbitDistance * 0.1f;
        _orbitTarget.X -= (rightX * deltaX + upX * deltaY) * panAmount;
        _orbitTarget.Y += upY * deltaY * panAmount;
        _orbitTarget.Z -= (rightZ * deltaX + upZ * deltaY) * panAmount;
        UpdateCamera();
    }

    public void Zoom(float delta)
    {
        _orbitDistance -= delta * ZoomSpeed;
        _orbitDistance = Math.Max(_orbitDistance, 0.5f);
        UpdateCamera();
    }

    public void FocusOnOrigin()
    {
        _orbitTarget = new ErgoVec3(0, 0, 0);
        _orbitDistance = 15.0f;
        _orbitYaw = 0;
        _orbitPitch = 0.4f;
        UpdateCamera();
    }

    /// <summary>
    /// Attempt to pick an object at the given screen-space coordinates.
    /// </summary>
    public void Pick(float screenX, float screenY)
    {
        if (!RenderTargetHandle.IsValid) return;
        var hit = EngineInterop.PickObject(RenderTargetHandle, screenX, screenY);
        SelectedObject = hit;
    }

    // -----------------------------------------------------------------
    // Internal
    // -----------------------------------------------------------------

    private void UpdateCamera()
    {
        if (!RenderTargetHandle.IsValid) return;

        float cosPitch = MathF.Cos(_orbitPitch);
        float sinPitch = MathF.Sin(_orbitPitch);
        float cosYaw = MathF.Cos(_orbitYaw);
        float sinYaw = MathF.Sin(_orbitYaw);

        var eye = new ErgoVec3(
            _orbitTarget.X + _orbitDistance * cosPitch * sinYaw,
            _orbitTarget.Y + _orbitDistance * sinPitch,
            _orbitTarget.Z + _orbitDistance * cosPitch * cosYaw
        );

        var up = new ErgoVec3(0, 1, 0);

        EngineInterop.SetCamera(
            RenderTargetHandle,
            eye, _orbitTarget, up,
            fovDegrees: 60.0f,
            nearPlane: 0.1f,
            farPlane: 1000.0f);
    }
}
