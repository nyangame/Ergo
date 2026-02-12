using Ergo.Editor.Interop;

namespace Ergo.Editor.Controls;

/// <summary>
/// Game preview view. Renders exactly what the player would see at runtime,
/// without editor overlays like grids or gizmos. The camera is controlled
/// by the game logic rather than the editor.
/// </summary>
public class GameView : RenderView
{
    // -----------------------------------------------------------------
    // Play state
    // -----------------------------------------------------------------

    public static readonly BindableProperty IsPlayingProperty =
        BindableProperty.Create(
            nameof(IsPlaying),
            typeof(bool),
            typeof(GameView),
            false);

    public bool IsPlaying
    {
        get => (bool)GetValue(IsPlayingProperty);
        set => SetValue(IsPlayingProperty, value);
    }

    public static readonly BindableProperty IsPausedProperty =
        BindableProperty.Create(
            nameof(IsPaused),
            typeof(bool),
            typeof(GameView),
            false);

    public bool IsPaused
    {
        get => (bool)GetValue(IsPausedProperty);
        set => SetValue(IsPausedProperty, value);
    }

    // -----------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------

    public GameView()
    {
        RenderMode = ErgoRenderMode.Game;
    }

    // -----------------------------------------------------------------
    // Overrides
    // -----------------------------------------------------------------

    internal override void OnNativeSurfaceCreated()
    {
        base.OnNativeSurfaceCreated();
        // Game view uses the game's own camera. We set a sensible default.
        if (RenderTargetHandle.IsValid)
        {
            EngineInterop.SetCamera(
                RenderTargetHandle,
                eye: new ErgoVec3(0, 5, -10),
                target: new ErgoVec3(0, 0, 0),
                up: new ErgoVec3(0, 1, 0),
                fovDegrees: 60.0f,
                nearPlane: 0.1f,
                farPlane: 1000.0f);
        }
    }

    internal override void OnBeforeRender(float dt)
    {
        base.OnBeforeRender(dt);

        if (IsPlaying && !IsPaused)
        {
            // Tick the engine simulation when the game is running
            EngineInterop.Tick(dt);
        }
    }

    // -----------------------------------------------------------------
    // Playback controls
    // -----------------------------------------------------------------

    public void Play()
    {
        IsPlaying = true;
        IsPaused = false;
    }

    public void Pause()
    {
        IsPaused = true;
    }

    public void Stop()
    {
        IsPlaying = false;
        IsPaused = false;
        // TODO: Restore scene to pre-play state
    }

    public void StepFrame()
    {
        if (!IsPlaying) return;
        IsPaused = true;
        EngineInterop.Tick(1.0f / 60.0f);
    }
}
