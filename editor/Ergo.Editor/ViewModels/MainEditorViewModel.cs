using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Ergo.Editor.Interop;

namespace Ergo.Editor.ViewModels;

/// <summary>
/// ViewModel for the main editor page. Manages the scene hierarchy list
/// and coordinates between the scene view, game view, and component view.
/// </summary>
public class MainEditorViewModel : INotifyPropertyChanged
{
    private bool _isPlaying;
    private bool _isPaused;
    private bool _engineInitialized;

    // -----------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------

    public ObservableCollection<SceneObjectEntry> SceneObjects { get; } = new();

    public bool IsPlaying
    {
        get => _isPlaying;
        set { _isPlaying = value; OnPropertyChanged(); OnPropertyChanged(nameof(CanPlay)); OnPropertyChanged(nameof(CanStop)); }
    }

    public bool IsPaused
    {
        get => _isPaused;
        set { _isPaused = value; OnPropertyChanged(); }
    }

    public bool CanPlay => !_isPlaying;
    public bool CanStop => _isPlaying;

    // -----------------------------------------------------------------
    // Engine lifecycle
    // -----------------------------------------------------------------

    public bool InitializeEngine()
    {
        if (_engineInitialized) return true;
        _engineInitialized = EngineInterop.Init() != 0;
        if (_engineInitialized)
        {
            RefreshSceneHierarchy();
        }
        return _engineInitialized;
    }

    public void ShutdownEngine()
    {
        if (!_engineInitialized) return;
        EngineInterop.Shutdown();
        _engineInitialized = false;
    }

    // -----------------------------------------------------------------
    // Scene hierarchy
    // -----------------------------------------------------------------

    public void RefreshSceneHierarchy()
    {
        SceneObjects.Clear();
        uint count = EngineInterop.GetObjectCount();
        if (count == 0) return;

        var handles = new ErgoGameObjectHandle[count];
        uint fetched = EngineInterop.GetObjects(handles, count);

        for (uint i = 0; i < fetched; i++)
        {
            SceneObjects.Add(new SceneObjectEntry
            {
                Handle = handles[i],
                Name = EngineInterop.GetObjectName(handles[i]),
            });
        }
    }

    // -----------------------------------------------------------------
    // INotifyPropertyChanged
    // -----------------------------------------------------------------

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

public class SceneObjectEntry
{
    public ErgoGameObjectHandle Handle { get; set; }
    public string Name { get; set; } = "";
}
