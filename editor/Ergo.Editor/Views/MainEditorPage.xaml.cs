using Ergo.Editor.Controls;
using Ergo.Editor.ViewModels;

namespace Ergo.Editor.Views;

public partial class MainEditorPage : ContentPage
{
    private readonly MainEditorViewModel _viewModel = new();
    private IDispatcherTimer? _renderTimer;

    public MainEditorPage()
    {
        BindingContext = _viewModel;
        InitializeComponent();

        // Wire up scene view selection events
        SceneViewControl.ObjectSelected += OnSceneObjectSelected;
    }

    // -----------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------

    protected override void OnAppearing()
    {
        base.OnAppearing();

        _viewModel.InitializeEngine();

        // Start the render loop (~60 FPS)
        _renderTimer = Dispatcher.CreateTimer();
        _renderTimer.Interval = TimeSpan.FromMilliseconds(16);
        _renderTimer.Tick += OnRenderTick;
        _renderTimer.Start();
    }

    protected override void OnDisappearing()
    {
        _renderTimer?.Stop();
        _renderTimer = null;

        _viewModel.ShutdownEngine();
        base.OnDisappearing();
    }

    // -----------------------------------------------------------------
    // Render loop
    // -----------------------------------------------------------------

    private void OnRenderTick(object? sender, EventArgs e)
    {
        const float dt = 1.0f / 60.0f;

        SceneViewControl.RenderFrame(dt);
        GameViewControl.RenderFrame(dt);
    }

    // -----------------------------------------------------------------
    // Toolbar
    // -----------------------------------------------------------------

    private void OnPlayClicked(object? sender, EventArgs e)
    {
        _viewModel.IsPlaying = true;
        GameViewControl.Play();
    }

    private void OnPauseClicked(object? sender, EventArgs e)
    {
        _viewModel.IsPaused = !_viewModel.IsPaused;
        if (_viewModel.IsPaused)
            GameViewControl.Pause();
        else
            GameViewControl.Play();
    }

    private void OnStopClicked(object? sender, EventArgs e)
    {
        _viewModel.IsPlaying = false;
        _viewModel.IsPaused = false;
        GameViewControl.Stop();
    }

    // -----------------------------------------------------------------
    // Hierarchy selection
    // -----------------------------------------------------------------

    private void OnHierarchySelectionChanged(
        object? sender, SelectionChangedEventArgs e)
    {
        if (e.CurrentSelection.FirstOrDefault() is SceneObjectEntry entry)
        {
            SceneViewControl.SelectedObject = entry.Handle;
            ComponentViewControl.SetSelectedObject(entry.Handle);
        }
    }

    // -----------------------------------------------------------------
    // Scene view picking
    // -----------------------------------------------------------------

    private void OnSceneObjectSelected(
        object? sender, Interop.ErgoGameObjectHandle handle)
    {
        ComponentViewControl.SetSelectedObject(handle);

        // Also update hierarchy selection
        _viewModel.RefreshSceneHierarchy();
    }
}
